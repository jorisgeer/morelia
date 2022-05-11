# python.lex - lexical analysis for python language

#  This file is part of Morelia, a subset of Python with emphasis on efficiency.

#  Copyright © 2022 Joris van der Geer.

#  Morelia is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.

#  Morelia is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU Affero General Public License
#  along with this program, typically in the file License.txt
#  If not, see http://www.gnu.org/licenses.

version 0.1.0
author joris
language minipython
requires genlex 1.0

eof while

token
# group 0 - storable terms
  id
  nlit
  slit

# group 1 - operators
  ast 1
  pm 1
  op 1

# group 2 - scope in
  co 2

# group 3 - scope out
  cc 3

  ro 4
  rc 4
#  so
#  sc
  qas 4
#  aas
  das 4
#  mulas
  colon 4
  sepa 4
  comma 4
#  dot
#  exp

set
  af _a-zA-Z
  nm 0123456789
  ws \20
  ht \09
  cr \0d
  nl \0a
  hs #
  qq '"
  co {
  cc }
#  so [
#  sc ]
  ro (
  rc )
  cl :
  sm ;
  ca ,
  dt .
  pm +-
  st *
  eq =
  op @%&^|
  o1 /
  o4 <
  o5 >
  o9 !
  EOF \00
 +an _a-zA-Z+nm
 +ee eE
 +oo oO
 +xx xX
 +hx abcdefABCDEF

keyword
#  and
#  as
  break 1
#  continue
  def
#  del
#  elif
  else
#  for
#  from
  if
#  import
#  in
#  is
#  not
#  or
#  pass
#  return
  while
#  with

action

# ----------------------
# int literal. < 4G decoded in bits, else orig ascii
# ----------------------
ilit
1 ilitcnt++;
2 if (litbin) { if (u4v < Ilit4) atrs[dn] = u4v; else { atrs[dn] = Ilit4; bits[bn++] = u4v; }
2 } else {
2   bits[bn++] = nlitpos; atrs[dn] = Ilita;
2   memcpy(nlitpool+nlitpos,sp+N,n-N); nlitpool[nlitpos++] = 0;
2   nlitpos += n-N;
2   litbin = 1;
2 }
1 nlitpos += n-N;
#  info("ilit %u",u4v);

# ----------------------
# float literal. < 4G decoded in bits, else orig ascii
# ----------------------
flit
1 flitcnt++;
2 if (litbin && fxp + fdp < E16max) fxp += fdp; else litbin = 0;
2 if (litbin) {
2   atrs[dn] = Flit8;
2   bits[bn++] = (fxs << 22) | (fxp << 21) | u4v;
2 } else {
2   bits[bn++] = nlitpos; atrs[dn] = Flita;
2   memcpy(nlitpool+nlitpos,sp+N,n-N); nlitpool[nlitpos++] = 0;
2   nlitpos += n-N; litfpi=0; litbin = 1;
2 }
2 fxp=0;fxs=0;fdp=0;
1 nlitpos += n-N;
#  info("flit %u",u4v);

doflitfr
2 if (litbin && u4v < D32max) u4v = u4v * 10 + c - '0'; else litbin = 0;
2 if (litbin && fdp < E16max) fdp++; else litbin = 0;

doslit0
2 slitx=slitpos; slitpool[slitx++] = c;

doslit
2 slitpool[slitx++] = c;

