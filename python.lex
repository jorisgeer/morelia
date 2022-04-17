# python.lex - lexical analysis for python language

#  This file is part of Morelia, a subset of Python with emphasis on efficiency.

#  Copyright © 2022 Joris van der Geer.

#  Morelia is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.

#  mpy is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU Affero General Public License
#  along with this program, typically in the file License.txt
#  If not, see <http://www.gnu.org/licenses/>.

version 0.1.0
author joris
language python-3.10
requires genlex 1.0

eof while
#+scope co
#-scope cc

token
  nlit
  slit
  id
  ind 1
  ded 1
  co 1
  cc 1
  ro 1
  rc 1
  so 1
  sc 1
  qas 2
  aas 2
  das 2
  mulas 2
  colon 1
  sepa 1
  comma 1
  dot 1
  exp 2
  ast 1
  pm 2
  ref 1
  op 2

set
  af _a-zA-Z\Z
  nm 0123456789
  ws \20
  ht \09
  cr \0d
  nl \0a
  hs #
  qq '"
  co {
  cc }
  so [
  sc ]
  ro (
  rc )
  cl :
  sm ;
  ca ,
  dt .
  pm +-
  st *
  eq =
  op /%@<>&|^!
  EOF \00
 +an _a-zA-Z\z+nm
 +ee eE
 +px bBrRfFuU
# +ff fF
# +rr rR
 +oo oO
 +xx xX
# +bb bB
# +ct \09\0a\0d
 +hx abcdefABCDEF
 +mn +op+ws+ht+nl+hs+sc+rc+cl+sm+ca+pm
 +xp _acdeghijklmnopqstvwxyzACDEGHIJKLMNOPQSTVWXYZ

keyword
  and
  as
  assert
  break
  class
  continue
  def
  del
  elif
  else
  for
  from
  global|nonlocal=glob_nl
  if
  import
  in
  is
  not
  or
  pass
  return
  while
  with

action

# ----------------------
# int literal. < 4G decoded in bits, else orig ascii
# ----------------------
ilit
1 ilitcnt++;
2 if (litrep == 0) { tkbits[dn] = u4v; u4v = 0; }
2 else {
2   tkbits[dn] = Litasc | nlitpos;
2   memcpy(nlitpool+nlitpos,sp+N,n-N); nlitpool[nlitpos++] = 0;
2   nlitpos += n-N;
2   litrep = 0;
2 }
1 if (n-N > 3) nlitpos += n-N;
#  info("ilit %u",u4v);

# ----------------------
# float literal. < 4G decoded in bits, else orig ascii
# ----------------------
flit
1 flitcnt++;
2 if (fxp + fdp < E16max) fxp += fdp; else litrep = Litasc;
2 if (litrep == 0) {
2   tkbits[dn] = Litflt | litfpi | (fxs << 22) | (fxp << 21) | u4v;
2   u4v = 0;
2 } else {
2   tkbits[dn] = Litflt | Litasc | litfpi | nlitpos;
2   memcpy(nlitpool+nlitpos,sp+N,n-N); nlitpool[nlitpos++] = 0;
2   nlitpos += n-N; litfpi=0; litrep = 0;
2 }
2 fxp=0;fxs=0;fdp=0;
1 if (n-N > 3) nlitpos += n-N;
#  info("flit %u",u4v);

doflitfr
2 if (u4v < D32max) u4v = u4v * 10 + c - '0'; else litrep = Litasc;
2 if (fdp < E16max) fdp++; else litrep = Litasc;

doslit0
2 slitx=slitpos; slitpool[slitx++] = c;

doslit
2 slitpool[slitx++] = c;

