/* msg.c - log and diagnostic messages

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

/* This file contains messaging logic used for 4 purposes:
   - diagnostics relating to the compiland e.g. syntax error in source
   - diagnostics relating to the language processor e.g. internal error
   - intermediate code emit
   - assertions
 */

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "base.h"

#include "chr.h"

#include "mem.h"

#include "fmt.h"
extern ub4 myvsnprint(char *dst,ub4 pos,ub4 len,const char *fmt,va_list ap);

static ub4 msgfile = Shsrc_msg;
#include "msg.h"

#include "os.h"

#include "util.h"

static inline ub4 strlen4(cchar *s) { return (ub4)strlen(s); }

static enum Msglvl msglvl = Info;
static enum Msgopts orgopts,msgopts = Msg_shcoord; // | Msg_tim;
static bool nobuffer = 0;

static int ttyfd = 1;
static int logfd = -1;

static ub8 T0;

static ub4 warncnt,errcnt,assertcnt,oserrcnt;

#define Msglen 1024

ub4 msgwarncnt(void) { return warncnt; }

ub4 msgerrcnt(void)
{
  ub4 cnt = errcnt + assertcnt + oserrcnt;

  return cnt;
}

static void dowritefd(int fd,const char *buf,ub4 len)
{
  int rv;

  if (len == 0) { oswrite(fd,"\nnil msg\n",9); return; }

  ub4 i,prvi=0;

#if 1
  for (i = 0; i < len; i++) {
    if (buf[i] == 0)  {
      if (prvi < i) oswrite(fd,buf+prvi,i - prvi);
      prvi = i+1;
      oswrite(fd," \\x00 ",6);
    }
  }
  if (prvi >= len) return;
#endif

  rv = oswrite(fd,buf+prvi,len-prvi);

  if (rv) {
    oswrite(2,"\nI/O error on msg write\n",24);
    oserrcnt++;
  }
}

static void dowrite(const char *buf,ub4 len)
{
  if (logfd != -1) dowritefd(logfd,buf,len);
  else dowritefd(ttyfd,buf,len);
}

static ub4 msgseq,errseq,warnseq;

static char msgbuf[8192];
static ub4 bufpos,buftop = 8192;

static char lastwarn[128];
static char lasterr[128];

static void flsbuf(void)
{
  if (bufpos) {
    dowrite(msgbuf,bufpos);
    bufpos = 0;
  }
}

void msgfls(void) { flsbuf(); }

static void msgwrite(cchar *p,ub4 len)
{
  ub4 n;

  if (nobuffer) { dowrite(p,len); return; }

  while (bufpos + len > buftop) {
    n = buftop - bufpos;
    memcpy(msgbuf + bufpos,p,n); p += n; len -= n;
    dowrite(msgbuf,buftop);
    bufpos = 0;
  }
  if (len) { memcpy(msgbuf + bufpos,p,len); bufpos += len; }
}

void msg_write(const char *buf,ub4 len) { msgwrite(buf,len); }

static void msg_swrite(const char *buf)
{
  if (buf) msgwrite(buf,strlen4(buf));
}

static const char *shflnames[Shsrc_count] = {
  [Shsrc_ast]    = "ast",
  [Shsrc_base]   = "base",
  [Shsrc_dia]    = "dia",
  [Shsrc_mem]    = "mem",
  [Shsrc_msg]    = "msg",
  [Shsrc_lex]    = "lex",
  [Shsrc_lex1]   = "lextb1",
  [Shsrc_lex2]   = "lextb2",
  [Shsrc_genlex] = "genlex",
  [Shsrc_gensyn] = "gensyn",
  [Shsrc_os]     = "os",
  [Shsrc_syn]    = "syn",
  [Shsrc_main]   = "morelia",
  [Shsrc_time]   = "time",
  [Shsrc_util]   = "util"
};

static void msg_wrfln(ub4 fln)
{
  char buf[64];

  if (fln == 0 || fln == hi32) return;

  ub4 lno = fln & 0xffff;
  ub4 fileno = fln >> 16;
  ub4 n;

  const char *name = fileno < Shsrc_count ? shflnames[fileno] : "???";

  n = mysnprintf(buf,0,64,"%6s.%-4u\n",name,lno);

  msg_write(buf,n);
}