# ----------------------
# \xx in string literal
# ----------------------
`A doesc
  c = sp[n];
  x1 = esctab[c];
  switch(x1) {
  case Esc_nl: lntab[l] = n; nlcol=n; break;
.
  case Esc_o: x1=doesco(sp,n); slitpool[slitx++] = x1; n += 3; break;
  case Esc_x: x1=doescx(sp,n); n += 2;
              if (x1 < 0x80 || (slitctl & Slit_b) ) slitpool[slitx++] = x1;
              else {
                slitpool[slitx]   = 0xc0 | (x1 >> 6);
                slitpool[slitx+1] = 0x80 | (x1 & 0x3f);
                slitx += 2;
              }
              break;
.
  case Esc_u:
  case Esc_U:
  case Esc_N: if (slitctl & Slit_b) {
                swarn(n,"unrecognised escape sequence '\\%s'",chprint(c));
                slitpool[slitx] = '\\';
                slitpool[slitx+1] = 'c';
                n += 2; slitx += 2;
              } else n=doescu(sp,n,slitpool,&slitx,x1);
              break;
.
  case Esc_inv: swarn(n,"unrecognised escape sequence '\\%s'",chprint(c));
                slitpool[slitx] = '\\';
                slitpool[slitx+1] = 'c';
                n += 2; slitx += 2;
                break;
.
  default:    slitpool[slitx++] = x1; n++;
  }

# ----------------------
# end of string literal
# ----------------------
slit
  R0=0;
2 len = slitx - slitpos;
2 switch(len) {
2 case 1: x1 = slitpool[slitpos]; if (x1 < Slit_len) { atrs[dn] = x1; break; } Fallthrough
2 case 2:
2 case 3: bits[bn] = *(ub4 *)(slitpool + slitpos);
2         atrs[dn] = len | Slit_len;
2         break;
2 default:
2   id = slitgetadd(slitpool,slitx);
2   bits[bn++] = id;
2   atrs[dn] = min(len,Slit_len - 1) | Slit_len;
2 }
.
1 len = n - N;
1 switch(len) {
1 case 1: slit1cnt++; break;
1 case 2: slit2cnt++; break;
1 case 3: slit3cnt++; break;
1 default:
1   if (len < Slitint) slitcnt++; else slitxcnt++;
1   slitpos += len;
1 }

# ----------------------
# ident or kwd
# ----------------------
id1
1 id1cnt++;
2 id = id1getadd(prvc1);
2 atrs[dn] = id;

id2
  kwd = lookupkw2(prvc1,prvc2);
  if (kwd < T99_kwd) {
1   dn++;
2   tk = kwd;
  } else {
1   id2cnt++;
1   id2chr[prvc1] = 1;
1   id2chr[prvc2] = 2;
2   tk = Tid;
2   id = id2getadd(prvc1,prvc2);
2   bits[bn++] = id;
2   atrs[dn] = Idlen_2;
  }
2 tks[dn] = tk; setfpos(dn,n)
2 dn++;

id
1 len = n - N;
1 hc = hashalstr(sp+N,len,Hshseed);
2 len = idxpos - idnampos;
2 hc = hashalstr(idnampool+idnampos,len,Hshseed);
1 if (len > idhilen) idhilen = len;
1 idcnt++;
1 idnplen += len;
1 exp_first0(hc);
2 kw = lookupkw(len,hc);
2 if (kw < t99_count) { setkwd(tk,atrs[dn],kw); if (Tkgrp > 1) tkgrps[kwgrps[tk]]++; }
2 else {
2   tk = Tid;
2   blt = lookupblt(len,hc);
2   if (blt < B99_count) { bits[bn++] = blt; atrs[dn] = Idctl_blt; bltcnt++; }
2   else {
2     x4 = idgetadd(idxpos,hc);
2     bits[bn++] = x4;
2     atrs[dn] = Idlen_n;
2   }
2 }
2 tks[dn] = tk; setfpos(dn,n)
2 dn++;

# ----------------------
# newline: maintain line table
# ----------------------
donl
2 lntab[l] = n;
  l++;
  nlcol = n;

# ----------------------
# indent / dedent - create { }
# ----------------------
dodent
  hasdent=0;
  if (bolvl == 0) {
  dent = n - nlcol;
  if (dent > dentst[dentlvl]) {
    if (dentlvl < Dent-1) dentlvl++;
    dentst[dentlvl] = dent;
2   tks[dn] = Tco; setfpos(dn,n)
    dn++;
  } else {
    while (dent < dentst[dentlvl]) {
2     tks[dn] = Tcc; setfpos(dn,n)
      hasdent=1;
      dn++;
      if (dentlvl) dentlvl--;
      else break;
    }
    if (hasdent == 0) {
2     tks[dn] = Tsepa; setfpos(dn,n)
    dn++;
    }
  }
  }

# ----------------------
# opening { [ (
# ----------------------
dobo
  if (bolvl + 1 >= Depth) lxerror(l,0,`L,c,Lxe_count);
  bolvls[bolvl] = n;
  bolvlc[bolvl] = c;
  bolvl++;