# ----------------------
# \xx in string literal
# ----------------------
doesc
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
2 len = slitx - slitpos;
2 switch(len) {
2 case 1:
2 case 2:
2 case 3: tkbits[dn] = *(ub4 *)(slitpool + slitpos) | (len << 30); break;
2 default:
2   id = slitgetadd(slitpool,slitx);
2   tkbits[dn] = id;
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
2 tkbits[dn] = prvc1 | Idctl_1;

id2
2 kwd = lookupkw2(sp[N],sp[N+1]);
1 id2cnt++;
1 id2chr[prvc1] = 1;
1 id2chr[prvc2] = 2;
2 if (kwd < T99_kwd) tk = kwd;
2 else {
2   tk = Tid;
2   id = id2getadd(prvc1,prvc2);
2   tkbits[dn] = id | Idctl_2;
2 }

id
  len = idxpos - idnampos;
  hc = hashalstr(idnampool+idnampos,len,Hshseed);
1 if (len > idhilen) idhilen = len;
1 idcnt++;
1 idnplen += len;
1 exp_first0(hc);
2 kw = lookupkw(len,hc);
2 if (kw < t99_count) { setkwd(tk,tkbits[dn],kw); }
2 else {
2   tk = Tid;
2   blt = lookupblt(len,hc);
2   if (blt < B99_count) { tkbits[dn] = blt | Idctl_blt; bltcnt++; }
2   else {
2     id = idgetadd(idxpos,hc);
2     tkbits[dn] = id;
2   }
2 }
2 tks[dn] = tk; tkfpos[dn] = n;
2 dn++;
1 // info("id '%.*s'",len,sp+N);

id_
  len = idxpos - idnampos;
  if (len > 2 && idnampool[idxpos-2] == '_' && idnampool[idxpos-1] == '_') { // dunder
    hc = hashalstr(idnampool + idnampos,len-2,Hshdseed);
2    dun = lookupdun(len-2,hc);
2    if (dun < D99_count) { tkbits[dn] = dun | Idctl_dun; duncnt++; }
2    else {
2      sinfo(n,"unknown dunder '__%.*s'",len,idnampool+idnampos);
2      lxerror(l,0,`L,c,Lxe_count);
2    }
2  } else {
2     id = idgetadd(idxpos,hc);
2     tkbits[dn] = id | Idctl_cls;
  }

# ----------------------
# newline: maintain line table
# ----------------------
donl
2 lntab[l] = n;
  l++;
  nlcol = n;
  // info("ln %u col %u",l,n);

# ----------------------
# indent / dedent - create { }
# ----------------------
dodent
  hasdent=0;
  if (bolvl == 0) {
  dent = n - nlcol;
  // info("dent %u - %u = %u  lvl %u",n,nlcol,dent,dentlvl);
  if (dent > dentst[dentlvl]) {
    if (dentlvl < Dent-1) dentlvl++;
    dentst[dentlvl] = dent;
2   tks[dn] = Tind; tkfpos[dn] = n;
    // info("dent %u  lvl %u",dent,dentlvl);
    dn++;
  } else {
    while (dent < dentst[dentlvl]) {
2     tks[dn] = Tded; tkfpos[dn] = n;
      hasdent=1;
      // info("dent %u  lvl %u",dent,dentlvl);
      dn++;
      if (dentlvl) dentlvl--;
      else break;
    }
    if (hasdent == 0) {
2     tks[dn] = Tsepa; tkfpos[dn] = n;
      // info("dent %u  lvl %u",dent,dentlvl);
    dn++;
    }
  }
  }

# ----------------------
# enter expr from short f-string
# ----------------------
fstrexp1
  fstrQ = Q;
  fstrctl = slitctl;
  bolvls[bolvl] = n;
  bolvlc[bolvl] = c;
  bolvl++;
  fstrco = bolvl;

# ----------------------
# closing }: continue f-string after expr if pending
# ----------------------
fstrexp2
  if (bolvl == 0) lxerror(l,0,`L,c,Lxe_count);
  bolvl--;
  if (bolvlc[bolvl] != '{') lxerror(l,0,`L,c,Lxe_count); // ser("pos %u unmatched opening '{' at pos %u lvl %u",n,bolvls[bolvl],bolvl);
.
  if (fstrco && bolvl == fstrco) { // end of f-string exp
    Q = fstrQ;
    slitctl = fstrctl;
    fstrco = 0;
    slitx = slitpos;
    switch (slitctl & (Slit_r | Slit_f) ) {
    case Slit_f: goto `slitf0;
    case Slit_r: goto `slitr0;
    case Slit_r|Slit_f: goto `slitrf0;
    default: goto `slit0;
    }
  } else {
2   tks[dn] = Tcc; tkfpos[dn] = n; // no f-str, regular } token
    dn++;
    goto `root;
  }

# ----------------------
# opening { [ (
# ----------------------
dobo
  if (bolvl + 1 >= Depth) lxerror(l,0,`L,c,Lxe_count); // ser("exceeding nesting depth %u",Depth);
  bolvls[bolvl] = n;
  bolvlc[bolvl] = c;
  bolvl++;

# ----------------------
# closing ]
# ----------------------
dosc
  if (bolvl == 0) lxerror(l,0,`L,c,Lxe_count); // ser("pos %u missing opening '['",n);
  bolvl--;
  if (bolvlc[bolvl] != '[') lxerror(l,0,`L,c,Lxe_count); // ser("pos %u unmatched opening '[' at pos %u lvl %u ln %u",n,bolvls[bolvl],bolvl,L);

# ----------------------
# closing )
# ----------------------
dorc
  if (bolvl == 0) lxerror(l,0,`L,c,Lxe_count); // ser("pos %u missing opening '('",n);
  bolvl--;
  if (bolvlc[bolvl] != '(') lxerror(l,0,`L,c,Lxe_count); // ser("pos %u unmatched opening '(' at pos %u lvl %u ln %u",n,bolvls[bolvl],bolvl,L);

# ----------------------
# decode double string lit prefix
# ----------------------
pfx2slit
  Q=sp[n+1];
1 N=n;
.
  slitctl = 0;
  switch (c | 0x20) {
  case 'b': slitctl |= Slit_b; break;
  case 'f': slitctl |= Slit_f; break;
  case 'r': slitctl |= Slit_r; break;
  case 'u': break;
  default:  lxerror(l,0,`L,c,Lxe_count);
  }
.
  switch (sp[n] | 0x20) {
  case 'b': slitctl |= Slit_b; break;
  case 'f': slitctl |= Slit_f; break;
  case 'r': slitctl |= Slit_r; break;
  case 'u': break;
  default:  lxerror(l,0,`L,c,Lxe_count);
  }
.
  switch (slitctl & (Slit_r | Slit_f) ) {
  case Slit_f: goto `slitf0;
  case Slit_r: goto `slitr0;
  case Slit_r|Slit_f: goto `slitrf0;
  default: goto `slit0;
  }

# ----------------------
# decode single string lit prefix
# ----------------------
pfx1slit
  Q=sp[n];
1 N=n;
.
  switch (c | 0x20) {
  case 'b': slitctl = Slit_b; goto `slit0;
  case 'f': slitctl = Slit_f; goto `slitf0;
  case 'r': slitctl = Slit_r; goto `slitr0;
  case 'u': slitctl = 0; goto `slit0;
  default:  lxerror(l,0,`L,c,Lxe_count);
  }

INIT
  bolvl = 0;
  fstrco = 0;

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
  nmmn -root 2.nlit  .ilitcnt++;\
                 2. tkbits[dn] = (c - '0');

  0 zilit0 . N=n;\
             2. u4v = 0;

  nm filit . N=n;\
             2. u4v = c - '0';

# identifier, not similar to prefix
  xp id1 . .prvc1 = c;

# prefixed string literal
  pxpxqq * . pfx2slit
  pxqq   * . pfx1slit

# identifier
  af id1 3.id .prvc1 = c;

# string literal
  qq slit0 . .Q=c; slitctl = 0;\
             1.N=n;

# float literal
  .nm flitfr . N=n;\
               2. u4v=0; fdp=0;

# brackets
  { . co dobo
  } * 3.cc fstrexp2

  [ . so dobo
  ] . sc dosc

  ( . ro dobo
  ) . rc dorc

  := . qas

# punctuators
  : . colon
  ; . sepa
  , . comma
  . . dot

# operators
  ** . exp
  *= . mulas
  * . ast

  op oper . 2. tkbits[dn] = c;

  pm= . aas.

  pm . pm. 2. tkbits[dn] = c;

  = . das
  -> . ref

# escaped newline
  \`nl . . donl

  EOF EOF

# ---------------------
# operators
# ---------------------
oper
  op= root aas 2. tkbits[dn] |= (c << 8);
  op  root op  2. tkbits[dn] |= (c << 8);
  = root aas
  ot -root op

# ---------------------
# start of line, handle indent
# ---------------------
sol
  `ws
  ht
  nl . 3.ded donl
  hs cmt
  ot -root 3.ind dodent

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
  an * . .if (c == '_' && prvc1 == '_') { idxpos = idnampos; goto `id_2; } else { prvc2 = c; goto `id2; }
  ot -root 2.id id1

id2*
  an id . .idxpos = idnampos + 3;\
          .idnampool[idnampos] = prvc1; idnampool[idnampos+1] = prvc2; idnampool[idnampos+2] = c;
  ot -root 2.id id2

id_2*
  an . . .idnampool[idxpos++] = c;
  ot -root . id_

id
  an . . .idnampool[idxpos++] = c;
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
  nm filit . 2. u4v = c - '0'; ## decimal with leading zero
  . flitfr . 2. fdp=0; ## 0.
  j root 2.nlit 2. litfpi = 1;\
               flit
  ot -root 2.nlit ilit

# ---------------------
# float or int literal
# ---------------------
.filit
  nm . . 2. if (u4v < D32max) u4v = u4v * 10 + c - '0'; else litrep = Litasc;
  _
  . flitfr . 2. fdp=0;
  j root 2.nlit 2. litfpi = 1;\
               flit
  ot -root 2.nlit ilit

# ---------------------
# float literal fraction
# ---------------------
.flitfr
  nm . . doflitfr
  _
  eepm flitxp . 2. fxs = (sp[n+1] == '+'); fxp = 0;
  ee flitxp .   2. fxs = 0; fxp = 0;
  j root 2.nlit 2. litfpi = 1;\
               flit
  ot -root 2.nlit flit

# ---------------------
# float literal exp
# ---------------------
.flitxp
  nm . . 2. if (fxp < E16max) fxp = fxp * 10 + c - '0'; else litrep = Litasc;
  _
  j root 2.nlit 2. litfpi = 1;\
               flit

  ot -root 2.nlit flit

# ---------------------
# int literal binary
# ---------------------
.ilitb
  0 . . 2. if (u4v < B32max) u4v <<= 1; else litrep = Litasc;
  1 . . 2. if (u4v < B32max) u4v = u4v << 1 | 1; else litrep = Litasc;
  _
  ot -root 2.nlit ilit

# ---------------------
# int literal octal
# ---------------------
.ilito
  8 Err
  9 Err
  nm . . 2.u4v = u4v << 3 | (c - '0');
  _
  ot -root 2.nlit ilit

# ---------------------
# int literal hex
# ---------------------
.ilitx
  nm . . 2. if (u4v < X32max) u4v = u4v << 4 | (c - '0');
  hx . . 2. if (u4v < X32max) u4v = u4v << 4 | ( (c | 0x20) + 10 - 'a');
  _
  ot -root 2.nlit ilit

# ---------------------
# string literal start
# ---------------------
slit0
  QQ sllit0 . 2.slitx=slitpos;
  Q root 2.slit 1.slit0cnt++;
1 \nl . . donl
  \ . . 2doesc
  nl Err-str-nl
  ot slit . doslit0

# idem, raw
slitr0*
  QQ sllit0 . 2.slitx=slitpos;
  ~\Q root 2.slit 1.slit0cnt++;
  nl Err-str-nl
  ot slitr . doslit0

# idem, fmt
slitf0*
  QQ sllit0 . 2.slitx=slitpos;
  Q root 2.slit 1.slit0cnt++;
1 \nl . . donl
  \ . . 2doesc
  nl Err-str-nl
  ot slitf . doslit0

slitrf0*
  QQ sllit0 . 2.slitx=slitpos;
  ~\Q root 2.slit 1.slit0cnt++;
  nl Err-str-nl
  ot slitrf . doslit0

# ---------------------
# string literal
# ---------------------
slit string literal
  Q  root 2.slit slit
1 \nl . . donl
  \ . . 2doesc
  nl Err-str-nl
  EOF Err
  ot . . 2doslit

slitr string literal # raw
  ~\Q  root 2.slit slit
  nl Err-str-nl
  EOF Err
  ot . . 2doslit

slitf string literal
  Q  root 2.slit slit
1 \nl . . donl
  \ . . 2doesc
  {{ . . 2doslit
  { root . fstrexp1
  nl Err-str-nl
  EOF Err
  ot . . 2doslit

slitrf string literal
  ~\Q  root 2.slit slit
  {{ . . 2doslit
  { root . fstrexp1
  nl Err-str-nl
  EOF Err
  ot . . 2doslit

# ---------------------
# string literal long
# ---------------------
sllit0
  QQQ root 2.slit 1.slit0cnt++;
1 \nl sllit . donl
  \ sllit . 2doesc
  {{ sllit . 2doslit
#  { root . flstrexp1
  nl sllit . donl
  EOF Err
  ot sllit . 2doslit

sllit
  QQQ root 2.slit slit
1 \nl . . donl
  \ . . 2doesc
  {{ . . 2doslit
#  { root . flstrexp1
  nl . . donl
  EOF Err
  ot . . 2doslit

## TODO include later

builtin
  False
  None
  True
.
  id
  type
  NotImplemented
  Ellipsis
  numbers
  Number
  Integral
  Real
  Complex
  int
  bool
  float
  complex
.
  abs
  alter
  all
  any
  anext
  ascii
  bin
  breakpoint
  bytearray
  bytes
  callable
  chr
  classmethod
  compile
  delatytr
  dict
  dir
  divmod
  enumerate
  eval
  exec
  filter
  format
  frozenset
  getattr
  globals
  hasattr
  hash
  help
  hex
  input
  isinstance
  issubclass
  iter
.
  send
  throw
  close

dunder
  doc
  name
  file
  qualname
  module
  defaults
  code
  globals
  dict
  closure
  annotations
  kwdefaults
.
  self
  func
  new
  init
  del
  repr
  str
  bytes
  format
  lt
  le
  eq
  ne
  gt
  ge
  hash
  bool
.
  getattr
  getattribute
  setattr
  delattr
  dir
  class
.
  get
  set
  delete
  slots
  init_subclass
  set_name
  instancecheck
  subclasscheck
  class_getitem
  call
  len
  length_hint
  getitem
  setitem
  delitem
  missing
  iter
  reversed
  contains
.
  add
  sub
  mul
  matmul
  truediv
  floordiv
  mod
  divmod
  pow
  lshift
  rshift
  and
  xor
  or
.
  radd
  rsub
  rmul
  rmatmul
  rtruediv
  rfloordiv
  rmod
  rdivmod
  rpow
  rlshift
  rrshift
  rand
  rxor
  ror
.
  iadd
  isub
  imul
  imatmul
  itruediv
  ifloordiv
  imod
  idivmod
  ipow
  ilshift
  irshift
  iand
  ixor
  ior
.
  neg
  pos
  abs
  invert
  complex
  int
  float
  index
  round
  trunc
  floor
  ceil
  enter
  exit
  match_args
.
  await
  aiter
  anext
  aenter
  aexit
