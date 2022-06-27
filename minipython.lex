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

eof so

token
# group 0 - storable terms
  id
  nlit
  slit
  flit

# group 1 - operators and (aug)assign
  op 1
  aas 1

# group 2 - scope in
  co 2

# group 3 - scope out
  cc 3

  ro 4
  rc 4
  so 4
  sc 4

  das 4
  colon 4
  sepa 4
  comma 4
  dot 4
  ell 4

set
  aa a
  ii i
  oo o
  uu _
  af b-hj-np-zA-Z
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
  eq =
  pm +-
  o1 @%&^|*~
  o2 <>!
  EOF \00
 +an _a-zA-Z+nm
 +ee eE
 +oO oO
 +xx xX
 +hx abcdefABCDEF

keyword
#  and -> && op
  as
  assert
  def
  del
  elif
  else
  for
  from
  global
  if
  import
  in
  is
  nonlocal
  not
#  or -> || op
  pass
  return
  while
  with
  break|continue=ctlxfer

`Bbuiltin
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

`Ddunder
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

action

# ----------------------
# int literal. < 4G decoded in bits, else orig ascii
# ----------------------
ilit
1 ilitcnt++;
2 if (litbin) {
2   if (u4v < La_msk) atrs[dn] = u4v | (sign << La_bit);
2   else { atrs[dn] = La_ilit4 | (sign << La_bit); bits[bn++] = u4v; }
2 } else {
2   bits[bn++] = nlitpos; atrs[dn] = La_ilita;
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
2 if (litbin && fxp > fdp) fxp -= fdp; else litbin = 0;
2 if (litbin) {
2   atrs[dn] = La_flit8 | (sign << La_bit);
2   bits[bn++] = ((ub8)fxs << 63) | ((ub8)fxp << 54) | u4v;
2 } else {
2   bits[bn++] = nlitpos; atrs[dn] = La_flita;
2   memcpy(nlitpool+nlitpos,sp+N,n-N); nlitpool[nlitpos++] = 0;
2   nlitpos += n-N; litfpi=0; litbin = 1;
2 }
2 fxp=0;fxs=0;fdp=0;sign=0;
1 nlitpos += n-N;
#  info("flit %u",u4v);

doflitfr
2 if (litbin && u4v < D32max) u4v = u4v * 10 + c - '0'; else litbin = 0;
2 if (litbin && fdp < E16max) fdp++; else litbin = 0;

# ----------------------
# \xx in string literal
# ----------------------
doesc
  c = sp[n];
  if (c < 0x80) x1 = esctab[c];
  else x1 = 0;
  switch(x1) {
  case Esc_nl: lntab[l] = n; nlcol=n; break;
.
  case Esc_o: x1=doesco(sp,n); slitpool[slitx++] = x1; n += 3; break;
  case Esc_x: x1=doescx(sp,n); n += 2;
              if (x1 < 0x80 || (slitctl & 4) ) slitpool[slitx++] = x1;
              else { // utf8
                slitpool[slitx]   = 0xc0 | (x1 >> 6);
                slitpool[slitx+1] = 0x80 | (x1 & 0x3f);
                slitx += 2;
              }
              break;
.
  case Esc_u:
  case Esc_U: n=doescu(sp,n,slitpool,&slitx,x1); break;
  case Esc_N: n=doescn(sp,n,slitpool,&slitx); break;
.
  case Esc_inv: swarn(n,"unrecognised escape sequence '\\%s'",chprint(c));
                slitpool[slitx] = '\\';
                slitpool[slitx+1] = c;
                n += 2; slitx += 2;
                break;
.
  default:    slitpool[slitx++] = x1; n++;
  }
  switch (slitctl) {
  case 1: goto `rslit;
  case 2: goto `fslit;
  case 3: goto `frslit;
  }

# ----------------------
# start of string literal
# ----------------------
slit0
  if (tks[dn-1] == Tslit || tks[dn-1] == Tflit) { // merge dup
    dn--;
    slitx = slitpx;
    slitpos = slitppos;
  } else {
    slitx = slitpos;
  }

# ----------------------
# end of string literal
# ----------------------
.slit2
  R0=slitctl=0;
  len = slitx - slitpos;
  slitpx = slitx;
  switch(len) {
  case 0: atrs[dn] = 0; break;
  case 1: atrs[dn] = slitpool[slitpos]; break;
  case 2: x1 = slitpool[slitpos]; x11 = slitpool[slitpos+1];
          atr = x1 | (x11 << 8) | La_slit2;
          if (atr >= La_msk) {
            bits[bn++] = *(ub4 *)(slitpool + slitpos); atrs[dn] = La_slit4 | len;
          } else atrs[dn] = atr;
          break;
  case 3:
  case 4: bits[bn++] = *(ub4 *)(slitpool + slitpos); atrs[dn] = La_slit4 | len; break;
  default:
    slitppos = slitpos;
    id = slitgetadd(slitpool,slitx);
    if (id < La_msk) atrs[dn] = id | La_slits;
    else { bits[bn++] = id; atrs[dn] = La_slit | len; }
  }

slit1
  R0=slitctl=0;
  len = n - N;
  slitpos += len;
.
  switch(len) {
  case 1: slit1cnt++; break;
  case 2: slit2cnt++; break;
  case 3: slit3cnt++; break;
  default:
    if (len < Slitint) slitcnt++; else slitxcnt++;
  }

# ----------------------
# ident or kwd
# ----------------------

# 2 char private
id2prv
1 id2cnt++;
1 id2chr[sp[N]] = 1;
1 id2chr[sp[N+1]] = 2;
2 id = id2getadd(sp[N],sp[N+1]);
2 atrs[dn] = La_id2 | La_idprv | id;

# 2 char
id2
1 id2cnt++;
  info("id2 '%s' '%s'",chprint(prvc1),chprint(prvc2));
1 id2chr[prvc1] = 1;
1 id2chr[prvc2] = 2;
2 id = id2getadd(prvc1,prvc2);
2 atrs[dn] = La_id2 | id;

# 3+ char
id
  len = n - N;
1 if (len >= Idlen) lxerror(l,0,`L,len,"exceeding ID length limit"); // lx_error(l,0,`L,"id '%.*s' len %u exceeds %u",16,sp+N,len,Idlen);
  len2 = len;
  if (N & 3) {
    memcpy(albuf,sp+N,len2);
    idp = albuf;
  } else idp = sp+N;
  hc = hashalstr(idp,len2,Hshseed);
1 idcnt++;
1 idnplen += len2;
1 exp_first0(hc);
2 kw = lookupkw(idp,len2,hc);
2 if (kw < t99_count) {
2   if (kw < (enum token)T99_mrg) tk = kw; else { atrs[dn] = kw; tk = kwhshmap[kw]; tkgrps[0]++; }
2 } else {
2   tk = Tid;
2   blt = lookupblt(idp,len2,hc);
2   if (blt < B99_count) { atrs[dn] = blt | La_idblt; bltcnt++; }
2   else {
2     x4 = idgetadd(idp,len2,hc);
2     if (x4 < La_idprv) atrs[dn] = x4;
2     else { bits[bn++] = x4; atrs[dn] = La_id4; }
2   }
2 }
2 tks[dn] = tk; fpos[dn] = n;
2 dn++;

# __*__ = dunder or __* = class-private
`D dunder
  len = n - N;
  if (len > 2 && sp[n-1] == '_' && sp[n-2] == '_') { // dunder
    len2 = len - 2;
    if (len == 2) dun = lookupdun2(sp[N],sp[N+1]);
    else dun = lookupdun(sp+N,len2);
    if (dun == D99_count) lxwarn(l,0,`L,c,"unknown dunder");
    atrs[dn] = dun | La_iddun;
  } else { // class-private
1   if (len >= Idlen) lxerror(l,0,`L,len,"exceeding ID len limit");
    len2 = len;
    if (N & 3) {
      memcpy(albuf,sp+N,len2);
      idp = albuf;
    } else idp = sp+N;
    hc = hashalstr(idp,len2,Hshseed);
1   exp_first0(hc);
2   x4 = idgetadd(idp,len2,hc);
2   if (x4 < La_idprv) atrs[dn] = x4 | La_idprv;
2   else { bits[bn++] = x4; atrs[dn] = La_id4 | La_idprv; }
  }

`d dunder
  atrs[dn] = 0;

# ----------------------
# slit prefix
# ----------------------
dopfx1
  switch (sp[n-2] | 0x20) {
  case 'b': slitctl=4; break;
  case 'r': slitctl=1; break;
  case 'f': slitctl=2; break;
  case 'u': break;
1 default: lxerror(l,0,`L,c,"unknown slit prefix");
  }
.
2 if (tks[dn-1] == Tslit || tks[dn-1] == Tflit) { // merge dup
2   dn--;
2   slitx = slitpx;
2   slitpos = slitppos;
2 } else {
2   slitx = slitpos;
2 }
.
  switch (slitctl) {
  case 1: goto `rslit0;
  case 2: goto `fslit0;
  default: goto `slit0;
  }

dopfx2
  switch (sp[n-1] | 0x20) {
  case 'b': slitctl=4; break;
  case 'r': slitctl=1; break;
  case 'f': slitctl=2; break;
  case 'u': break;
1 default: lxerror(l,0,`L,c,"unknown slit prefix");
  }
  switch (sp[n-2] | 0x20) {
  case 'b': slitctl|=4; break;
  case 'r': slitctl|=1; break;
  case 'f': slitctl|=2; break;
  case 'u': break;
1 default: lxerror(l,0,`L,c,"unknown slit prefix");
  }
.
2 if (tks[dn-1] == Tslit || tks[dn-1] == Tflit) { // merge dup
2   dn--;
2   slitx = slitpx;
2   slitpos = slitppos;
2 } else {
2   slitx = slitpos;
2 }
.
  switch (slitctl) {
  case 1: goto `rslit0;
  case 2: goto `fslit0;
  case 3: goto `frslit0;
  }

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
2   tks[dn] = Tco; fpos[dn] = n;
    dn++;
  } else {
2   tks[dn] = Tsepa; fpos[dn] = n;
    dn++;
    while (dent < dentst[dentlvl]) {
2     tks[dn] = Tcc; fpos[dn] = n;
      dn++;
      if (dentlvl) dentlvl--;
      else break;
    }
  }
  }

# ----------------------
# opening { [ (
# ----------------------
dobo
1 if (bolvl + 1 >= Depth) lxerror(l,0,`L,bolvl,"exceeding nesting depth");
  bolvls[bolvl] = n;
  bolvlc[bolvl] = c;
  bolvl++;

# ----------------------
# closing ]
# ----------------------
dosc
1 if (bolvl == 0) lxerror(l,0,`L,c,"unbalanced");
  bolvl--;
1 if (bolvlc[bolvl] != '[') lxerror(l,0,`L,c,"unmatched");

# ----------------------
# closing )
# ----------------------
dorc
1 if (bolvl == 0) lxerror(l,0,`L,c,"unbalanced");
  bolvl--;
1 if (bolvlc[bolvl] != '(') lxerror(l,0,`L,c,"unmatched");

# ----------------------
# closing }
# ----------------------
docc
  if (bolvl == fsxplvl) { slitctl = orgslitctl; R0 = orgR0; goto `fslit; }
1 else if (bolvl == 0) lxerror(l,0,`L,c,"unbalanced");
  bolvl--;
1 if (bolvlc[bolvl] != '{') lxerror(l,0,`L,c,"unmatched");

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

# plus min
  pm pm . 2. sign = c;

# int literals

  # single digit
  nm. flitfr . 2`u4v = c - '0'; fdp=0;` .N=n;

  nmee flitxp0 . 2`u4v = c - '0';` .N=n;

  0 zilit0 . 2`u4v = c - '0';` .N=n;

  nm filit . 2`u4v = c - '0';` .N=n;

# id or kwd
  a a1
  i i1
  o o1

  _ u1

# id / kwd /slit pfx
  af id1 . .N=n;

# string literal - no prefix
  qq slit0 . 1`N=n;` .Q=c;\
             2slit0

# float literal
  .nm flitfr . 2`u4v=0; fdp=0;` .N=n;

# brackets
  { . co dobo
  } . cc docc

  [ . so dobo
  ] . sc dosc

  ( . ro dobo
  ) . rc dorc

  =  . das 2.atrs[dn] = c;
  := . das 2.atrs[dn] = c;

# punctuators
  : . colon
  ; . sepa

  , . comma
  . dot

# operators @ % & ^ | * ~
  o1 op11 . 2. atrs[dn] = c; Q=c;

# < > !
  o2 op21 . 2. atrs[dn] = c; Q=c;

# escaped newline
  \`nl . . donl

  EOF EOF

dot
 .. root ell
 ot -root dot

# ---------------------
# operators / augassign
# ---------------------
op11
  Q op12 . 2. atrs[dn] |= Lxop2;
  = root aas
  ot -root op

op12
  = root aas
  ot -root op

op21
  = root op 2. atrs[dn] |= Lxoe;
  Q op22 .  2. atrs[dn] |= Lxop2;
  ot -root op

op22
  = root aas
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
# and as in is if or
# ---------------------
a1
  nd root op 2. atrs[dn] = '&' | 0x80;
  s  root as
  an id2 . .N=n-1;
  ot -root 2.id 1`id1cnt++;` 2`atrs[dn] = id1getadd('a') | La_id1;`

i1
  f root if
  s root is
  n root in
  an id2 . .N=n-1;
  ot -root 2.id 1`id1cnt++;` 2`atrs[dn] = id1getadd('i') | La_id1;`

o1
  r root op 2. atrs[dn] = '|' | 0x80;
  an id2 . .N=n-1;
  ot -root 2.id 1`id1cnt++;` 2`atrs[dn] = id1getadd('o') | La_id1;`

u1
  _ dun0 . .N=n;
  an id2
  ot -root 2.id 1`id1cnt++;` 2`atrs[dn] = La_id_;`

dun0
  an dun1
  ot -root 2.id `prvc1 = prvc2 = '_';` id2

dun1
  an dun2
  ot -root 2.id 1`id1cnt++;` 2`atrs[dn] = id1getadd(sp[N]) | La_id1 | La_idprv;`

dun2
  an dun
  ot -root  2.id id2prv

dun
  an
  ot -root 2.id dunder

# ---------------------
# identifier or keyword. lookup in action
# ---------------------
id1
  an id2s
  qq . . 1`N=n;` `Q=c;` dopfx1
  ot -root 2.id 1`id1cnt++;` 2`atrs[dn] = id1getadd(sp[N]) | La_id1;`

id2s
  an id
  qq . . 1`N=n;` `Q=c;` dopfx2
  ot -root . `prvc1 = sp[N]; prvc2 = c;` id2

id2
  an id
  ot -root . `prvc1 = sp[N]; prvc2 = c;` id2

id
  an
#  nl root . donl\
#            id
  ot -root . id

# --------------
# sign or operator. Process here for nlits
# --------------
pm
  pm . . 2. sign ^= 1; ## -- -> +  +- -> -
  0 zilit0 . N=n;\
             2. u4v = 0;
  nm -root
  = root aas  2. atrs[dn] = sign ? '-' : '+';
  ot -root op 2. atrs[dn] = sign ? '-' : '+';

# ---------------------
# float literal tramp
# ---------------------
flit
  ot -root 2.nlit flit

# ---------------------
# int   literal tramp
# ---------------------
ilit
  ot -root 2.nlit ilit

# ---------------------
# prefixed integer literal
# ---------------------
.zilit0
  bb ilitb
  oO ilito
  xx ilitx
  _
  0
  nm filit . 2. u4v = c - '0'; ## decimal with leading zero
  . flitfr . 2. fdp=0; ## 0.
  j flit   . 2. litfpi = 1;
  ot -root 2.nlit 2`atrs[dn] = 0;` 1. ilit1cnt++;

# ---------------------
# float or int literal
# ---------------------
.filit
  nm . . 2. if (litbin && u4v < D32max) u4v = u4v * 10 + c - '0'; else litbin = 0;
  _
  . flitfr . 2. fdp=0;
  ee flitxp0 .   2. fxs = 0; fxp = 0;
  j flit . 2. litfpi = 1;
  ot -ilit

# ---------------------
# float literal fraction
# ---------------------
.flitfr
  nm . . doflitfr
  _
  ee flitxp0 . 2. fxs = 0; fxp = 0;
  j  flit    . 2. litfpi = 1;
  ot -flit

# ---------------------
# float literal exp
# ---------------------
.flitxp0
  nm flitxp . 2. if (litbin && fxp < E16max) fxp = fxp * 10 + c - '0'; else litbin = 0;
  pm flitxp . 2. fxs = (c == '-');
  _
  j  flit    . 2. litfpi = 1;

.flitxp
  nm . . 2. if (litbin && fxp < E16max) fxp = fxp * 10 + c - '0'; else litbin = 0;
  _
  j  flit  . 2. litfpi = 1;
  ot -flit

# ---------------------
# int literal binary
# ---------------------
.ilitb
  0 . . 2. if (litbin && u4v < B32max) u4v <<= 1; else litbin = 0;
  1 . . 2. if (litbin && u4v < B32max) u4v = u4v << 1 | 1; else litbin = 0;
  _
  ot -ilit

# ---------------------
# int literal octal
# ---------------------
.ilito
  8 . . .lxerror(l,n-nlcol,$L,c,"invalid digit in octal nlit");
  9 . . .lxerror(l,n-nlcol,$L,c,"invalid digit in octal nlit");
  nm . . 2. if (litbin && u4v < D32max) u4v = u4v << 3 | (c - '0'); else litbin = 0;
  _
  ot -ilit

# ---------------------
# int literal hex
# ---------------------
.ilitx
  nm . . 2. if (litbin && u4v < X32max) u4v = u4v << 4 | (c - '0'); else litbin = 0;
  hx . . 2. if (litbin && u4v < X32max) u4v = u4v << 4 | ( (c | 0x20) + 10 - 'a'); else litbin = 0;
  _
  ot -ilit

# ---------------------
# string literal start
# ---------------------
slit0
  QQ slit . .R0=1; # start of long slit
  Q root 2.slit 1`slit0cnt++;` 2.atrs[dn] = 0; # empty short slit ''
  \nl . . donl
  \ -slit
1 R00nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  ot slit . 2.slitpool[slitx++] = c;

# ---------------------
# string literal
# ---------------------
slit string literal
  Q slitq
  \nl . . donl
1 \Q
2 \ slitesc
1 R00nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
1 EOF  . . .lxerror(l,n-nlcol,$L,c,"unterminated slit");
  ot . . 2.slitpool[slitx++] = c;

# possible end of slit
slitq
  R01QQ slit9 # end of long slit
  R01   slit  . 2. slitpool[slitx++] = Q; slitpool[slitx++] = c; # less than 3q in long slit
  ot   -slit9 # end of short slit

# end of slit
slit9
  ot -root 2.slit 1slit1\
                  2slit2

# ---------------------
# \ esc in slit or flit
# ---------------------
slitesc*
  \ slit . 2doesc

# ---------------------
# raw string literal start
# ---------------------
rslit0*
  QQ rslit . .R0=1; # start of long slit
  Q root 2.slit 1`slit0cnt++;` 2.atrs[dn] = 0; # empty short slit ''
  \ -rslit
1 R00nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  ot rslit . 2.slitpool[slitx++] = c;

# ---------------------
# raw string literal
# ---------------------
rslit string literal
  Q rslitq
  \nl . . 2`slitpool[slitx++] = '\\'; slitpool[slitx++] = '\n';` donl
  \Q . . 2.slitpool[slitx++] = '\\'; slitpool[slitx++] = Q;
1 R00nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
1 EOF  . . .lxerror(l,n-nlcol,$L,c,"unterminated slit");
  ot . . 2.slitpool[slitx++] = c;

# possible end of raw slit
rslitq
  R01QQ slit9 # end of long slit
  R01   rslit  . 2. slitpool[slitx++] = Q; slitpool[slitx++] = c; # less than 3q in long slit
  ot   -slit9 # end of short slit

# ---------------------
# fstring literal start
# ---------------------
fslit0*
  QQ fslit . .R0=1; # start of long slit
  Q root 2.slit 1`slit0cnt++;` 2.atrs[dn] = 0; info("empty slit %u",dn);  # empty short slit ''
  { -fslit
  \ -fslit
1 R00nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  ot fslit . 2.slitpool[slitx++] = c;

# ---------------------
# fstring literal
# ---------------------
fslit string literal
  Q fslitq
  {{ . . 2.slitpool[slitx++] = c;
  }} . . 2.slitpool[slitx++] = c;
  { root 2.flit .fsxplvl = bolvl; orgslitctl = slitctl; orgR0 = R0;\
              1slit1\
              2slit2
  \nl . . donl
1 \Q
2 \ slitesc
1 R00nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
1 EOF  . . .lxerror(l,n-nlcol,$L,c,"unterminated slit");
  ot . . 2.slitpool[slitx++] = c;

# possible end of fslit
fslitq
  R01QQ fslit9 # end of long slit
  R01   fslit  . 2. slitpool[slitx++] = Q; slitpool[slitx++] = c; # less than 3q in long slit
  ot   -fslit9 # end of short slit

# ---------------------
# raw fstring literal start
# ---------------------
frslit0*
  QQ frslit . .R0=1; # start of long slit
  Q root 2.slit 1`slit0cnt++;` 2.atrs[dn] = 0; # empty short slit ''
  \ -frslit
  { -frslit
1 R00nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  ot frslit . 2.slitpool[slitx++] = c;

# ---------------------
# raw fstring literal
# ---------------------
frslit string literal
  Q frslitq
  {{ . . 2.slitpool[slitx++] = c;
  }} . . 2.slitpool[slitx++] = c;
  { root 2.flit .fsxplvl = bolvl; orgslitctl = slitctl; orgR0 = R0;\
              1slit1\
              2slit2
  \nl . . 2`slitpool[slitx++] = '\\'; slitpool[slitx++] = '\n';` donl
  \Q . . 2.slitpool[slitx++] = '\\'; slitpool[slitx++] = Q;
1 R00nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
1 EOF  . . .lxerror(l,n-nlcol,$L,c,"unterminated slit");
  ot . . 2.slitpool[slitx++] = c;

# possible end of fslit
frslitq
  R01QQ fslit9 # end of long slit
  R01   frslit  . 2. slitpool[slitx++] = Q; slitpool[slitx++] = c; # less than 3q in long slit
  ot   -fslit9 # end of short slit

# end of fslit
fslit9
  ot -root 2.slit 1slit1\
                  2slit2