# ----------------------
# closing ]
# ----------------------
#dosc
#  if (bolvl == 0) lxerror(l,0,`L,c,Lxe_count);
#  bolvl--;
#  if (bolvlc[bolvl] != '[') lxerror(l,0,`L,c,Lxe_count);

# ----------------------
# closing )
# ----------------------
dorc
  if (bolvl == 0) lxerror(l,0,`L,c,Lxe_count);
  bolvl--;
  if (bolvlc[bolvl] != '(') lxerror(l,0,`L,c,Lxe_count);

# ----------------------
# closing }
# ----------------------
docc
  if (bolvl == 0) lxerror(l,0,`L,c,Lxe_count);
  bolvl--;
  if (bolvlc[bolvl] != '{') lxerror(l,0,`L,c,Lxe_count);

INIT
  bolvl = 0;

# ---------------------
table
# ---------------------

# state lead pat

# ---------------------
# initial state
# ---------------------
root

# pat nxstate token action

# whitespace and newlines
  `ws
  cr
  nl sol . donl

# line comments
  hs cmt

# int literals - different from slit prefix

  # single digit
  nm. flitfr . N=n;\
             2. u4v = c - '0'; fdp=0;

  nmee flitxp0 . N=n;\
             2. u4v = c - '0';

  0 zilit0 . N=n;\
             2. u4v = 0;

  nm filit . N=n;\
             2. u4v = c - '0';

# identifier
  af id1 3.id .prvc1 = c;\
              1. N=n;

# string literal
  qq slit0 . .Q=c;\
             1.N=n;

# float literal
  .nm flitfr . N=n;\
               2. u4v=0; fdp=0;

# brackets
  { . co dobo
  } . cc docc

#  [ . so dobo
#  ] . sc dosc

  ( . ro dobo
  ) . rc dorc

  := . qas

# punctuators
  : . colon
  ; . sepa

  , . comma
#  . . dot

# operators
#  ** . exp
#  *= . mulas
  op .   . 2. atrs[dn] = c;
  * . ast 2. atrs[dn] = Lomul;
  / oper . 2. atrs[dn] = Lodiv;
  < oper . 2. atrs[dn] = Lolt;
  > oper . 2. atrs[dn] = Logt;
  ! .    . 2. atrs[dn] = c;

# pm= . aas.

  pm . pm. 2. atrs[dn] = c;

  = . das

# escaped newline
  \`nl . . donl

  EOF EOF

# ---------------------
# 2-3 char operators
# ---------------------
oper
#  op= root aas 2. atrs[dn] |= (c << 8);
#  op  root op  2. atrs[dn] |= (c << 8);

  < root . 2. atrs[dn] = Loshl;
  > root . 2. atrs[dn] = Loshr;
  & root . 2. atrs[dn] = Loand;
  | root . 2. atrs[dn] = Loor;

#  = root aas
  ot -root op

# ---------------------
# start of line, handle indent
# ---------------------
sol
  `ws
  ht
  nl . 3.cc donl
  hs cmt
  ot -root 3.co dodent

# ---------------------
# line comment
# ---------------------
.cmt
  `nl sol . donl
  `EOF EOF
  ot

