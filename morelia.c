/* morelia.c - morelia bytecode compiler main program

   This file is part of Morelia, a subset of Python with emphasis on efficiency.

   Copyright © 2022 Joris van der Geer.

   Morelia is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Morelia is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program, typically in the file License.txt
   If not, see http://www.gnu.org/licenses.
 */

#include <stddef.h>
#include <string.h>

#include "base.h"

#include "mem.h"

#include "os.h"

static ub4 msgfile = Shsrc_main;
#include "msg.h"

#include "dia.h"

#include "lexsyn.h"

#include "syndef.h"
#include "synast.h"

#include "util.h"

struct globs globs;

enum Cmdopt { Co_until=1,Co_prog,Co_emit,Co_include,Co_Werror,Co_Wwarn,Co_Winfo,Co_Wtrace };

static ub2 msgopt = Msg_shcoord | Msg_fno | Msg_lno | Msg_col; // | Msg_tim

#define Incdirs 64
static const ub1 *incdirs[Incdirs];
static ub4 incdircnt;

static struct lexsyn ls;
static struct synast sa;

#define Incstk 64

struct incfile {
  const ub1 *nam;
  ub4 len;
  bool isfile;
};

static struct incfile incstk[Incstk];
static ub2 incspos;

static int docc(const ub1 *src,ub4 slen,bool isfile)
{
  int rv;
  ub8 T0 = 0,T1 = 0;
  struct ast *ap;

  ls.incdircnt = incdircnt;
  ls.incdirs = incdirs;

  timeit(&T0,nil);

  if (isfile) {
    vrb("compile from file %.32s",src);
    ls.name = (cchar *)src;
    rv = lexfile(FLN,(cchar *)src,"",Inone,&ls,T0);
  } else {
    ls.name = "cmdline";
    vrb("compile from cmdline len %u '%.16s%s'",slen,src,slen > 16 ? "..." : "");
    rv = lexstr(src,slen,&ls,T0);
  }
  if (rv) return rv;

  if (globs.rununtil == 3) { info("until lex %u",globs.rununtil); return 0; }

  if (ls.tkcnt == 0) {
    if (isfile) info("%s is empty",src);
    return 0;
  }

  if (inisyn()) return 1;

  timeit(&T1,nil);

  rv = syn(&ls,&sa);

  timeit2(&T1,ls.srclen,"parse took ");

  timeit2(&T0,ls.srclen,"lex + parse- took ");

  if (rv) return rv;

  if (sa.ndcnt == 0) {
    info("%s is empty",ls.name);
  }

  if (globs.rununtil == 4) { info("until syn %u",globs.rununtil); return 0; }

  ap = mkast(&sa,&ls);

  timeit2(&T0,ls.srclen,"lex + parse+ took ");

  if (rv) return 1;

//  afree(ls.tkbas,"lex tokens",nextcnt);

  return 0;
}

void addmod(const ub1 *nam,ub4 len,bool isfile)
{
  struct incfile *ip = incstk + incspos++;

  ip->nam = nam;
  ip->len = len;
  ip->isfile = isfile;
}

static int domod(void)
{
  ub2 i;
  int rv;
  struct incfile *ip;

  while (incspos) {
    for (i = 0; i < incspos; i++) {
      ip = incstk + i;
      rv = docc(ip->nam,ip->len,ip->isfile);
      if (rv) return rv;
      incspos--;
    }
  }
  return 0;
}

static struct cmdopt cmdopts[] = {
  { "",        'c', Co_prog,    "prog", "program to run as string" },
  { "emit",    ' ', Co_emit,    "%elex,syn,ast,sem","intermediate pass output to emit" },
  { "until",   ' ', Co_until,   "%einit,file,lex1,lex,syn,ast", "process until <pass>" },

  { "include", 'I', Co_include, "dir",  "add directory to include search path" },

  { "Werror",  ' ', Co_Werror,  "list", "comma-separated list of diags to report as error" },
  { "Wwarn",   ' ', Co_Wwarn,   "list", "comma-separated list of diags to report as warning" },
  { "Winfo",   ' ', Co_Winfo,   "list", "comma-separated list of diags to report as info" },

  { "Wtrace",  ' ', Co_Wtrace,  "list", "comma-separated list of diags to report as trace" },

  { nil,0,0,"<srcfile>","Morelia"}
};

