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

#include <math.h>
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

#include "lexsyn.h"

#include "syndef.h"

#include "astyp.h"
#include "synast.h"

#include "ast.h"

#define Depth 256 // block nesting

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

#define Expdep 16

static ub1 oprecs[Obcnt] = {
  [Omul]  = 12, [Odiv]    = 12, [Omod]   = 12, [Oadd]  = 11, [Osub]  = 11,
  [Oshl]  = 10, [Oshr]    = 10, [Olt]    = 9,  [Ogt]   = 9,  [Ole]   = 9,
  [Oge]   = 9,  [One]     = 8,  [Oeq]    = 7,  [Oand]  = 6,  [Oxor]  = 5,
  [Oor]   = 4,  [Oreland] = 3,  [Orelor] = 2 };

#define Obbit 27
#define Regbit 23
#define Regmsk 0x7fffff
#define Obmsk  0x7ffffff

// create full expr from list of half 'unary-expr+operator' using precedence
static void precexp(struct ast *ap,struct rexp *rxp,ub4 *ops,ub4 *src,ub2 n)
{
  ub2 i;
  ub1 opp,ost[Expdep];
  ub2 vst[Expdep];
  ub2 vi=0,osp=0,vsp=0;
  ub2 reg=0,r1,r2,hit=0;
  ub4 ei;
  ub4 pnn,pnh,en;
  enum Astyp et;
  enum Bop op;
  struct pexp *pe,*peps = ap->pexps;
  ub4 *nhs = ap->nhs;

  *ost = 0xff;

  for (i = 0; i < n-1; i++) {
    pnn = src[i];
    pnh = nhs[pnn];
    et = pnh >> Atybit; // pexp
    ei = pnh & Atymsk;

    if (et != Apexp) serror(0,"expected pexp, found %u",et);
    pe = peps + ei;
    op = pe->op;
    en = pe->e;

    reg = vsp;
    vst[vsp++] = reg | (i << 8);
    if (reg > hit) hit = reg;
    ops[vi++]  = en | ((ub4)reg << Regbit) | ((ub4)Obcnt << Obbit); // eval req  4+4+24

    opp = oprecs[op] << 4;
    while (opp > ost[osp]) {
      op = ost[osp--];
      r1 = vst[vsp-2] & 0xff;
      r2 = vst[vsp-1] & 0xff;
      ops[vi++] = r1 | ((ub4)r2 << Regbit) | ((ub4)op << Obbit); // r0 = r1 op r2 4+4+4
      vst[vsp-2] = 0xff00; // r0
      vsp--;
    }

    if (osp >= Expdep) serror(0,"exp stk exceeds %u",Expdep);
    ost[++osp] = op | opp;
  }

  // last item without op
  pnn = src[i];
  pnh = nhs[pnn];
  et = pnh >> Atybit; // pexp
  ei = pnh & Atymsk;

  if (et != Apexp) serror(0,"expected pexp, found %u",et);
  pe = peps + ei;
  op = pe->op;
  en = pe->e;

  reg = vsp;
  vst[vsp++] = reg | (i << 8);
  ops[vi++]  = en | ((ub4)reg << Regbit) | ((ub4)Obcnt << Obbit);

  while (osp) {
    op = ost[osp--];
    r1 = vst[vsp-2] & 0xff;
    r2 = vst[vsp-1] & 0xff;
    ops[vi++] = r1 | ((ub4)r2 << Regbit) | ((ub4)op << Obbit);
    vst[vsp-2] = 0xff00;
    vsp--;
  }

  rxp->n = vi;
  rxp->hit = hit;
}

// todo placeholders
static ub4 mkop(ub1 flvl,ub4 x,ub4 y)
{ return 0;
}

static ub4 mklb(ub1 flvl,ub4 x)
{return 0;
}

static ub4 setlb(ub1 flvl,ub4 x)
{return 0;
}

static ub4 beval(ub4 ival1,enum Bop op,ub4 ival2) { return ival1 * ival2; }

#if 0
static void mkfnprms(struct fndef *fp,ub4 fn,ub4 pln)
{
  struct stmtlst *slp;
  ub4 pos = slp->pos;
  ub2 cnt = slp->cnt;
  ub4 ss,sn,si;
  enum Astyp st;

  if (cnt < 2) ice(0,0,"stmt list len %u",cnt);
  cnt--;
  nn = repool[pos+cnt];
  do {
    cnt--;
    ss = repool[pos+cnt];
    sn = nhs[ss];
    st = sn >> Atybit;
    si = sn & Atymsk;
    switch (st) {
//      case Aid: idp = ids + si; idp->rol = Fprm; break;
      case Ailit: case Ailits:
      case Aflit:
      case Aslit:
      case Akwd: break;
      default:
    }
  } while (cnt);

}
#endif