# ---------------------
# identifier or keyword. lookup in action
# ---------------------
id1
  an id2 . .prvc2 = c;
  ot -root 2.id id1

id2
  an id . 2.idxpos = idnampos + 3;\
          2.idnampool[idnampos] = prvc1; idnampool[idnampos+1] = prvc2; idnampool[idnampos+2] = c;
  ot -root . id2

id
  an . . 2.idnampool[idxpos++] = c;
#  nl root . donl\
#            id
  ot -root . id

# ---------------------
# prefixed integer literal
# ---------------------
.zilit0
  bb ilitb
  oo ilito
  xx ilitx
  _
  0
  nm filit . 2. u4v = c - '0'; ## decimal with leading zero
  . flitfr . 2. fdp=0; ## 0.
  j root 2.nlit 2. litfpi = 1;\
               flit
  ot -root 2.nlit 2. atrs[dn] = 0;\
                  1. ilit1cnt++;

# ---------------------
# float or int literal
# ---------------------
.filit
  nm . . 2. if (litbin && u4v < D32max) u4v = u4v * 10 + c - '0'; else litbin = 0;
  _
  . flitfr . 2. fdp=0;
  ee flitxp0 .   2. fxs = 0; fxp = 0;
  j root 2.nlit 2. litfpi = 1;\
               flit
  ot -root 2.nlit ilit

# ---------------------
# float literal fraction
# ---------------------
.flitfr
  nm . . doflitfr
  _
  ee flitxp0 .   2. fxs = 0; fxp = 0;
  j root 2.nlit 2. litfpi = 1;\
               flit
  ot -root 2.nlit flit

# ---------------------
# float literal exp
# ---------------------
.flitxp0
  nm flitxp . 2. if (litbin && fxp < E16max) fxp = fxp * 10 + c - '0'; else litbin = 0;
  pm flitxp . 2. fxs = (c == '-');
  _
  j root 2.nlit 2. litfpi = 1;\
               flit

  ot -root 2.nlit flit

.flitxp
  nm . . 2. if (litbin && fxp < E16max) fxp = fxp * 10 + c - '0'; else litbin = 0;
  _
  j root 2.nlit 2. litfpi = 1;\
               flit

  ot -root 2.nlit flit

# ---------------------
# int literal binary
# ---------------------
.ilitb
  0 . . 2. if (litbin && u4v < B32max) u4v <<= 1; else litbin = 0;
  1 . . 2. if (litbin && u4v < B32max) u4v = u4v << 1 | 1; else litbin = 0;
  _
  ot -root 2.nlit ilit

# ---------------------
# int literal octal
# ---------------------
.ilito
  8 Err
  9 Err
  nm . . 2. if (litbin && u4v < D32max) u4v = u4v << 3 | (c - '0'); else litbin = 0;
  _
  ot -root 2.nlit ilit

# ---------------------
# int literal hex
# ---------------------
.ilitx
  nm . . 2. if (litbin && u4v < X32max) u4v = u4v << 4 | (c - '0'); else litbin = 0;
  hx . . 2. if (litbin && u4v < X32max) u4v = u4v << 4 | ( (c | 0x20) + 10 - 'a'); else litbin = 0;
  _
  ot -root 2.nlit ilit

# todo merge adjacent strings
# ---------------------
# string literal start
# ---------------------
slit0
  QQ . . 2.slitx=slitpos;\
           R0=1; # start of long slit

  R00Q root 2.slit 1.slit0cnt++; # empty short slit ''

1 \ . . donl
`A2  \ . . 2doesc
`a2  \ . . donl
1 nl Err-str-nl
  ot slit . doslit0

# ---------------------
# string literal
# ---------------------
slit string literal
  R00Q   root 2.slit slit # end of short slit
  R01QQQ root 2.slit slit # end of long slit

1 \ . . donl
`A2  \ . . 2doesc
`a2  \ . . donl
1 nl Err-str-nl
  EOF Err
  ot . . 2doslit