void msg_errwrite(ub4 fln,ub4 fln2,const char *buf)
{
  msg_swrite("\n");
  msg_wrfln(fln);
  msg_wrfln(fln2);
  msg_swrite(" error: ");

  if (buf) msg_swrite(buf);
}

static void msginfo(ub4 shfln)
{
  globs.shfln = shfln;
}

/* main message printer.
 * if ap is nil, print fmt as %s
 */
static void msgps(ub4 shfln,enum Msglvl lvl,cchar *srcnam,ub4 lno,ub4 col,cchar *fmt,va_list ap,cchar *pfx,cchar *sfx)
{
  ub4 n,pos = 0,maxlen = Msglen - 1;
  ub4 shfno=0,shlno=0;
  cchar *shfnam = nil;
  cchar *lvlnam = "";
  cchar *p;
  char buf[Msglen];
  char msgbuf2[32];
  enum Msgopts dotim = msgopts & Msg_tim;
  ub4 ec;
  ub8 T1,dt=0;

  if (dotim) {
    T1 = gettime_usec();
    dt = T1 - T0;
  }

  if (fmt == nil) {
    fmt = "(nil fmt)"; ap = nil; lvl = Fatal;
  } else if ((size_t)fmt < 4096) {
    fmtstring(msgbuf2,"(int fmt 0x%x)",(ub4)(ub8)fmt);
    fmt = msgbuf2; ap = nil; lvl = Fatal;
  }

  if (shfln && shfln != hi32) {
    shfno = shfln >> 16;
    shlno = shfln & hi16;

    shfnam = shfno < Shsrc_count ? shflnames[shfno] : "???";
    if (shfnam == nil) shfnam = "???";
  }

  switch (lvl) {
    case Fatal:   lvlnam = "fatal";   errcnt++;    break;
    case Assert:  lvlnam = "assert";  assertcnt++; break;
    case Oserror: lvlnam = "Error";   oserrcnt++;  break;
    case Error:   lvlnam = "error";   errcnt++;    break;
    case Warn:    lvlnam = "warning"; warncnt++;   break;
    case Info:    lvlnam = "info";                 break;
    case Vrb:     lvlnam = "vrb";                  break;
    case Vrb2:    lvlnam = "Vrb";                  break;
    case Nolvl:   lvlnam = "";                     break;
  }

  if (dotim) pos += mysnprintf(buf,pos,maxlen,"%4u ",(ub4)dt);

  if (shfln && shfln != hi32 && (msgopts & Msg_shcoord)) pos += mysnprintf(buf,pos,maxlen,"%6s.%-4u ",shfnam,shlno);

  if (lvl < Info || (msgopts & Msg_lvl)) pos += mysnprintf(buf, pos, maxlen, "%-8s",lvlnam);

  if (srcnam) {
    if (msgopts & Msg_fno) {
      pos += mysnprintf(buf,pos,maxlen,"%s ",srcnam);
    }
    if (lno && (msgopts & Msg_lno)) {
      if (col && (msgopts & Msg_col) ) pos += mysnprintf(buf,pos,maxlen,"%3u.%-2u: ",lno,col);
      else pos += mysnprintf(buf, pos, maxlen, "%3u: ",lno);
    }
  }

  if (pfx) {
    while (*pfx && pos + 1 < maxlen) buf[pos++] = *pfx++;
  }

  if (ap) pos += myvsnprint(buf,pos,maxlen,fmt,ap);
  else { while (pos < maxlen && *fmt) buf[pos++] = *fmt++; }

  if (lvl == Oserror) {
    p = getoserr(&ec);
    pos += mysnprintf(buf,pos,maxlen," : %s (%u)",p,ec);
  }

  if (sfx) {
    if (pos + 1 < maxlen) buf[pos++] = ' ';
    while (*sfx && pos + 1 < maxlen) buf[pos++] = *sfx++;
  }
  buf[pos++] = '\n';

  if (logfd != -1 && lvl < Info) {
    flsbuf();
    logfd = -1;
  }

  msgwrite(buf,pos);

  if (pos >= maxlen) {
    flsbuf();
    logfd = -1;
    msg_errwrite(shfln,FLN,"\nerror: buffer full at above message\n");
    msg_swrite(fmt);
    flsbuf();
  } else

  if (lvl == Warn) {
    memcpy(lastwarn,buf,n=min(pos-1,sizeof(lastwarn)-1));
    lastwarn[n] = 0;
    warnseq = msgseq;
  } else if (lvl < Warn) {
    memcpy(lasterr,buf,n=min(pos-1,sizeof(lasterr)-1));
    lasterr[n] = 0;
    errseq = msgseq;
    flsbuf();
  }
  msgseq++;
}