struct varscope {
  ub4 v0;
  ub2 n;
};

#define psh(n,p) stk[sp++] = n | (p << 28)
#define pop(n,p) sp--; n = stk[sp] & hi28; p = stk[sp] >> 28

static ub4 *process(struct ast *ap,bool emit)
{
  ub4 sp=0;

  ub4 nh,*nhs = ap->nhs;

  struct agen *gp,*gs = ap->gens;

  struct aid *idp,*ids = ap->ids;
  struct var *varp,*vars = ap->vars;

  struct ilit *ilitp,*ilits = ap->ilits;
  struct flit *flitp,*flits = ap->flits;
  struct slit *slitp,*slits = ap->slits;

  struct uexp *uexpp,*uexps = ap->uexps;
  struct bexp *bexpp,*bexps = ap->bexps;
  struct aexp *aexpp,*aexps = ap->aexps;

  struct asgnst *asgnstp,*asgnsts = ap->asgnsts;

  struct blk *blkp,*blks = ap->blks;

  struct witer *witerp,*witers = ap->witers;

  struct fndef *fndefp,*fndefs = ap->fndefs;
  struct param *prmp,*prms = ap->prms;

  struct rexp *rexpp,*rexps = ap->rexps;
  struct prmlst *prmlp,*prmls = ap->prmls;
  struct stmtlst *stmtlp,*stmtls = ap->stmtls;

  ub4 witercnt = ap->ndcnts[Awhile];

  ub4 nid = ap->nid;
  ub4 aidcnt = ap->aidcnt;

  ub4 uid,uidcnt = ap->uidcnt;

  ub4 vid = 0,parvid,vid0,curvid = 0,vidf0=0,varcnt = nid;

  ub2 hiblklvl = ap->hiblklvl;

  const ub4 *idfpos = ap->idfpos;

  ub4 enn,lnn,rnn,tnn,ss,en;
  ub4 ne,enh,lnh,rnh,tnh,plnh,sn;
  ub4 nn,ni,ei,ni2,nidef,blkni,lni,rni,li,ri,tni,plni,si;
  enum Astyp t,tt,et,lt,rt,st,nt;
  ub4 tgt,ret,tru,fal;

  ub4 prmlst,nnd=0,ndcnt = ap->len;
  ub4 n;
  ub4 lpos,lcnt,pos=0;
  ub4 scid;
  ub4 ofs;
  ub4 id;
  ub2 lvl=0,vl,lvv;
  ub2 loplvl=0;
  ub2 fnlvl=0;
  ub4 *ops,*rep,*repool = ap->repool;
  enum Typ ty,lty,rty;
  ub2 cnt=0,fac,r,oi;

  ub4 ival,ival1=0,ival2=0;
  ub4 x4;
  double fval,fval1,fval2;
  enum Bop op;

  ub4 head,pc0,pc = 0;
  ub1 reg,reg1,breg,res;

  ub4 qlen = ap->histlstsiz + Depth + 256;

  ub4 *stk = alloc(qlen,ub4,Mnofil,"ast q",nextcnt);
  ub1 pas;

//  ub1 blkbit = ap->blkbit;
  ub4 lvlen = hiblklvl + 2;
  ub8 lvlmsk,*lvlvars = alloc(uidcnt,ub8,Mnofil,"ast lvlvars",nextcnt); // idid
  ub4 *var2uid = alloc(varcnt,ub4,Mnofil,"ast var2uid", nextcnt);

  ub4 *idvars  = alloc(uidcnt * lvlen,ub4,Mnofil,"ast idvars", nextcnt);

  struct varscope varscs[Depth];
  struct varscope *vlp;
  ub2 blkbit=1;
  ub4 vp,vp0,nxv0;

  memset(varscs,0,sizeof(varscs));

  ub4 *seqs;
  ub4 seq=0,seqlen = aidcnt * 4 + witercnt * 8 + 4096;

  if (emit) {
    seqs = alloc(seqlen,ub4,Mnofil,"ast emit",nextcnt);
  } else seqs = nil;

  psh((aidcnt-1),0); // eof

  psh(ap->root,0);

  do {

    pop(nn,pas);

// -----
    next:
// -----

    nh = nhs[nn];
    t = nh >> Atybit;
    ni = nh & Atymsk;
    gp = gs + nn;

    if (emit) seqs[seq++] = nn | (pas << 28);

    info("%3u %3u %u %-6s ",nn,ni,pas,atynam(t));

    switch(t) {

    // leaves
    case Avar: break;

    case Aid: // todo x
      idp = ids + ni;
      idp->lvl = lvl;
      id = idp->id;
    break;

    case Ailit:
    case Ailitn:
    break;

    case Ailits:
    case Ailitns:
    break;

    case Aflit:
      flitp = flits + ni;
    break;

    case Aslit:
      slitp = slits + ni;
    break;

    case Atru:
    case Afal: break;

    case Akwd: break;

    case Aop: break;

    // expressions
    case Auexp:
      uexpp = uexps + ni;
      enn = uexpp->e;
      enh = nhs[enn];
      et = enh >> Atybit;
      ei = enh & Atymsk;
      if (pas == 0) { // bind var
        info("uexp %u %u",enn,uexpp->op);
        switch (et) {
          case Aid: // todo bind var

            break;
          case Ailit: gp->ty = Yint; break;
          case Aflit: gp->ty = Yflt; break;
          default:    psh(nn,1); nn = enn; goto next;
        }
      } else {
        gp->ty = gs[enn].ty; break;
      }
    break;

    case Apexp: break; // replaced to Abexp

    case Abexp:
      bexpp = bexps + ni;

      rnn = bexpp->r;
      lnn = bexpp->l;
      rnh = nhs[rnn];
      lnh = nhs[lnn];
      rt  = rnh >> Atybit;
      lt  = lnh >> Atybit;
      ri  = rnh & Atymsk;
      li  = lnh & Atymsk;
      breg = gp->res;

      if (pas == 0) {
        if (lt > Aval && rt > Aval) {
          psh(nn,1);
          psh(rnn,0);
          gs[lnn].res = breg;
          gs[rnn].res = breg+1;
          nn = lnn; goto next;
        }

        lty = rty = 0;
        switch (lt) {
          case Aid: // todo bind var
            break;
          case Ailit:
            lty = Yint;
            ival1 = ilits[li].val;
            mkop(0,0,ival1); // ld r0
            break;
          case Ailits:
            lty = Yint;
            ival1 = li;
            break;
          case Aflit:
            lty = Yflt;
            fval1 = flits[li].val;
            break;
          case Avar:
          case Auexp:
            psh(nn,1);
            gs[lnn].res = breg;
            nn = lnn; goto next;
          default: break;
        }
        gp->ty = lty;

        switch (rt) {
          case Aid: // todo bind var
            break;
          case Ailit:
            rty = Yint;
            ival2 = ilits[ri].val;
            mkop(0,0,ival2); // ld r0
            break;
          case Ailits:
            rty = Yint;
            ival2 = ri;
            break;
          case Aflit:
            rty = Yflt;
            fval2 = flits[ri].val;
            break;
          case Avar:
          case Auexp:
            psh(nn,1);
            gs[rnn].res = breg;
            nn = rnn; goto next;
          default: break;
        }
        gp->ty = rty;

        if (lty && rty) { // 1 + 1
          lni = lnh & Atymsk;
          rni = lnh & Atymsk;
          if (lty == Yint) {
             if (rty == Yint) {
               nhs[nn] = lnn;
               ilits[lni].val = beval(ival1,bexpp->op,ival2);
             }
           }
        }

      } else { // pas 1

        switch (lt) {
          case Auexp:
            gp->ty = gs[lnn].ty;
            break;
          default: break;
        }

        switch (rt) {
          case Auexp: gp->ty = gs[lnn].ty; break;
          default: break;
        }

      }
    break;

    case Aaexp: // a := b todo
      aexpp = aexps + ni;

      enn = aexpp->e;
      enh = nhs[enn];

      et = enh >> Atybit;
      ei = enh & Atymsk;

      info("aexp %u",ei);
      nn = enn;

    goto next;

    // statements
    case Aasgnst: // a[i] = 3
      asgnstp = asgnsts + ni;
      enn = asgnstp->e;
      if (pas == 0) {

        tnn = asgnstp->tgt;
        tnh = nhs[tnn];
        tt  = tnh >> Atybit;
        tni = tnh & Atymsk;

        gs[tnn].ty = gs[enn].ty;

        if (tt == Aid) { // common a =
          idp = ids + tni;
          uid = idp->id;
          lvlmsk = lvlvars[uid];
          lvv = lvl;
          while ( (lvlmsk & (1U << lvv)) == 0) lvv--;
          if (lvv) {
            vid = idvars[lvv * uid];
            varp = vars + vid;
          } else {
            lvv = lvl;
            vid = curvid++;
            lvlvars[uid] |= (1U << lvv);
            var2uid[vid] = vid;
            varp = vars + vid;
            varp->id = tni;
            varp->ofs = vid - vidf0;
            sinfo(idfpos[tni],"new var %u@%u lvl %u",uid,vid,lvl);
          }
          varp->lvl = lvv;
          var2uid[vid] = uid;
          nhs[tnn] = (Avar << Atybit) | vid;
        }

        enh = nhs[enn];
        et  = enh >> Atybit;
        ei  = enh & Atymsk;

        switch (et) { // rhs expr
          case Aid:
            idp = ids + ei;
            uid = idp->id;
            lvlmsk = lvlvars[uid];
            lvv = lvl;
            while ( (lvlmsk & (1U << lvv)) == 0) lvv--;
            if (lvv == 0) serror(idfpos[ei],"unknown var %u at lvl %u",uid,lvl);
            vid = idvars[lvv * uid];

            break;
          case Ailit: gp->ty = Yint; break;
          case Aflit: gp->ty = Yflt; break;
          case Avar: break;
          default:
            psh(nn,1);
            gs[enn].res = 0;
            nn = enn; goto next; // exp
        }

      } else if (pas == 1) { // after rhs in r0
        tnn = asgnstp->tgt;
        tnh = nhs[tnn];
        tt  = tnh >> Atybit;
        tni = tnh & Atymsk;

        gs[tnn].ty = gs[enn].ty;

        if (tt == Avar) { // common a = todo bind
          ofs = vars[tni].ofs;
          mkop(0,0,ofs);
        } else {
          nn = tnn; pas = 2; goto next; // a[i] =
        }

      } else if (pas == 2) { // after tgt
      }
    break;

    case Ablk:
      blkp = blks + ni;
      if (pas == 0) {
        info("blk %u",nn);
        psh(nn,1);
        varscs[++lvl].v0 = vid;
        blkbit <<= 1;
        nn = blkp->s;
        goto next;
      } else { // clear out-of-scope vars
        vlp = varscs + lvl;
        vp0 = vlp->v0;
        nxv0 = vlp[1].v0;
        if (nxv0 != hi32) {
          for (vp = vp0; vp < nxv0; vp++) {
            uid = var2uid[vp];
            lvlvars[uid] &= ~blkbit;
          }
          for (vp = nxv0 + vlp[1].n; vp < vp0 + vlp->n; vp++) {
            uid = var2uid[vp];
            lvlvars[uid] &= ~blkbit;
          }
        } else {
          for (vp = vp0; vp < vid; vp++) {
            uid = var2uid[vp];
            lvlvars[uid] &= ~blkbit;
          }
        }
        lvl--;
        blkbit >>= 1;
      }
    break;

/* while (expr) trustmt else falstmt
1 :head
2  a = expr()
3  bne a, :false
4 :true
5  stmts
6  jmp :head
7 :false
8  stmts
9 :end   */
    case Awhile:
      witerp = witers + ni;
      breg = gp->res;
      if (pas == 0) {
        witerp->lvl = loplvl++;
        witerp->head = mklb(pc,0); // 1
        enn = witerp->e;
        enh = nhs[enn];
        et  = enh >> Atybit;
        ei  = enh & Atymsk;

        switch (et) {
        case Atru: pas = 2; goto next;
        case Afal: pas = 3; goto next;
        case Avar:
          // mkldop(reg,vars[ei].ofs);
          pas = 1; goto next;
        case Ailit: if (ilits[ei].val) pas = 2; else pas = 3; goto next;
        case Auexp:
        case Abexp:
        case Arexp:
          psh(nn,1);
          nn = enn; gs[enn].res = breg; goto next;
        default: break;
        }

      } else if (pas == 1) { // expr

        witerp->bcc = mkop(0,breg,0); // 3, bnz patch after tru stmt
        mklb(pc,ni); // 4

        psh(nn,2);
        tru = witerp->tb;
        nn = tru; pas = 0; // 5

      } else if (pas == 2) { // tru body
        head = witerp->head;
        mkop(0,head,0); // 6 jmp
        setlb(witerp->bcc,pc);
        loplvl--;
        fal = witerp->fb;
        if (fal != hi32) {
          mklb(0,0);
          psh(nn,3); nn = fal; pas = 0; goto next;
        } else {
          witerp->tail = pc;
          break;
        }

      } else if (pas == 3) { // fal body
        witerp->tail = pc;
      }
    break;

    case Aparam: // todo
    break;

    case Afndef:
      fndefp = fndefs + ni;
      if (pas == 0) {
        psh(nn,1);
        fndefp->parfn = ni;
        fndefp->parvid = vidf0;
        vidf0 = fndefp->vid0 = curvid;
        fndefp->pc0 = pc;
        fnlvl++;
        prmlst = fndefp->plst;
        idp = ids + (fndefp->id & Atymsk);
        id = idp->id;
        ret = fndefp->ret;
        if (prmlst != hi32) {
          plnh = nhs[prmlst];
          plni = plnh & Atymsk;
          prmlp = prmls + plni;
          pos = prmlp->pos;
          cnt = prmlp->cnt;
          fndefp->argc = cnt;
          fac = 0;
          if (cnt < 2) ice(0,0,"type %u stmt list %u",t,cnt);
          do {
            ss = repool[pos+fac];
            sn = nhs[ss];
            st = sn >> Atybit;
            si = sn & Atymsk;
            switch (st) {
            case Aid: // todo bind
              idp = ids + si;
              varp = vars + curvid;
              nhs[ss] = curvid | (Avar << Atybit);
              info("new fn par %u at lvl %u",curvid,lvl);
              break;
            default: break;
            }
          } while (fac < cnt);
        }
        blkni = fndefp->blk;
        nn = blkni;
        goto next;
      } else if (pas == 1) {
        fndefp->vidcnt = curvid - fndefp->vid0;
        fndefp->pc1 = pc;
        fnlvl--;
        fndefp = fndefs + fndefp->parfn;
        vidf0 = fndefp->vidf0;
      }
    break;

    case Arexp:
      rexpp = rexps + ni;
      cnt = rexpp->n;
      ops = repool + rexpp->pos;
      breg = gp->res;

      oi = rexpp->opndx;

      if (oi < cnt) {
        x4 = ops[oi];
        op = x4 >> Obbit;
        reg = (x4 >> Regbit) & Regmsk;
        rexpp->opndx = oi + 1;
        if (op == Obcnt) { // eval req
          psh(nn,0);
          en = x4 & Obmsk;
          nn = en;
          gs[en].res = reg + breg;
          goto next;
        }
        reg1 = x4 & Regmsk;
        mkop(op,reg1,reg); // r0 = r1 op r2
        goto next;
      }
    break;

    case Aprmlst: // handled at fndef ?
    break;

    // lists
    case Astmts:
      stmtlp = stmtls + ni;
      cnt = stmtlp->cnt;
      pos = stmtlp->pos;

      if (cnt < 2) ice(0,0,"type %u stmt list %u",t,cnt);

      do {
        cnt--;
        nn = repool[pos+cnt];
        nh = nhs[nn];
        nt = nh >> Atybit;
        switch (nt) {
          case Ailit: case Ailits:
          case Aflit:
          case Aslit:
          case Akwd: break;
          default: psh(nn,0);
        }
      } while (cnt);

      break;

    case Acount: return seqs;
    } // switch (typ)

  } while (1);

  return seqs;
}

