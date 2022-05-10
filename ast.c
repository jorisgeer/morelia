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

#ifdef __clang__
 #pragma clang diagnostic warning "-Wduplicate-enum"
#endif

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

  *stk = 0xff; stk[1] = 0;

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

static inline ub2 findvar(ub8 *scids,ub4 nscid,ub4 *scs,ub2 lvl,ub4 id)
{
  ub4 n,o;
  ub4 sc;
  ub8 b;

  do {
    sc = scs[lvl];
    n = sc >> 6;
    b = 1UL << (sc & 0x3f);
    o = id * nscid + n;
    if (scids[o] & b) return lvl;
    else if (lvl == 0) return hi16;
    lvl--;
  } while (1);
}

static inline void addvar(ub8 *scids,ub4 nscid,ub4 sc,ub2 lvl,ub4 id)
{
  ub4 n = sc >> 6;
  ub8 b = 1UL << (sc & 0x3f);
  ub4 o = id * nscid + n;

  scids[o] |= b;
}

static void process(struct ast *ap)
{
  ub4 q[Depth];
  ub4 qi=0;

  struct stmtlst *stmts;

  struct stmtlst *stmtlp,*stmtls = ap->stmtls;

  struct id *idp,*ids = ap->ids;
  struct ilit *ilitp,*ilits = ap->ilits;

  struct uexp *uexpp,*uexps = ap->uexps;
  struct bexp *bexpp,*bexps = ap->bexps;
  struct aexp *aexpp,*aexps = ap->aexps;

  struct rexp *rexpp,*rexps = ap->rexps;

  struct blk *blkp,*blks = ap->blks;

  struct asgnst *asgnstp,*asgnsts = ap->asgnsts;

  struct fndef *fndefp,*fndefs = ap->fndefs;
  struct param *prmp,*prms = ap->prms;

  ub4 nid = ap->nid;

  ub4 uidcnt = ap->uidcnt;

  ub8 *scids = ap->scids;
  ub2 *scidns = ap->scidns;
  ub4 nscid = ap->nscid;

  const ub4 *idfpos = ap->idfpos;

  ub4 n,ni,ni2,nidef,blkni,tgt,ret,prmlst,nnd=0,ndcnt = ap->len;
  ub4 nn;
  ub4 lpos,lcnt,pos=0;
  ub4 scid;
  ub4 ofs;
  ub4 id;
  ub2 lvl=0,vl;
  ub4 *rep,*repool = ap->repool;
  enum Astyp t;
  ub2 cnt=0,r;
  ub1 lhs = 0;

  ub1 blkbit = ap->blkbit;
  ub4 lvlen = (1U << blkbit) * uidcnt;
  ub4 *lvlvars = alloc(lvlen,ub4,Mo_nofill,"ast lvlvars",nextcnt);

  ub2 cursc = 0;
  ub4 scs[Depth];

  q[qi++] = ap->root;

  scs[0] = 0; // globals

  ub1 pass = 0;

  while (qi && nnd < ndcnt) {

    nn = q[--qi];

// -----
    next:
// -----

    nnd++;

    t = nn >> Atybit;
    ni = nn & Atymsk;

    switch((ub1)t) {

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
      idp->lvl = lvl;
      id = idp->id;
      vl = findvar(scids,nscid,scs,lvl,id);
      if (vl == hi16) {
        if (lhs) {
          lhs = 0;
          scid = scs[lvl];
          info("new var at lvl %u scid %u",lvl,scid);
          idp->def = ni;
          addvar(scids,nscid,scid,lvl,id);
          ofs = scidns[scid];
          idp->ofs = ofs;
          scidns[scid] = ofs + 1;
          lvlvars[(id << blkbit) + lvl] = ni;
        } else serror(idfpos[ni],"unknown var %u at lvl %u scid %u",id,lvl,cursc);
      } else {
        scid = scs[vl];
        idp->scid = scid;
        nidef = lvlvars[(id << blkbit) + vl];
        info("found at lvl %u scid %u def %u",vl,scid,nidef);
        idp->def = nidef;
      }
    break;

    case Ailit:
      ilitp = ilits + ni;
      info("ilit %u",ilitp->val);
    break;

    // block
    case Ablk:
      q[qi++] = nn | Aback; // set return
      scs[++lvl] = ++cursc;
      blkp = blks + ni;
      n = blkp->s;
      info("blk %u",n);
    goto next;

    // statements
    case Aasgnst: // a[i] = 3
      asgnstp = asgnsts + ni;
      tgt = asgnstp->tgt;
      q[qi++] = tgt;
      q[qi++] = nn | Aback;
      n = asgnstp->r;
      info("astmt tgt %u r %ulvl %u",tgt,n,lvl);
    goto next;

    case Afndef:
      fndefp = fndefs + ni;
      blkni = fndefp->blk;
      prmlst = fndefp->plst;
      idp = ids + fndefp->id * Atymsk;
      id = idp->id;
      ret = fndefp->ret;
      fndefp->scid0 = cursc;
      q[qi++] = nn | Aback;
      if (prmlst != hi32) q[qi++] = prmlst;
      n = blkni;
    goto next;

    // expressions
    case Auexp:
      uexpp = uexps + ni;
      n = uexpp->e;
      info("uexp %u %u",n,uexpp->op);
    goto next;

    case Apexp: break; // replaced to Abexp

    case Abexp:
      bexpp = bexps + ni;
      q[qi++] = bexpp->r;
      n = bexpp->l;
      info("bexp %u %u",n,bexpp->op);
    goto next;

    case Aaexp: // a := b
      aexpp = aexps + ni;
      q[qi++] = aexpp->e;
      n = aexpp->id;
      ni2 = n & Atymsk;

      idp = ids + ni2;
      id = idp->id;

      vl = findvar(scids,nscid,scs,lvl,id);

      idp->lvl = lvl;
      idp->rol = Lhs;
      if (vl == hi16) { // new def
        scid = scs[lvl];
        info("new var at lvl %u scid %u",lvl,scid);
        idp->def = ni2;
        addvar(scids,nscid,scid,lvl,id);
        ofs = scidns[scid];
        idp->ofs = ofs;
        scidns[scid] = ofs + 1;
        lvlvars[(id << blkbit) + lvl] = ni2;
      } else {
        scid = scs[vl];
        info("found at lvl %u scid %u",vl,scid);
        nidef = lvlvars[(id << blkbit) + vl];
        info("found at lvl %u scid %u def %u",vl,scid,nidef);
        idp->def = nidef;
      }
      info("aexp id %u lvl %u",id,lvl);
      idp->scid = scid;

    goto next;

    case Ablk | Aback:
      if (lvl) lvl--;
    break;

    case (enum Astyp)(Aasgnst | Aback):
      lhs = 1;
    break;

    case (enum Astyp)(Afndef | Aback):
      fndefp = fndefs + ni;
      fndefp->scid1 = cursc;
    break;

    default: break;
    }

    if (t >= Arep) {
      for (r = 0; r < cnt; r++) {
        memcpy(q+qi,repool + pos,cnt * 4);
        qi += cnt;
      }
    } else {

    }
  }

  ub4 *varsctab;
  ub4 *varscs = alloc(nscid,ub4,Mo_nofill,"ast vars",nextcnt);
  ub4 varsclen;

  if (pass == 0) {
    ofs = 0;
    for (scid = 0; scid < nscid; scid++) { varscs[scid] = ofs; ofs += scidns[scid]; }
    varsclen = ofs;
    varsctab = alloc(varsclen,ub4,Mo_nofill,"ast varsctab",nextcnt);
    for (ni = 0; ni < nid; ni++) {
      idp = ids + ni;
      scid = idp->scid;
      ofs = varscs[scid] + idp->ofs;
      varsctab[ofs] = ni;
    }
  }
}