static void msg(ub4 shfln,enum Msglvl lvl,cchar *srcnam,ub4 lno,ub4 col,cchar *fmt,va_list ap)
{
  msgps(shfln,lvl,srcnam,lno,col,fmt,ap,nil,nil);
}

void msglog(cchar *fnam,cchar *fext,cchar *desc)
{
  char path[Fname];
  ub1 x;

  if (logfd >= 0) {
    msgopts = orgopts;
    info("done emitting %s intermediate code",desc);
    flsbuf();
    osclose(logfd);
    logfd = -1;
  }
  if (fnam == nil || *fnam == 0) {
    return;
  }

  fmtstring(path,"%s.%s",fnam,fext);

  filebck(path);

  info("emitting %s intermediate code in %s",desc,path);
  flsbuf();
  logfd = oscreate(path);
  if (logfd == -1) { oserror("cannot create %s: %m",path); return; }
  orgopts = msgopts;
  x = (ub1)msgopts & ~ (ub1)(Msg_shcoord|Msg_tim);
  msgopts = (enum Msgopts)x;
  info("%s - intermediate %s code dump\n",path,desc);
}

static struct expmem fnaminfos,fnampos;

static ub4 fpos9;

struct fnaminf *getsrcmfile(void)
{
  if (fnaminfos.inc == 0) icefln(FLN,0,hi32,"msg not inited");
  struct fnaminf *mf = (struct fnaminf *)minalloc(sizeof(struct fnaminf),8,0,"filnam");

  struct fnaminf **mfp = blkexp(&fnaminfos,1,struct fnaminf *);
  ub4 *poss = blkexp(&fnampos,1,ub4);

  *mfp = mf;
  mf->fpos0 = *poss = fpos9;
  mf->parfpos = hi32;

  return mf;
}

int chksrcmfile(struct fnaminf *mf)
{
  ub4 n;
  struct fnaminf *mf2,**mfs = (struct fnaminf **)fnaminfos.bas;

  for (n = 0; n < fnaminfos.pos; n++) {
    mf2 = mfs[n];
    if (mf2 == mf) continue;
    if (mf->ino == mf2->ino && mf->dev == mf2->dev) { // skip same file
      return 1;
    }
  }
  return 0;
}

void setsrcmfile(struct fnaminf *mf,ub4 *lntab,ub4 lncnt,ub4 len)
{
  mf->lntab = lntab;
  mf->lncnt = lncnt;
  fpos9 += len;
}

/* returns file name,line,col given 32-bit linear pos
 * uses file and line table
 * interprets pos as line if no line table
 */
static const char *getsrcpos(ub4 fpos,ub4 *plno,ub4 *pcol,ub4 *pparfpos)
{
  ub4 n,pos,lno,col,fpos0;
  struct fnaminf *sp,**spp;
  ub4 lncnt=0,*lntab=nil;
  ub4 filcnt;
  ub4 *srcfpos = (ub4 *)fnampos.bas;
  bool linonly=0;

  *plno = 0; *pcol = 0;
  *pparfpos = hi32;

  if (fpos == hi32) return nil;
  else if (fpos & Lno) { linonly = 1; fpos &= ~Lno; }

  filcnt = fnaminfos.pos;

  pos = fpos;
  if (filcnt == 0) {
    *plno = fpos;
    return "";
  } else if (filcnt == 1) n = 0;
  else {
    n = bsearch4(srcfpos,filcnt,fpos,FLN);
    n = min(n,filcnt-1);
    while (n && srcfpos[n] >= fpos) n--;
    if (n) {
      fpos0 = srcfpos[n-1];
      if (fpos < fpos0) { errorfln(FLN,0,"invalid pos %u below file org %u",fpos,fpos0); return nil; }
      pos = fpos - fpos0;
    }
  }
  if (n >= filcnt) { errorfln(FLN,0,"invalid file no %u",n); return nil; }
  spp = (struct fnaminf **)fnaminfos.bas;
  sp = spp[n];
  lntab = sp->lntab;
  lncnt = sp->lncnt;
  if (lntab == nil || linonly) { *plno = fpos; return sp->path + sp->incdir; } // pass line numbers directly if no lntab
  if (lncnt == 0) lno = 0;
  else if (lncnt == 1) lno = 0;
  else {
    lno = bsearch4(lntab,lncnt,pos,FLN);
    while (lno && lntab[lno] >= pos) lno--;
  }
  if (lno) {
    if (pos < lntab[lno]) { errorfln(FLN,0,"invalid pos %u below line %u org %u",pos,lno,lntab[lno]); return sp->name; }
    col = pos - lntab[lno];
  } else col = pos;
  col++;
  *plno = lno; *pcol = col;

  *pparfpos = sp->parfpos;
  return sp->path + sp->incdir;
}