void *mkast(struct synast *sa,struct lexsyn *lsp)
{
  struct ast *ap = minalloc(sizeof(struct ast),0,0,"ast");

  struct rnode *rp,*rpp = sa->nodes;
  ub4 *args = sa->args;
  ub8 val=0,*vals = sa->vals;
  ub4 vi=0;
  ub4 nvar,nuex,nbex;
  ub4 repos=0;
  ub4 a=0,a0,a1,ni,ni0,i,ii,i0,ai;
  ub4 fpos,fpos_ilit=0;
  ub2 dfp;
  ub4 bits=0;
  ub1 atr;
  const ub8 *tkbits = lsp->bits;
  ub2 am,ac,ainc,la;
  enum Production ve;

  ub4 rni = 0;
  ub4 gi,lnn,rnn;
  ub4 nh,a0h,a1h,lnh,rnh;
  ub4 li,ri;
  enum Astyp t,t0,lt,rt;
  enum Bop bop;
  enum Uop uop;

  ub4 cnt,cnt2=0,len;

  ub4 fxs;
  int fxp;
  ub4 ival1,ival2;
  double fval;

  bool emit = (globs.emit & Astpas);

  sassert(Acount <= Atymsk,"atypes");
  sassert(Obcnt <= Aopmsk,"opbits");
  sassert(Acount < 32,"Aback");

  ub4 rndcnt = sa->ndcnt;

  ub4 aidcnt = ap->aidcnt;

  ub4 uidcnt = sa->uidcnt;
  ub4 idcnt = sa->idcnt;

  ub4 *ndcnts = sa->ndcnts;
  ub4 *rep2cnts = sa->rep2cnts;

  ub4 nid = ndcnts[Aid];

  if (nid != idcnt) ice(0,0,"idcnt %u vs %u,",idcnt,nid);

  ub4 nilit = ndcnts[Ailit];
  ub4 nilitn = ndcnts[Ailitn];
  ub4 nflit = ndcnts[Aflit];

  ub4 nuexp = ndcnts[Auexp];
  ub4 naexp = ndcnts[Aaexp];

  ub4 npexp = ndcnts[Apexp];

  ub4 nrexp = ndcnts[Arexp];
  ub4 nbexp = rep2cnts[Arexp];

  ub4 nwiter = ndcnts[Awhile];

  ub4 nblk  = ndcnts[Ablk];

  ub4 nasgnst = ndcnts[Aasgnst];

  ub4 nstmtlst = ndcnts[Astmts];
  ub4 histlstsiz = 0;

  struct mempart ndpart[Acount];

  if (nstmtlst == 0) ice(hi32,0,"nil stmt list for %u nodes",ap->len);

  ub4 *nhs = sa->nhs;
  ub4 repcnt=0;

  for (t = 0; t < Acount; t++) {
    cnt = ndcnts[t];
    cnt2 += cnt;
    if (cnt) info("%3u type %u %s",a,t,atynam(t));
    if (t >= Arexp) repcnt += cnt;
    switch (t) {
    case Apexp: cnt = 0; break;
    case Arexp: repcnt += 2 * cnt; break;
    case Aprmlst:
    case Astmts: repcnt += cnt; break;
    default: break;
    }
    ndpart[t].nel = cnt;
  }
  info("aidcnt %u / %u",aidcnt,cnt2);
  ndpart[Avar].nel = nid;

  struct agen *genp,*gens = alloc(aidcnt,struct agen,Mnofil,"ast gen",nextcnt);

  ndpart[Aid].siz = sizeof(struct aid);
  ndpart[Avar].siz = sizeof(struct var);

  ndpart[Ailit].siz = sizeof(struct ilit);
  ndpart[Ailitn].siz = sizeof(struct ilit);
  ndpart[Aflit].siz = sizeof(struct flit);

  ndpart[Auexp].siz = sizeof(struct uexp);
  ndpart[Aaexp].siz = sizeof(struct aexp);

  ndpart[Aasgnst].siz = sizeof(struct asgnst);

  ndpart[Ablk].siz = sizeof(struct blk);
  ndpart[Awhile].siz = sizeof(struct witer);

  ndpart[Afndef].siz = sizeof(struct fndef);
  ndpart[Aparam].siz = sizeof(struct param);

  ndpart[Arexp].siz = sizeof(struct rexp);
  ndpart[Aprmlst].siz = sizeof(struct prmlst);
  ndpart[Astmts].siz = sizeof(struct stmtlst);

  void *ndbas = allocset(ndpart,Acount,Mnofil,"ast tree",nextcnt);

  struct aid *idp,*ids = ndpart[Aid].ptr;
  struct var *varp,*vars = ndpart[Avar].ptr;

  struct ilit *ilitp,*ilits = ndpart[Ailit].ptr;
  struct ilit *ilitnp,*ilitns = ndpart[Ailit].ptr;
  struct flit *flitp,*flits = ndpart[Aflit].ptr;
  struct slit *slitp,*slits = ndpart[Aslit].ptr;

  struct uexp *uexpp,*uexps = ndpart[Auexp].ptr;
  struct aexp *aexpp,*aexps = ndpart[Aaexp].ptr;

  struct asgnst *asgnstp,*asgnsts = ndpart[Aasgnst].ptr;

  struct blk *blkp,*blks = ndpart[Ablk].ptr;

  struct witer *witerp,*witers = ndpart[Awhile].ptr;

  struct fndef *fndefp,*fndefs = ndpart[Afndef].ptr;
  struct param *prmp,*prms = ndpart[Aparam].ptr;

  struct rexp *rexpp,*rexps = ndpart[Arexp].ptr;
  struct prmlst *prmlp,*prmls = ndpart[Aprmlst].ptr;
  struct stmtlst *stmtlp,*stmtls = ndpart[Astmts].ptr;

  struct pexp *pexpp,*pexps = alloc(npexp,struct pexp,Mnofil,"ast pexp",nextcnt);
  struct bexp *bexpp,*bexps = alloc(nbexp + npexp,struct bexp,Mnofil,"ast bexp",nextcnt);

  ub4 *rep,*repool = alloc(repcnt,ub4,Mnofil | Mo_ok0,"ast rep",nextcnt);

  ub4 *idfpos = alloc(nid,ub4,Mnofil | Mo_ok0,"ast id",nextcnt);

  memcpy(ap->ndcnts,ndcnts,Acount * ub4siz);

  ap->gens = gens;
  ap->nhs = nhs;

  ap->uexps = uexps;
  ap->bexps = bexps;
  ap->aexps = aexps;

  ap->rexps = rexps;

  ap->ids = ids;
  ap->vars = vars;

  ap->ilits = ilits;
  ap->flits = flits;
  ap->slits = slits;

  ap->asgnsts = asgnsts;

  ap->fndefs = fndefs;
  ap->prms = prms;

  ap->witers = witers;

  ap->prmls = prmls;
  ap->stmtls = stmtls;

  ap->repool = repool;

  ap->vars = vars;

  ap->nid = nid;

  ub2 hiblklvl = sa->hiblklvl;

  ub4 lfargs[Nodarg];

  rp = rpp;
  ai = 0;

  do {
    rp = rpp + rni;

    ve = rp->ve;

    am = rp->amask;

    info("ni %u lvl %u ve %u args %x",rni,rp->lvl,ve,am);

    gi = rp->ni;
    nh = nhs[gi];

    t = nh >> Atybit;
    i = (nh >> Aopbit) & Aopmsk;

    ni = nh & Atymsk;

    ac = 0;
    a0 = args[0];
    a1 = args[1];
    a0h = nhs[a0];
    if ( (a0h >> Atybit) < Aleaf) lfargs[ac++] = a0;
    ainc = 2;

    switch (t) {

    // leaves
    case Aid:
    case Avar:
    case Ailit:
    case Ailitn:
    case Aflit:
    case Aslit:
    case Ailits:
    case Ailitns: break;

    case Atru:
    case Afal:
    case Akwd:   break;

    case Aop:    break;

    // exp nodes
    case Apexp:
      a1h = nhs[a1];
      pexpp = pexps + ni;
      pexpp->e = a0;
      pexpp->op = a1h >> Aopbit;
    break;

    case Auexp:
      uexpp = uexps + ni;
      uexpp->e = a0;
      a1h = nhs[a1];
      uexpp->op = a1h >> Aopbit;
    break;

    case Abexp: break;

    case Aaexp:
      aexpp = aexps + ni;
      aexpp->id = a0;
      aexpp->e = a1;
    break;

    case Aasgnst:
      asgnstp = asgnsts + ni;
      asgnstp->tgt = a0;
      asgnstp->e = a1;
    break;

    case Ablk:
      blkp = blks + ni;
      blkp->s = a0;
      ainc = 1;
    break;

    // iters
    case Awhile:
      witerp = witers + ni;
      witerp->e = a0;
      witerp->tb = a1;
      if (am & 4) {
        witerp->fb = args[2];
        ainc = 3;
      }
    break;

   case Afndef:
     fndefp = fndefs + ni;
     fndefp->id = a0;
     fndefp->plst = a1;
     fndefp->blk = args[2];
   break;

   case Aparam:
     prmp = prms + ni;
     prmp->id = a0;
     if (am & 2) {
       prmp->t = a1;
     } else ainc = 1;
     if (am & 4) {
       prmp->def = args[2];
       ainc++;
     }
    break;

    // lists
    case Arexp: // a + b + ...
      cnt = *args++;

      if (cnt == 2) { // a + b
        lnn = *args++;
        lnh = nhs[lnn];
        lt = lnh >> Atybit;
        li = lnh & Atymsk;

        pexpp = pexps + li;
        bop = pexpp->op;

        rnn = *args++;
        rnh = nhs[rnn];
        rt = rnh >> Atybit;
        ri = rnh & Atymsk;

        if (lt == Ailit && rt == Ailit) { // 1 + 1
          ival1 = ilits[li].val;
          ival2 = ilits[ri].val;
          ilits[li].val = beval(ival1,bop,ival2);
          nhs[gi] = lnn;
        } else { // a + b
          bexpp = bexps + nbexp; // new bexp node
          bexpp->l = pexpp->e;
          bexpp->op = bop;
          bexpp->r = pexps[ri].e;
          nhs[gi] = Abexp << Atybit | nbexp++;
        }
        ainc = 2;
      } else {
        rexpp = rexps + ni;
        rexpp->pos = repos;
        precexp(ap,rexpp,repool + repos,args,cnt);
        repos += cnt * 2;
        ainc = cnt;
      }
    break;

    case Aprmlst:   cnt = *args++;
                    prmlp = prmls + ni;
                    prmlp->pos = repos;
                    prmlp->cnt = cnt;
                    memcpy(repool + repos,args,cnt * 4);
                    repos += cnt;
                    ainc = cnt;
    break;

    case Astmts:
      cnt = *args++;
      if (cnt == 0) ice(0,0,"node %u empty stmt list",gi);
      stmtlp = stmtls + ni;
      stmtlp->cnt = cnt;
      stmtlp->pos = repos;
      if (cnt == 1) {
        a0 = *args;
        a0h = nhs[a0];
        t0 = a0h >> Atybit;
        ni0 = a0h & Atymsk;
        nhs[gi] = a0;
        break;
      }
      memcpy(repool + repos,args,cnt * 4);
      repos += cnt;
      ainc = cnt;
      if (cnt > histlstsiz) histlstsiz = cnt;
    break;

    case Acount: goto end;
    }
    args += ainc;

    for (la = 0; la < ac; la++) { // handle leaf args
      a0 = lfargs[la];
      a0h = nhs[a0];
      t0 = a0h >> Atybit;
      ni0 = a0h & Atymsk;
      atr = (a0h >> Aopbit) & Aopmsk;

      if (t0 <= Aval) val = vals[vi++];

      switch (t0) {
      case Aid:
        idp = ids + ni0;
        idp->id = val & hi32;
        idfpos[ni0] = val >> 32;
      break;

      case Ailit:
        ilitp = ilits + ni;
        ilitp->val = val;
        break;

      case Ailitn:
        ilitnp = ilitns + ni;
        ilitnp->val = val;
        break;

      case Aflit:
        flitp = flits + ni;
        fval = (double)(val & hi56);
        if (val & Bit63) fval = -fval;
        fxs = val >> 62;
        fxp = (val >> 55) & 0xff;
        if (fxp == 1 && fxs == 0) fval *= 10;
        else if (fxp) {
          if (fxs) fxp = -fxp;
          fval *= exp(10 * log(fxp));
        }
        flitp->val = fval;
        break;

      case Aslit: // todo
        break;

      case Ailits:
      case Ailitns:
      break;

      case Akwd: break;
      default: break;

    }
  } // postprocess leaf args

    rni++;
    gi++;
  } while (1);

// ---
  end:
// ---

  ap->root = nstmtlst - 1;
  ap->histlstsiz = histlstsiz;

  afree(pexps,"ast pexp",nextcnt);

  ub4 *seqs = process(ap,emit);
  ub4 pos=0,blen = 4096;
  char buf[4096];
  ub2 lvl=0;
  ub4 x4;
  ub4 nn,enn;
  ub4 enh;
  ub4 ei;
  ub4 id;
  ub1 pas;
  enum Astyp et;

  if (emit == 0) return ap;

  vi = 0;
  lvl = 0;
  do {
    x4 = seqs[vi++];
    nn = x4 & hi28;
    pas = x4 >> 28;

    nh = nhs[nn];
    t = nh >> Atybit;
    ni = nh & Atymsk;
    genp = gens + nn;

    if (pas == 0) {
      pos = mysnprintf(buf,0,blen,"%3u %3u %u %*s %-6s ",nn,ni,pas,lvl," ",atynam(t));
    } else continue;

    switch(t) {

    // leaves
    case Avar: break;

    case Aid: // todo x
      idp = ids + ni;
      idp->lvl = lvl;
      id = idp->id;
    break;

    case Ailit:
    case Ailitn:
      ilitp = ilits + ni;
      pos += mysnprintf(buf,pos,blen,"%lu",ilitp->val);
    break;

    case Ailits:
    case Ailitns:
      info("ilits %u",ni);
    break;

    case Aflit:
      flitp = flits + ni;
      info("flit %e",flitp->val);
    break;

    case Aslit:
      slitp = slits + ni;
      info("slit %u",slitp->val);
    break;

    case Atru:
    case Afal: break;

    case Akwd: break;

    case Aop: break;

    // expressions
    case Auexp:
      uexpp = uexps + ni;
      enn = uexpp->e;
      enh = nhs[enn];
      et = enh >> Atybit;
      ei = enh & Atymsk;
    break;

    case Apexp: break; // replaced to Abexp

    case Abexp:
      bexpp = bexps + ni;

      rnn = bexpp->r;
      lnn = bexpp->l;
      rnh = nhs[rnn];
      lnh = nhs[lnn];
    break;

    case Aaexp: // a := b todo
      aexpp = aexps + ni;

      enn = aexpp->e;
      enh = nhs[enn];

      et = enh >> Atybit;
      ei = enh & Atymsk;

      info("aexp %u",ei);
      nn = enn;

    break;

    // statements
    case Aasgnst: // a[i] = 3
      asgnstp = asgnsts + ni;
      enn = asgnstp->e;
    break;

    case Ablk:
      blkp = blks + ni;
      info("blk %u",nn);
    break;

    case Awhile:
      witerp = witers + ni;
    break;

    case Aparam: // todo
    break;

    case Afndef:
      fndefp = fndefs + ni;
    break;

    case Arexp:
      rexpp = rexps + ni;
      cnt = rexpp->n;
//      ops = repool + rexpp->pos;

    break;

    case Aprmlst: // handled at fndef ?
    break;

    // lists
    case Astmts:
      stmtlp = stmtls + ni;
      cnt = stmtlp->cnt;
      pos = stmtlp->pos;

      break;

    case Acount: return ap;
    } // switch (typ)

  } while (1);

//  if (globs.runast) runast(ap,emit);

  return ap;
}

enum Astyp prd2nod[Pcount] = {
  [Ppexp]   = Apexp,
  [Puexp_0]  = Auexp,
  [Puexp_1]  = Auexp,
//  [Pasexp]  = Aaexp,
  [Pblock]  = Ablk,
  [Pwhile]  = Awhile,
  [Pfndef]  = Afndef,
  [Pparam]  = Aparam,
  [Prexp]   = Arexp,
  [Pgrpexp] = Apexp,
  [Pprmlst] = Aprmlst,
  [Pasgnst] = Aasgnst,
  [Pstmt_0] = Astmts
};
