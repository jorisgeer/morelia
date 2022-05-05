/* ast.c - create abstract syntax tree from parse tree

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

#include <string.h>

#include "base.h"
#include "mem.h"

#include "chr.h"

#include "fmt.h"

static ub4 msgfile = Shsrc_ast;
#include "msg.h"

#include "tok.h"

#include "lexsyn.h"

#include "syndef.h"

#include "synast.h"

#include "ast.h"
#include "exp.h"

// int cexp   , ?: || && ^ | & != == < > << >> b+ b- * / % ~ ! u+ u- ( ) 0
//            a b  c  d  e e f g  g  h h i  i  j  j  k k k l l l  l  l l l
//            0 1  2  3  4   5 6     7   8     9     10    11

static ub1 oprecs[Ocnt] = {
  [Olit]  = 26,  [Oneg]  = 25,  [Onot]  = 25,  [Oumin] = 25,  [Oupls] = 25,
  [Omul]  = 22,  [Odiv]  = 22,  [Omod]  = 22,  [Omin]  = 20,  [Opls]  = 20,
  [Oshl]  = 18,  [Oshr]  = 18,  [Olt]   = 16,  [Ogt]   = 16,  [Ole]   = 16,
  [Oge]   = 16,  [One]   = 14,  [Oeq]   = 14,  [Oand]  = 12,  [Oxor]  = 10,
  [Oor]   = 8,  [Oreland] = 6,  [Orelor] = 4,  [Oqst]  = 2,  [Ocol]  = 2,
// ass
  [Ocom]  = 0 };

// create full expr from list of half 'unary-expr+operator' using precedence
static ub2 precexp(struct ast *ap,ub4 *vdst,ub4 *odst,ub4 *src,ub2 n)
{
  ub2 i;
  ub4 vals[Explen];
  ub1 stk[Explen];
  ub2 vi=0,oi=0,sp=1,rr=0,ra,rb=0,b=0;
  ub4 e,bi;
  enum Astyp t;
  enum Op op;
  struct uexp *ue,*ueps = ap->uexps;
  struct pexp *pe,*peps = ap->pexps;

  *stk = 0;

  for (i = 0; i < n-1; i++) {
    e = src[i];
    t = e >> Atybit; // should be Apexp, last Auexp
    bi = e & Atymsk;
    pe = peps + bi;
    op = pe->op;
    vals[vi] = pe->l;
    rb = vi++;
    while (op > stk[sp]) {
      ra = b-1;
      odst[oi++] = stk[sp--] | (ra << 8) | (rb << 4);
      rb--;
    }
    stk[++sp] = op;
  }
  while (sp) {
    ra = b-1;
    odst[oi++] = stk[sp--] | (ra << 8) | (rb << 4);
    rb--;
  }

  e = src[n-1];
  t = e >> Atybit; // should be Auexp
  bi = e & Atymsk;
  ue = ueps + bi;
  vals[vi++] = ue->e;
  return vi + oi;
}

static cchar atynames[] = "Aid\0Ailit\0Ailits\0Aflit\0Aslit\0Atoken\0Aop\0Apexp\0Auexp\0Abexp\0Aiter\0Arep\0Arexp\0Aargs\0Astmts\0Acount\0*inv*\0";

cchar *atynam(enum Astyp t)
{
  cchar *p = atynames;

  if (t > Acount) t = Acount;
  while (t--) {
    do ++p; while (*p);
    p++;
  }
  return p;
}

void emitast(struct ast *ap)
{
  ub4 q[Depth];
  ub4 qi=0;

  struct stmtlst *stmts;

  struct stmtlst *stmtlp,*stmtls = ap->stmtls;
  struct id *idp,*ids = ap->ids;
  struct ilit *ilitp,*ilits = ap->ilits;
  struct uexp *uexpp,*uexps = ap->uexps;
  struct rexp *rexpp,*rexps = ap->rexps;

  ub4 n,ni,nnd=0,ndcnt = ap->len;
  ub4 lpos,lcnt,pos=0;
  ub4 *rep,*repool = ap->repool;
  enum Astyp t;
  ub2 cnt=0,r;

  q[qi++] = ap->root;

  while (qi && nnd < ndcnt) {

    n = q[qi--];

// -----
    next:
// -----

    nnd++;

    t = n >> Atybit;
    ni = n & Atymsk;

    switch(t) {

    // lists
    case Astmts:
      stmtlp = stmtls + ni;
      cnt = stmtlp->cnt;
      pos = stmtlp->pos;
      break;

    case Arexp:
      rexpp = rexps + ni;
      cnt = rexpp->cnt;
      pos = rexpp->pos;
      break;

    // leaves
    case Aid:
      idp = ids + ni;
      info("id %u",idp->id);
      goto next;

    case Ailit:
      ilitp = ilits + ni;
      info("ilit %u",ilitp->val);
      goto next;

    // nodes
    case Auexp:
      uexpp = uexps + ni;
      n = uexpp->e;
      info("uexp %u %u",n,uexpp->op);
      goto next;
    case Apexp: break; // replaced to Abexp

    default: break;
    }

    if (t >= Arep) {
      for (r = 0; r < cnt; r++) {
        memcpy(q+qi,repool + pos,cnt * 4);
        qi += cnt;
      }
    }
  }
}

void *mkast(struct synast *sa,struct lexsyn *lsp)
{
  struct ast *ap = minalloc(sizeof(struct ast),0,0,"ast");

  struct rnode *rp,*rpp = sa->nodes;
  ub4 *args = sa->args;
  ub4 nvar,nuex,nbex;
  ub4 repos=0;
  ub4 ri,a,a0,ni,ni0,i,ii,i0,ai;
  ub4 ti=0,fpos,fpos_ilit=0;
  ub2 dfp;
  ub4 bits=0;
  const ub4 *tkbits = lsp->tkbits;
  const ub4 *tkfpos = lsp->tkpos;
  ub2 am;
  enum Astyp t,t0;
  enum Production ve;
  enum Op op;
  ub2 cnt,len;

  ub4 *ndcnts = sa->ndcnts;

  ub4 nstmtlst = ndcnts[Astmts];
  ub4 nilit = ndcnts[Ailit];

  for (t = 0; t < Acount; t++) {
    a = ndcnts[t];
    if (a) info("%3u type %u %s",a,t,atynam(t));
  }

  if (nstmtlst == 0) ice(hi32,0,"nil stmt list for %u nodes",ap->len);

  struct uexp *uexpp,*uexps = alloc(ndcnts[Auexp],struct uexp,Mo_nofill | Mo_ok0,"ast exp",nextcnt);
  struct bexp *bexps = alloc(ndcnts[Abexp],struct bexp,Mo_nofill | Mo_ok0,"ast exp",nextcnt);
  struct pexp *pexpp,*pexps = alloc(ndcnts[Apexp],struct pexp,Mo_nofill | Mo_ok0,"ast exp",nextcnt);
  struct rexp *rexpp,*rexps = alloc(ndcnts[Arexp],struct rexp,Mo_nofill | Mo_ok0,"ast exp",nextcnt);

  struct id *idp,*ids = alloc(ndcnts[Aid],struct id,Mo_nofill | Mo_ok0,"ast id",nextcnt);

  struct ilit *ilitp,*ilits = alloc(nilit,struct ilit,Mo_nofill | Mo_ok0,"ast ilit",nextcnt);
  ub2 *ilitfps = alloc(nilit,ub2,Mo_nofill | Mo_ok0,"ast ilit",nextcnt);

  struct stmtlst *stmtlp,*stmtls = alloc(nstmtlst,struct stmtlst,Mo_nofill | Mo_ok0,"ast stmtlst",nextcnt);
//  struct if *ifs = alloc(ndcnts[Aif],struct if,Mo_nofill | Mo_ok0,"ast if",nextcnt);

  struct args *rargp,*rargs = alloc(ndcnts[Aargs],struct args,Mo_nofill | Mo_ok0,"ast args",nextcnt);

  ub4 *rep,*repool = alloc(ndcnts[Arep],ub4,Mo_nofill | Mo_ok0,"ast rep",nextcnt);

  ap->uexps = uexps;
  ap->ids = ids;
  ap->ilits = ilits;
  ap->stmtls = stmtls;

  ap->repool = repool;

  enum Token tk;

  rp = rpp;
  ai = 0;

  do {
    ve = rp->ve;

    am = rp->amask;

    info("ni %u lvl %u ve %u args %x",(ub4)(rp - rpp),rp->lvl,ve,am);

    a = rpp->ni;
    t = a >> Atybit;
    i = (a >> Aopbit) & Aopmsk;

    ni = a & Atymsk;

    while (am & 1) {
      a0 = args[ai++];

      t0 = a0 >> Atybit;
      i0 = (a0 >> Aopbit) & Aopmsk;

      ni0 = a0 & Atymsk;

      switch (t0) {
      case Aid:    ti = *args++; break;

      case Ailits: break;

      case Ailit:
        ti = *args++;
        ilitp = ilits + ni;
        ilitp->val = tkbits[ti];
        fpos = tkfpos[ti];
        if ( (ni & 0xf) == 0) ilitfps[ni >> 4] = fpos;
        else { dfp = fpos - fpos_ilit; ilitfps[ni] = min(dfp,hi16); }
        fpos_ilit = fpos;
        break;

      case Aflit:  ti = *args++; break;
      case Aslit:  ti = *args++; break;
      case Aop:    op = (i0 >> Aopbit) & Aopmsk; break;
      default:
        if (t0 <= Atoken) {
          cnt = (i0 >> Aopbit) & Aopmsk;
          bits = ti >> 24; // todo rep
        } else {
          ni0 = ii;
        }
      }
      am >>= 1;
    } // each arg

    switch (t) {

    // nodes
    case Auexp:   uexpp = uexps + ni;
                  uexpp->e = args[0];
                  uexpp->op = op;
    break;

    case Apexp:   pexpp = pexps + ni;
                  pexpp->l = args[0];
                  pexpp->op = op;
    break;

    // leaves
    case Aid:     idp = ids + ni;
                  idp->id = bits;
//                  idfpos[ni] = fpos;
    break;

    case Ailit: break;

    // lists
    case Arexp:     cnt = *args++;
                    rexpp = rexps + ni;
                    rexpp->pos = repos;
                    len = precexp(ap,repool + repos,repool + repos + cnt,args,cnt);
                    rexpp->len = len;
                    repos += cnt * 2;
                    args += cnt;
    break;

    case Aargs:     cnt = *args++;
                    rargp = rargs + ni;
                    rargp->pos = repos;
                    rargp->cnt = cnt;
                    memcpy(repool + repos,args,cnt * 4);
                    repos += cnt;
                    args += cnt;
    break;
    case Astmts:    cnt = *args++;
                    stmtlp = stmtls + ni;
                    stmtlp->cnt = cnt;
                    stmtlp->pos = repos;
                    memcpy(repool + repos,args,cnt * 4);
                    repos += cnt;
                    args += cnt;
    break;

    case Acount: goto end;

    default: break;
    }
    rp++;
  } while (1);

// ---
  end:
// ---

  ap->root = nstmtlst - 1;

  return ap;
}

enum Astyp prd2nod[Pcount] = {
  [Pwhile] = Aiter,
  [Pexpr_0] = Apexp
};