ub4 getsrcln(ub4 fpos)
{
  ub4 par,col,lno;

  getsrcpos(fpos,&lno,&col,&par);
  return lno;
}

void vpmsg(ub4 shfln,enum Msglvl lvl,const char *srcnam,ub4 lno,ub4 col,const char *fmt,va_list ap,const char *suffix)
{
  if (msglvl >= lvl || lvl >= Nolvl) msgps(shfln,lvl,srcnam,lno,col,fmt,ap,nil,suffix);
}

static void vmsgint(ub4 shfln,enum Msglvl lvl,ub4 fpos,cchar *fmt,va_list ap,cchar *pfx,cchar *sfx)
{
  ub4 lno,col;
  ub4 parfpos = hi32;
  cchar *name = nil;

  if (fpos != hi32) {
    name = getsrcpos(fpos,&lno,&col,&parfpos);
    if (name == nil) errorfln(FLN,shfln,"nil name for fpos %u",fpos);
  } else { name = nil; lno = col = 0; }

  msgps(shfln,lvl,name,lno,col,fmt,ap,pfx,sfx);

  while (parfpos != hi32) {
    if (parfpos >= fpos) { errorfln(FLN,0,"invalid parent pos %u above %u",parfpos,fpos); return; }
    fpos = parfpos;
    name = getsrcpos(fpos,&lno,&col,&parfpos);
    msg(Info,shfln,name,lno,col,"included",nil);
  }
  if (lvl <= Fatal) doexit(1);
}

void vmsg(ub4 shfln,enum Msglvl lvl,ub4 fpos,cchar *fmt,va_list ap)
{
  if (msglvl >= lvl || lvl >= Nolvl) vmsgint(shfln,lvl,fpos,fmt,ap,nil,nil);
}

void vmsgps(ub4 shfln,enum Msglvl lvl,ub4 fpos,cchar *fmt,va_list ap,cchar *pfx,cchar *sfx)
{
  if (msglvl >= lvl || lvl >= Nolvl) vmsgint(shfln,lvl,fpos,fmt,ap,pfx,sfx);
}

void __attribute__ ((format (printf,3,4))) genmsgfln(ub4 fln,enum Msglvl lvl,const char *fmt,...)
{
  va_list ap;

  msginfo(fln);

  if (msglvl < lvl) return;

  va_start(ap, fmt);
  msg(fln,lvl,nil,0,0,fmt,ap);
  va_end(ap);
}

void __attribute__ ((format (printf,2,3))) vrb2fln(ub4 fln,const char *fmt,...)
{
  va_list ap;
  enum Msglvl lvl = Vrb2;

  msginfo(fln);

  if (msglvl < lvl) return;

  va_start(ap, fmt);
  msg(fln,lvl,nil,0,0,fmt,ap);
  va_end(ap);
}

void __attribute__ ((format (printf,2,3))) vrbfln(ub4 fln,const char *fmt,...)
{
  va_list ap;
  enum Msglvl lvl = Vrb;

  msginfo(fln);

  if (msglvl < lvl) return;

  va_start(ap, fmt);
  msg(fln,lvl,nil,0,0,fmt,ap);
  va_end(ap);
}

void __attribute__ ((format (printf,3,4))) svrbfln(ub4 shfln,ub4 fpos,const char *fmt, ...)
{
  va_list ap;

  enum Msglvl lim = Vrb;

  if (msglvl < lim) return;

  va_start(ap, fmt);
  vmsgint(shfln,lim,fpos,fmt,ap,nil,nil);
  va_end(ap);
}