void *mkast(struct synast *sa,struct lexsyn *lsp)
{
  struct ast *ap = minalloc(sizeof(struct ast),0,0,"ast");

  struct rnode *rp,*rpp = sa->nodes;
  ub4 *args = sa->args;
  ub8 val,*vals = sa->vals;
  ub4 vi=0;
  ub4 nvar,nuex,nbex;
  ub4 repos=0;
  ub4 ri,a=0,a0,ni,ni0,i,ii,i0,ai;
  ub4 fpos,fpos_ilit=0;
  ub2 dfp;
  ub4 bits=0;
  const ub8 *tkbits = lsp->bits;
  ub2 am,ac,ainc,la;
  enum Astyp t,t0;
  enum Production ve;
  enum Op op;
  ub2 cnt,len;

  sassert(Acount <= Atymsk,"atypes");
  sassert(Ocnt <= Aopmsk,"opbits");

  ub4 uidcnt = sa->uidcnt;
  ub4 idcnt = sa->idcnt;

  ub4 *ndcnts = sa->ndcnts;

  ub4 nid = ndcnts[Aid];

  if (nid != idcnt) ice(0,0,"idcnt %u vs %u,",idcnt,nid);

  ub4 nilit = ndcnts[Ailit];

  ub4 nuexp = ndcnts[Auexp];
  ub4 nbexp = ndcnts[Abexp];
  ub4 npexp = ndcnts[Apexp];
  ub4 naexp = ndcnts[Aaexp];

  ub4 nrexp = ndcnts[Arexp];

  ub4 nwiter = ndcnts[Awhile];

  ub4 nblk  = ndcnts[Ablk];

  ub4 nstmtlst = ndcnts[Astmts];

  struct mempart ndpart[Acount];

  for (t = 0; t < Acount; t++) {
    cnt = ndcnts[t];
    if (cnt) info("%3u type %u %s",a,t,atynam(t));
    ndpart[t].nel = cnt;
  }

  if (nstmtlst == 0) ice(hi32,0,"nil stmt list for %u nodes",ap->len);

  ndpart[Aid].siz = sizeof(struct id);
  ndpart[Ailit].siz = sizeof(struct ilit);

  ndpart[Auexp].siz = sizeof(struct uexp);
  ndpart[Abexp].siz = sizeof(struct bexp);
  ndpart[Apexp].siz = sizeof(struct pexp);
  ndpart[Aaexp].siz = sizeof(struct aexp);

  ndpart[Arexp].siz = sizeof(struct rexp);

  ndpart[Aasgnst].siz = sizeof(struct asgnst);

  ndpart[Afndef].siz = sizeof(struct fndef);
  ndpart[Aparam].siz = sizeof(struct param);

  ndpart[Ablk].siz = sizeof(struct blk);

  ndpart[Awhile].siz = sizeof(struct witer);

  ndpart[Astmts].siz = sizeof(struct stmtlst);
  ndpart[Aprmlst].siz = sizeof(struct prmlst);

  void *ndbas = allocset(ndpart,Acount,Mo_nofill,"ast tree",nextcnt);

  struct id *idp,*ids = ndpart[Aid].ptr;

  struct ilit *ilitp,*ilits = ndpart[Ailit].ptr;

  struct uexp *uexpp,*uexps = ndpart[Auexp].ptr;
  struct bexp *bexps = ndpart[Abexp].ptr;
  struct pexp *pexpp,*pexps = ndpart[Apexp].ptr;
  struct aexp *aexpp,*aexps = ndpart[Aaexp].ptr;

  struct rexp *rexpp,*rexps = ndpart[Arexp].ptr;

  struct asgnst *asgnstp,*asgnsts = ndpart[Aasgnst].ptr;

  struct blk *blkp,*blks = ndpart[Ablk].ptr;

  struct fndef *fndefp,*fndefs = ndpart[Afndef].ptr;
  struct param *prmp,*prms = ndpart[Aparam].ptr;

  struct witer *witerp,*witers = ndpart[Awhile].ptr;

  struct stmtlst *stmtlp,*stmtls = ndpart[Astmts].ptr;

  struct prmlst *rprmp,*rprms = ndpart[Aprmlst].ptr;

  ub4 *rep,*repool = alloc(ndcnts[Arep],ub4,Mo_nofill | Mo_ok0,"ast rep",nextcnt);

  ub4 *idfpos = alloc(nid,ub4,Mo_nofill | Mo_ok0,"ast id",nextcnt);

  ap->uexps = uexps;
  ap->bexps = bexps;
  ap->aexps = aexps;

  ap->rexps = rexps;

  ap->ids = ids;
  ap->ilits = ilits;

  ap->asgnsts = asgnsts;

  ap->fndefs = fndefs;
  ap->prms = prms;

  ap->witers = witers;

  ap->stmtls = stmtls;

  ap->repool = repool;

  ap->nid = nid;

  ub4 nscid = sa->nscid;
  ub4 scopsiz = (uidcnt * nscid) >> 6;
  ub8 *scids = alloc(scopsiz,ub8,Mo_nofill,"ast scopes",nextcnt);

  ub2 hiblklvl = sa->hiblklvl;

  ub1 blkbit = msb(hiblklvl+1);

  if (cntbits(blkbit) > 1) blkbit++;

  ub2 *scidns = alloc(nscid,ub2,Mo_nofill,"ast scopes",nextcnt);
//  ub4 *varscs = alloc(sum,ub4,);

  ap->nscid = nscid;
  ap->scids = scids;
  ap->scidns = scidns;
  ap->blkbit = blkbit;

  ub4 lfargs[Nodarg];

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

    ac = 0;
    a0 = args[0];
    if (a0 < (Aleaf << Atybit)) lfargs[ac++] = a0;
    ainc = 2;

    switch (t) {

    // exp nodes
    case Auexp:
      uexpp = uexps + ni;
      uexpp->e = a0;
      uexpp->op = args[1] >> Aopbit;
    break;

    case Apexp:
      pexpp = pexps + ni;
      pexpp->l = a0;
      pexpp->op = args[1] >> Aopbit;
    break;

    case Aaexp:
      aexpp = aexps + ni;
      aexpp->id = a0;
      aexpp->e = args[1];
    break;

    case Abexp: break;

   // block
    case Ablk:
      blkp = blks + ni;
      blkp->s = a0;
      ainc = 1;
    break;

    case Aasgnst:
      asgnstp = asgnsts + ni;
      asgnstp->tgt = a0;
      asgnstp->r = args[1];
    break;

   case Afndef:
     fndefp = fndefs + ni;
     fndefp->id = a0;
     fndefp->plst = args[1];
     fndefp->blk = args[2];
   break;

   case Aparam:
     prmp = prms + ni;
     prmp->id = a0;
     if (am & 2) {
       prmp->t = args[1];
     } else ainc = 1;
     if (am & 4) {
       prmp->def = args[2];
       ainc++;
     }
    break;

    // iters
    case Awhile:
      witerp = witers + ni;
      witerp->e = a0;
      witerp->tb = args[1];
      if (am & 4) {
        witerp->fb = args[2];
        ainc = 3;
      }
    break;

    // leaves
    case Aid:
    case Ailit:
    case Aflit:
    case Aslit:
    case Ailits: break;

    case Aop:    break;

    // lists
    case Arexp:     cnt = *args++;
                    rexpp = rexps + ni;
                    rexpp->pos = repos;
                    len = precexp(ap,repool + repos,repool + repos + cnt,args,cnt);
                    rexpp->len = len;
                    repos += cnt * 2;
                    ainc = cnt;
    break;

    case Aprmlst:   cnt = *args++;
                    rprmp = rprms + ni;
                    rprmp->pos = repos;
                    rprmp->cnt = cnt;
                    memcpy(repool + repos,args,cnt * 4);
                    repos += cnt;
                    ainc = cnt;
    break;

    case Astmts:    cnt = *args++;
                    stmtlp = stmtls + ni;
                    stmtlp->cnt = cnt;
                    stmtlp->pos = repos;
                    memcpy(repool + repos,args,cnt * 4);
                    repos += cnt;
                    ainc = cnt;
    break;

    case Acount: goto end;

//    default: break;
    }
    args += ainc;

    for (la = 0; la < ac; la++) {
      a0 = lfargs[la];
      t0 = a0 >> Atybit;
      ni0 = a0 & Atymsk;

      val = vals[vi++];

      switch (t0) {
      case Aid:
        idp = ids + ni0;
        idp->id = val & hi32;
        idfpos[ni0] = val >> 32;
      break;

      case Ailits: break;

      case Ailit:
        bits = *args++;
        ilitp = ilits + ni;
        ilitp->val = bits;
        break;

      case Aflit:  bits = *args++; break;
      case Aslit:  bits = *args++; break;

      default: break;

    }
  } // postporcess leaf args

    rp++;
  } while (1);

// ---
  end:
// ---

  ap->root = nstmtlst - 1;

  process(ap);

  return ap;
}

enum Astyp prd2nod[Pcount] = {
  [Pwhile] = Awhile,
  [Pexp]   = Apexp
};