static void modinfo(void)
{
  cchar *l = lex_info();
  cchar *s = syn_info();

  infofln(0,"%s\n%s\n",l,s);
}

static const ub1 *srcnam;
static const ub1 *cmdprog;
static ub2 cmdprglen;

static int cmdline(int argc, char *argv[])
{
  ub4 orgargc = (ub4)argc;
  struct cmdval coval;
  struct cmdopt *op;
  enum Parsearg pa;
  cchar *prgnam;
  bool haveopt,havereg,endopt;

  ub2 cmdlut[256];

  if (argc > 0) {
    prgnam = strrchr(argv[0],'/');
    if (prgnam) prgnam++; else prgnam = argv[0];
    argc--; argv++;
  } else prgnam = "morelia";

  globs.prgnam = prgnam;
  globs.msglvl = Info;
  globs.rununtil = 0xff;

  iniutil();

  prepopts(cmdopts,cmdlut,1);

  while (argc) { // options
    haveopt = 0;
    havereg = 0;
    endopt = 0;
    pa = parseargs((ub4)argc,argv,cmdopts,&coval,cmdlut,1);

    switch (pa) {
    case Pa_nil:
    case Pa_eof:   endopt = 1; break;

    case Pa_min2:  endopt = 1; break;

    case Pa_min1:
    case Pa_plusmin:
    case Pa_plus1: havereg = 1; break;

    case Pa_regarg:havereg = 1; break; // first non-option regular

    case Pa_notfound:
      error("option '%.*s' at arg %u not found",coval.olen,*argv,orgargc - (ub4)argc);
      return 1;

    case Pa_noarg:
      error("option '%.*s' missing arg",coval.olen,*argv);
      return 1;

    case Pa_found:
    case Pa_genfound:
      if (coval.err) { errorfln(FLN,coval.err,"option '%.*s' invalid arg",coval.olen,*argv); return 1; }
      haveopt = 1;
      argc--; argv++;
      break;
    case Pa_found2:
    case Pa_genfound2:
      if (coval.err) { error("option '%.*s' invalid arg",coval.olen,*argv); return 1; }
      haveopt = 1;
      argc -=2;
      argv += 2;
      break;
    }

    if (havereg) break;
    else if (endopt) { argc--; argv++; break; }

    else if (haveopt) {
      op = coval.op;

      switch(op->opt) {
      case Co_help:   return 1;
      case Co_license:return 1;
      case Co_version:if (globs.msglvl >= Info) modinfo(); return 1;
      case Co_verbose:
      case Co_dry:    break;

      case Co_include:if (incdircnt < Incdirs) incdirs[incdircnt++] = coval.sval;
                      break;

      case Co_prog:   cmdprog = coval.sval; cmdprglen = coval.vlen; break;
      case Co_until:  globs.rununtil = coval.uval; break;
      case Co_emit:   globs.emit |= (1U << coval.uval); break;
      case Co_Werror: diaset(Error,(cchar *)coval.sval); break;
      case Co_Wwarn:  diaset(Warn,(cchar *)coval.sval); break;
      case Co_Winfo:  diaset(Info,(cchar *)coval.sval); break;
      case Co_Wtrace: diaset(Vrb,(cchar *)coval.sval); break;
      }
    }
  }

  while (argc) { // regular args
    if (!srcnam) srcnam = (const ub1 *)*argv;
    argc--; argv++;
  }

  setmsglvl(globs.msglvl,msgopt);
  return 0;
}

static int init(void)
{
  inios();
  inimem();
  setsigs();

  inimsg(msgopt);

  return 0;
}

static void myexit(void)
{
  achkfree();
  eximsg();
  exios(1);
  eximem(1);
}

int main(int argc, char *argv[])
{
  int rv;

  init();

  rv = cmdline(argc,argv);
  if (rv) {
    eximsg();
    return 1;
  }

  if (srcnam) {
    addmod(srcnam,0,1);
  } else if (cmdprog) {
    addmod(cmdprog,cmdprglen,0);
  } else {
    errorfln(FLN,0,"script file or script arg expected");
    return 1;
  }

  inilex();

  if (globs.rununtil == 0) { infofln(FLN,"until lex"); return 0; }

  rv = domod();

  myexit();

  return rv || globs.retval;
  lastcnt
}