void __attribute__ ((format (printf,2,3))) infofln(ub4 fln,const char *fmt,...)
{
  va_list ap;

  enum Msglvl lim = Info;

  msginfo(fln);

  if (msglvl < lim) return;

  va_start(ap, fmt);
  msg(fln,lim,nil,0,0,fmt,ap);
  va_end(ap);
}

void __attribute__ ((format (printf,3,4))) sinfofln(ub4 shfln,ub4 fpos,const char *fmt, ...)
{
  va_list ap;

  msginfo(shfln);
  if (msglvl < Info) return;

  va_start(ap, fmt);
  vmsgint(shfln,Info,fpos,fmt,ap,nil,nil);
  va_end(ap);
}

void __attribute__ ((format (printf,2,3))) warnfln(ub4 fln,const char *fmt,...)
{
  va_list ap;

  msginfo(fln);
  if (msglvl < Warn) return;

  va_start(ap, fmt);
  msg(fln,Warn,nil,0,0,fmt,ap);
  va_end(ap);
}

void __attribute__ ((format (printf,3,4))) swarnfln(ub4 shfln,ub4 fpos,const char *fmt, ...)
{
  va_list ap;

  msginfo(shfln);
  if (msglvl < Warn) return;

  va_start(ap, fmt);
  vmsgint(shfln,Warn,fpos,fmt,ap,nil,nil);
  va_end(ap);
}

Noret void __attribute__ ((format (printf,4,5))) fatalfln(ub4 fln,ub4 fln2,ub4 fpos,const char *fmt, ...)
{
  va_list ap;

  msg_wrfln(fln2);

  va_start(ap, fmt);
  vmsgint(fln,Fatal,fpos,fmt,ap,nil," : exiting");
  va_end(ap);
  doexit(1);
}

Noret void __attribute__ ((format (printf,4,5))) icefln(ub4 fln,ub4 fln2,ub4 fpos,const char *fmt, ...)
{
  va_list ap;

  msg_wrfln(fln2);

  va_start(ap, fmt);
  vmsgint(fln,Fatal,fpos,fmt,ap,"ice "," : exiting");
  va_end(ap);
  doexit(1);
}

Noret void __attribute__ ((format (printf,3,4))) serrorfln(ub4 shfln,ub4 fpos,const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vmsgint(shfln,Error,fpos,fmt,ap,nil,nil);
  va_end(ap);
  doexit(1);
}

void __attribute__ ((format (printf,3,4))) errorfln(ub4 fln,ub4 fln2,const char *fmt, ...)
{
  va_list ap;

  msg_wrfln(fln2);

  va_start(ap, fmt);
  msg(fln,Error,nil,0,0,fmt,ap);
  va_end(ap);
}

Noret void __attribute__ ((format (printf,2,3))) assertfln(ub4 fln,const char *fmt,...)
{
  va_list ap;

  va_start(ap,fmt);
  msgps(fln,Assert,0,0,0,fmt,ap,nil," : exiting");
  va_end(ap);
  doexit(1);
}

void __attribute__ ((format (printf,2,3))) oserrorfln(ub4 fln,const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  msgps(fln,Oserror,0,0,0,fmt,ap,nil,nil);
  va_end(ap);
}

ub4 limit_gt_fln(ub4 x,ub4 lim,ub4 arg,const char *sx,const char *slim,const char *sarg,ub4 fln)
{
  if (lim == 0) assertfln(fln,"zero limit %s for %s",sx,slim);
  if (x < lim - 1) return x;
  if (x == lim - 1) {
    warnfln(fln,"limiting %s:%u to %s:%u for %s:%u",sx,x,slim,lim,sarg,arg);
    return x;
  }
  return lim;
}

void showcntfln(ub4 fln,cchar *nam,ub4 cnt)
{
  ub2 n = 0;

  if (cnt == 0 || nam == nil) return;

  if (Ctab[*nam] == Cnum) n = (ub2)(*nam++ - '0');
  if (*nam == '#') infofln(fln,"%*u` %s",n,cnt,nam+1);
  else infofln(fln,"%*u` %s%.*s",n,cnt,nam,cnt != 1,"s");
}

void showsizfln(ub4 fln,cchar *nam,ub4 siz)
{
  ub2 n = 0;

  if (siz == 0 || nam == nil) return;

  if (Ctab[*nam] == Cnum) n = (ub2)(*nam++ - '0');
  infofln(fln,"%s %*u`",nam,n,siz);
}

void timeit2fln(ub4 fln,ub8 *pt0,ub4 n,cchar *mesg)
{
  ub8 t0u = *pt0;
  ub8 t1u = gettime_usec();
  ub8 dtu,dtm,dts;
  ub8 nn,npx=0;
  ub4 x4,y4=0;
  bool rem = 0;
  cchar *pfx;
  cchar *btk;
  char u;
  static char buf[256];
  ub4 pos;

  *pt0 = t1u;
  if (t0u == 0 || globs.resusg == 0 || mesg == nil || msglvl < Info) return; // start measure

  nn = n;
  dtu = t1u - t0u;
  if (dtu < 1500) { x4 = (ub4)dtu; pfx = "micro"; u = 'u'; npx = nn / max(dtu,1); }
  else {
    pfx = "milli"; u = 'm';
    dtm = dtu / 1000;
    if (dtu < 10000) { x4 = (ub4)dtm; rem = 1; y4 = dtu % 1000; npx = nn / max(dtm,1); }
    else if (dtm >= 1500) {
      pfx = ""; u = ' ';
      dts = dtm / 1000;
      npx = nn / max(dts,1);
      x4 = (ub4)dts; y4 = dtm % 1000; rem = 1;
    } else { x4 = (ub4)dtm; npx = nn / max(dtm,1); }
  }

  btk = strchr(mesg,'`');
  if (btk) {
    pos = 0;
    while (mesg < btk) buf[pos++] = *mesg++;
    pos += mysnprintf(buf,pos,256,"%u`%s %u",n,btk+1,x4);
  } else {
    pos = mysnprintf(buf,0,256,"%s %u",mesg,x4);
  }
  if (rem) pos += mysnprintf(buf,pos,256,".%u",y4 / 10);
  pos += mysnprintf(buf,pos,256," %ssecond%.*s",pfx,x4 != 1,"s");

  if (n > 1024) {
    if (u == 'm') { npx *= 1000; u = ' '; }
    else if (u == 'u') { npx *= 1000000; u = ' '; }
    mysnprintf(buf,pos,256," ~ %u` / %csec",(ub4)npx,u);
  }

  msg(fln,Info,nil,0,0,buf,nil);
}

void timeitfln(ub4 fln,ub8 *pt0,cchar *mesg)
{
  timeit2fln(fln,pt0,0,mesg);
}

// level to be set beforehand
void inimsg(ub2 opts)
{
  msgopts = opts;

  fnaminfos.inc = 2048;
  fnaminfos.elsiz = sizeof(void*);
  fnaminfos.ini = 128;
  memcpy(fnaminfos.desc,"fname",5);

  fnampos.inc = 2048;
  fnampos.elsiz = 4;
  fnampos.ini = 128;
  memcpy(fnampos.desc,"fname",5);

  T0 = gettime_usec();
}

void eximsg(void)
{
  ub4 ecnt = msgerrcnt();
  ub4 wcnt = warncnt;

  if (ecnt) {
    if (ecnt > 1 || msgseq - errseq > 5) showcnt("error",ecnt);
    if (msgseq - errseq > 5) info("  last %s",lasterr);
  } else if (wcnt) {
    if (wcnt > 1 || msgseq - warnseq > 5) showcnt("warning",wcnt);
    if (msgseq - warnseq > 5) info("  last %s",lastwarn);
  }

  if (wcnt && globs.errwarn && globs.retval == 0) {
    globs.retval = 1;
    infofln(FLN,"Treating warnings as error");
  }
  flsbuf();
  nobuffer = 1;
}

void setmsgbuf(bool ena)
{
  if (ena == 0) nobuffer = 1;
}

void setmsglvl(enum Msglvl lvl,ub2 opts)
{
  msglvl = lvl;
  msgopts = opts;
  if (nobuffer) infofln(FLN,"message buffer disabled");
  if (lvl >= Vrb) info("message level %u",lvl);
}

enum Msglvl getmsglvl(void) { return msglvl; }
