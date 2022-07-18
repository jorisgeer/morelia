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
  co 2 {

# group 3 - scope out
  cc 3 }

  ro 4 (
  rc 4 )
  so 4 [
  sc 4 ]

  ind 4
  ded 4

  das 4 =
  colon 4 :
  sepa 4 ;
  comma 4 ,
  dot 4 .
  ell 4 ...

set
  uu _
  af a-zA-Z\Z
  nm 0123456789
  ht \09
  vt \0c
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
  o1 @%&^|*/~
  o2 <>!
  EOF \00
 +an _a-zA-Z\Z\z+nm
 +ee eE
 +oO oO
 +xx xX
 +hx abcdefABCDEF
 -ws \20
 -tk `

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
.
  context
  args

action

# ----------------------
# int literal. < 4G decoded in bits, else orig ascii
# ----------------------
ilit
1 ilitcnt++;
2 if (litbin) {
2   atrs[dn] = sign << La_bit;
2   if (u4v < La_msk) atrs[dn] |= u4v;
2   else { atrs[dn] |= La_ilit4; bits[bn++] = u4v | ((ub8)dn << 32); }
2 } else {
2   bits[bn++] = N | ((ub8)dn << 32); atrs[dn] = La_ilita | (n-N); ilitacnt++;
2   litbin = 1;
2 }
2 tks[dn] = Tnlit; sign=0;
2 fpos[dn] = N | (ilitctl << Lxabit); ilitctl=0;
2 dn++;

# ----------------------
# float literal. < 4G decoded in bits, else orig ascii
# ----------------------
flit
1 flitcnt++;
2 if (litbin && fxp > fdp) fxp -= fdp; else litbin = 0;
2 if (litbin) {
2   atrs[dn] = La_flit8 | sign << La_bit;
2   // vrbo("ln %u bn %u flit val %u.%u.%u",`L,bn,fxs,fxp,u4v);
2   bits[bn++] = ((ub8)fxs << 63) | ((ub8)fxp << 54) | u4v;
2 } else {
2   bits[bn++] = N; atrs[dn] = La_flita | (n-N); flitacnt++;
2   litfpi=0; litbin = 1;
2 }
2 fxp=0;fxs=0;fdp=0;sign=0;

doflitfr
2 if (litbin && u4v < D32max) u4v = u4v * 10 + c - '0'; else litbin = 0;
2 if (litbin && fdp < E16max) fdp++; else litbin = 0;

# ----------------------
# \xx in string literal
# ----------------------
doesc
  c = sp[n++];
  if (c < 0x80) x1 = esctab[c];
  else x1 = 0;
  switch(x1) {
  case Esc_nl: lntab[l++] = n; nlcol=n; break;
.
  case Esc_o:
    x1 = c - '0';
    c = sp[n];
    if (c >= '0' && c <= '7') { n++; x1 = (x1 << 3) + (c - '0'); } else { slitpool[slitx++] = x1; break; }
    c = sp[n];
    if (c >= '0' && c <= '7') { n++; x1 = (x1 << 3) + (c - '0'); }
    slitpool[slitx++] = x1;
    break;
.
  case Esc_x: n=doescx(sp,n,slitpool,&slitx,slitctl & 4);
              break;
.
  case Esc_u:
  case Esc_U: n=doescu(sp,n,slitpool,&slitx,x1); break;
  case Esc_N: n=doescn(sp,n,slitpool,&slitx);    break;
.
  case Esc_inv:
    // sinfo(n,"'\\%s'",chprint(c));
    // lxwarn(l,0,`L,c,"unrecognised escape sequence");
    slitpool[slitx] = '\\';
    slitpool[slitx+1] = c;
    slitx += 2;
    break;
.
  default:    slitpool[slitx++] = x1;
  }
  if (lang_fstring && (slitctl & 2)) goto `fslit; else goto `slit;

# ----------------------
# start of string literal
# ----------------------
slit0
  if (lang_concat && tks[dn-1] == Tslit) { // concat
    dn--;
    slitx = slitpx;
    bn = orgbn;
    slitpos = slitppos;
    slitcatcnt++;
  } else {
    slitx = slitpos;
  }
  // info("ln %u slit start %u ctl %u",l,slitx,slitctl);

# ----------------------
# end of string literal
# ----------------------
.slit2
  if (Q != '\'' && Q != '"') lxerror(l,0,`L,Q,"invalid quote");
  len = slitx - slitpos;
  slitpx = slitx; orgbn = bn; slitppos = slitpos;
  switch(len) {
  // case 0: atrs[dn] = La_slit0; break; // empty str
  case 1: atrs[dn] = slitpool[slitpos]; break;
  case 2:
  case 3: memcpy(bits+bn,slitpool + slitpos,len);
          // info("ln %u bn %u slit23 '%.*s'",`L,bn,len,slitpool + slitpos);
          bits[bn++] |= ((ub8)__LINE__ << 32); atrs[dn] = (len == 2 ? La_slit2 : La_slit3);
          break;
  default:
    if (slitpool[slitpos] == '.' && slitpool[slitpos+1] == '/') {
      vrbo("%.*s",len,slitpool + slitpos);
      memcpy(cursrc,slitpool+slitpos,min(len,250)); cursrc[min(len,250)] = 0;
    }
    // sinfo(n,"ln %u slit add ctl %x R0 %u len %u '%s'",l,slitctl,R0,len,chprintn(slitpool + slitpos,min(len,64)));
    id = slitgetadd(slitpool,slitx,slitctl);
    if (id < La_msk) atrs[dn] = id | La_slits;
    else {
      // vrbo("ln %u bn %u slit id %u",`L,bn,id);
      bits[bn++] = id | ((ub8)dn << 32); atrs[dn] = La_slit;
    }
  }
  tks[dn] = tk;
  fpos[dn] = N | (R2 << 31) | (R0 << 30) | ((Q == '"') << 29) | (slitctl << Lxabit);
  dn++;
  R0=slitctl=0;

# idem, empty
slit20
  if (Q != '\'' && Q != '"') lxerror(l,0,`L,Q,"invalid quote");
  orgbn = bn;
  tks[dn] = Tslit;
  atrs[dn] = La_slit0;
  fpos[dn] = N | (R2 << 31) | (R0 << 30) | ((Q == '"') << 29) | (slitctl << Lxabit);
  dn++;
  R0=slitctl=0;

slit1
  len = n - N;
  if (sp[N+1] == '.' && sp[N+2] == '/') { memcpy(cursrc,sp+N,len); cursrc[len] = 0; }
  // vrb(" add slit pos %u len %2u typ %u.%u ' %.*s '",dn1,len,slitctl,R0,len,chprintn(sp+N,min(len,512)));
  R0=slitctl=0;
  slitpos += len;
.
  switch(len) {
  case 0: slit0cnt++; break;
  case 1: slit1cnt++; break;
  case 2: slit2cnt++; break;
  case 3: slit3cnt++; break;
  default:
    if (len < Slitint) slitncnt++; else slitxcnt++;
  }

# ----------------------
# ident or kwd
# ----------------------

# 2 char private
id2prv
1 id2cnt++;
1 // info("add id.2  %s%s",chprint(prvc1),chprint(prvc2));
1 id2chr[sp[N]] |= 1;
1 id2chr[sp[N+1]] |= 2;
2 id = id2getadd(n,sp[N],sp[N+1]); // id2cnt++;
2 atrs[dn] = La_id2 | La_idprv | id;

# 2 char
id2
1 // info("add id.2  %s%s",chprint(prvc1),chprint(prvc2));
  if (prvc1 == 'o' && prvc2 == 'r') {
    tk = Top;
1   dn++;
2   atr = '|' | Lxop2;
  }
2 else if ( (kw = lookupkw2(prvc1,prvc2)) < t99_count) { tk = kw; atr = 0; }
  else {
1   id2cnt++;
1   id2chr[prvc1] |= 1;
1   id2chr[prvc2] |= 2;
2   id = id2getadd(n,prvc1,prvc2);
    tk = Tid;
2   atr = La_id2 | id;
  }
2 tks[dn] = tk; fpos[dn] = N; atrs[dn] = atr;
2 dn++;

id2u
1 id2cnt++;
1 // info("add id.2  %s%s",chprint(prvc1),chprint(prvc2));
1 id2chr[prvc1] |= 1;
1 id2chr[prvc2] |= 2;
2 id = id2getadd(n,prvc1,prvc2); // id2cnt++;
2 atrs[dn] = La_id2 | id;

# 3+ char
id
  len = n - N;
  // lx_error(l,0,`L,"id '%.*s' len %u exceeds %u",16,sp+N,len,Idlen);
  if (len >= Idlen) {
1   vrbo("len %u max %u",len,Idlen);
1   // lxwarn(l,0,`L,len,"exceeding ID length limit");
    len = Idlen-1;
  }
1 // info("ln %u add id.%-2u %.*s",l,len,len,chprintn(sp+N,len));
2 // sinfo(N,"ln %u add id.%-2u %.*s",l,len,len,chprintn(sp+N,len));
  len2 = len;
  if (len == 3 && sp[N] == 'a' && sp[N] == 'n' && sp[N] == 'd') {
    tk = Top;
1   dn++;
2   atr = '&' | Lxop2;
  } else {
  if (N & 3) {
    memcpy(albuf,sp+N,len2);
    idp = albuf;
  } else idp = sp+N;
  hc = hashalstr(idp,len2,Hshseed);
1 idcnt++;
1 idnplen += len2;
1 tk = Tid;
1 exp_first0(hc,idsketchs);
2 kw = lookupkw(idp,len2,hc);
2 if (kw < t99_count) {
2   atr = kw; if (kw < (enum token)T99_mrg) tk = kw; else { tk = kwhshmap[kw]; tkgrps[0]++; }
2 } else {
2   tk = Tid;
2   blt = lookupblt(idp,len2,hc);
2   if (blt < B99_count) { atr = blt | La_idblt; bltcnt++; }
2   else {
2     x4 = idgetadd(idp,len2,hc);
2     if (x4 < La_idprv) atr = x4;
2     else {
2       // if (/* bn > 22380 && */bn < 22399) vrbo("ln %u bn %u id id %u %.*s",l,bn,x4,len2,idp);
2       bits[bn++] = x4 | ((ub8)dn << 32); atr = La_id4;
2     }
2   }
2 }
  } // id
2 if (dn >= tkcnt) ice(n,"ln %u tk %u above cnt %u",l,dn,tkcnt);
2 tks[dn] = tk; atrs[dn] = atr; fpos[dn] = N;
2 dn++;

# __*__ = dunder or __* = class-private
`D dunder
1 idcnt++;
  len = n - N;
1 // info("add id.%-2u __%.*s",len,len,chprintn(sp+N,len));
  if (len > 2 && sp[n-1] == '_' && sp[n-2] == '_') { // dunder
    len2 = len - 2;
2   if (len2 == 2) dun = lookupdun2(sp[N],sp[N+1]);
2   else dun = lookupdun(sp+N,len2);
2   // if (dun == D99_count) svrb(n,"unknown dunder '%.*s'.%u",len2,sp+N,len2); // lxwarn(l,0,`L,c,"unknown dunder");
2   atrs[dn] = dun | La_iddun;
  } else { // class-private
1   if (len >= Idlen) lxerror(l,0,`L,len,"exceeding ID len limit");
    len2 = len;
    if (N & 3) {
      memcpy(albuf,sp+N,len2);
      idp = albuf;
    } else idp = sp+N;
    hc = hashalstr(idp,len2,Hshseed);
1   exp_first0(hc,idsketchs);
2   x4 = idgetadd(idp,len2,hc);
2   if (x4 < La_idprv) atrs[dn] = x4 | La_idprv;
2   else {
2     // vrbo("ln %u bn %u did id %u",`L,bn,x4);
2     bits[bn++] = x4 | ((ub8)dn << 32); atrs[dn] = La_id4 | La_idprv;
2   }
  }

`d dunder
  atrs[dn] = 0;

# ----------------------
# slit prefix
# ----------------------
dopfx1
.
2 if (lang_concat && tks[dn-1] == Tslit) { // concat
2   dn--;
2   slitx = slitpx;
2   bn = orgbn;
2   slitpos = slitppos;
2   slitcatcnt++;
2 } else {
2   slitx = slitpos;
2 }
.
  switch (sp[n-2] | 0x20) {
  case 'b': slitctl = 4; goto `slit0;
  case 'r': slitctl = 1; goto `rslit0;
  case 'f':
    if (fsxplvl == hi16) { slitctl = 2; if (lang_fstring) goto `fslit0; else goto `slit0; }
1   else lxwarn (l,0,`L,'f',"nested f-strings not supported");
  case 'u': goto `slit0;
  default:
1   info("%.16s",sp + n - 8); lxwarn(l,0,`L,sp[n-2],"unknown slit prefix");
    goto `slit0;
  }

dopfx2
1 // info("pfx2 '%c'",sp[n-2]);
  switch (sp[n-2] | 0x20) {
  case 'b': slitctl = 4; break;
  case 'r': slitctl = 1; break;
  case 'f':
    if (fsxplvl == hi16) slitctl = 2;
1   else lxwarn (l,0,`L,'f',"nested f-strings not supported");
2   else lxerror(l,0,`L,'f',"nested f-strings not supported");
    break;
  case 'u': break;
  default: slitctl = 0;
1  info("%.16s",sp + n - 8); lxwarn(l,0,`L,sp[n-2],"unknown slit prefix");
  }
1 // info("pfx2 '%c'",sp[n-3]);
  switch (sp[n-3] | 0x20) {
  case 'b': slitctl |= 4; break;
  case 'r': slitctl |= 1; break;
  case 'f':
    if (fsxplvl == hi16) slitctl |= 2;
1   else lxwarn(l,0,`L,'f',"nested f-strings not supported");
2   else lxerror(l,0,`L,'f',"nested f-strings not supported");
    break;
  case 'u': break;
1 default: info("%.16s",sp + n - 8); lxwarn(l,0,`L,sp[n-3],"unknown slit prefix");
  }
.
2 if (lang_concat && tks[dn-1] == Tslit) { // concat
2   dn--;
2   slitx = slitpx;
2   bn = orgbn;
2   slitpos = slitppos;
2   slitcatcnt++;
2 } else {
2   slitx = slitpos;
2 }
.
  if (lang_fstring) x1 = slitctl; else x1 = slitctl & ~2;
  switch (x1) {
  case 0: case 4: goto `slit0;
  case 1: case 5: goto `rslit0;
  case 2: case 6: goto `fslit0;
  case 3: case 7: goto `frslit0;
  default: lxerror(l,0,`L,slitctl,"invalid slit ctl");
  }

# ----------------------
# newline: maintain line table
# ----------------------
donl
2 lntab[l] = n;
2 // sinfo(n,"ln %u n %u",l,n);
  l++;
  nlcol = n;

# ----------------------
# indent / dedent - create { }
# ----------------------
.dodent
  hasdent=0;
  if (bolvl == 0) {
  dent = n - nlcol;
  if (dent > dentst[dentlvl]) {
    if (dentlvl < Dent-1) {
      dentlvl++;
# 2     vrbo("tk %u dentlvl %u",dn,dentlvl);
    }
    dentst[dentlvl] = dent;
2   tks[dn] = Tind; fpos[dn] = n;
    dn++;
  } else {
2   tks[dn] = Tsepa; fpos[dn] = n;
    dn++;
    while (dent < dentst[dentlvl]) {
2     tks[dn] = Tded; fpos[dn] = n;
      dn++;
      if (dentlvl) {
        dentlvl--;
# 2       vrbo("tk %u dentlvl %u",dn,dentlvl);
      } else break;
    }
  }
  }

# ----------------------
# opening { [ (
# ----------------------
doso
  // sinfo(l,"ln %u open %c lvl %u R3 %u",l,c,bolvl,R3);
1 if (bolvl + 1 >= Depth) lxerror(l,0,`L,bolvl,"exceeding nesting depth");
1 solvl++; socnt++;
  bolvls[bolvl] = n;
  bolvlc[bolvl] = c;
  bolvl++; R3++;
2 atrs[dn] = bolvl;

doro
  // sinfo(l,"ln %u open %c lvl %u R3 %u",l,c,bolvl,R3);
  if (bolvl + 1 >= Depth) lxerror(l,0,`L,bolvl,"exceeding nesting depth");
1 rolvl++; rocnt++;
  bolvls[bolvl] = n;
  bolvlc[bolvl] = c;
  bolvl++; R3++;
2 atrs[dn] = bolvl;

doco
  // sinfo(l,"ln %u open %c lvl %u R3 %u",l,c,bolvl,R3);
1 if (bolvl + 1 >= Depth) lxerror(l,0,`L,bolvl,"exceeding nesting depth");
1 colvl++; cocnt++;
  bolvls[bolvl] = n;
  bolvlc[bolvl] = c;
  bolvl++; R3++;
2 atrs[dn] = bolvl;

# ----------------------
# closing ]
# ----------------------
dosc
1 if (bolvl == 0) lxerror(l,0,`L,c,"unbalanced");
1 solvl--; sccnt++;
2 atrs[dn] = bolvl;
  bolvl--; R3--;
1 if (bolvlc[bolvl] != '[') lxerror(l,0,`L,c,"unmatched");

# ----------------------
# closing )
# ----------------------
dorc
1 if (bolvl == 0) lxerror(l,0,`L,c,"unbalanced");
1 rolvl--; rccnt++;
2 atrs[dn] = bolvl;
  bolvl--; if (R3) R3--;
1 if (bolvlc[bolvl] != '(') lxerror(l,0,`L,c,"unmatched");

# ----------------------
# closing }
# ----------------------
docc
  // vrb("bolvl %u fsxplvl %u",bolvl,fsxplvl);
2 tks[dn] = Tcc; atrs[dn] = bolvl; fpos[dn] = n;
  dn++;
  if (bolvl == fsxplvl) { // end of f-string expr without fmtspec
    slitctl = orgslitctl; R0 = orgR0; Q = orgQ; fsxplvl = hi16; R2=0;
    // info("slitctl %u",slitctl);
1   N=n; // dn++;
2   slitx = slitpos;
    if (slitctl & 1) goto `frslit; else goto `fslit;
  }
1 else if (bolvl == 0) lxerror(l,0,`L,c,"unbalanced");
  bolvl--; R3--;
1 if (bolvlc[bolvl] != '{') lxerror(l,0,`L,c,"unmatched");
1 if (colvl == 0) { info("cocnt %u",cocnt); lxerror(l,0,`L,c,"unbalanced"); }
1 colvl--; cccnt++;

dofcc
  sinfo(l,"fcc bolvl %u fsxplvl %u R3 %u",bolvl,fsxplvl,R3);
  if (R3 == 0) {  // end of f-string expr with fmtspec
    slitctl = orgslitctl; R0 = orgR0; Q = orgQ; fsxplvl = hi16; R2=0;
    info("end of flit slitctl %u",slitctl);
2   orgbn = bn;
2   len = n - N;
2   memcpy(slitpool + slitpos,sp+N,len);
2   id = slitgetadd(slitpool,slitpos+len,slitctl);
2   if (id < La_msk) atrs[dn] = id | La_slits;
2   else {
2     info("ln %u bn %u fslit id %u",`L,bn,id);
2     bits[bn++] = id | ((ub8)__LINE__ << 32); atrs[dn] = La_slit | len;
2   }
2   tks[dn] = Tslit; atrs[dn] = bolvl; fpos[dn] = n;
    dn++;
1   N=n;
    slitx = slitpos;
    if (slitctl & 1) goto `frslit; else goto `fslit;
  } else {
1   if (bolvl == 0) lxerror(l,0,`L,c,"unbalanced");
    bolvl--; R3--;
#   if (bolvlc[bolvl] != '{') lxerror(l,0,`L,c,"unmatched");
2   tks[dn] = Tcc; fpos[dn] = n;
1   if (colvl == 0) { info("cocnt %u",cocnt); lxerror(l,0,`L,c,"unbalanced"); }
1   colvl--; cccnt++;
    dn++;
  }

INIT
  bolvl = 0;

# ---------------------
table
# ---------------------

# state lead pat

# ---------------------
# initial state
# ---------------------
root.N

# pat nxstate token action

# whitespace and newlines
  `ws
  cr
  vt
  nl sol . donl

  tk # py2 repr

# line comments
  hs cmt0

# plus min
  pm pm . 2. signc = c; sign = (c == '-');

# int literals

  # single digit
  nm. flitfr . 2`u4v = c - '0'; fdp=0;`

  nmee flitxp0 . 2`u4v = c - '0';`

  0 zilit0

  nm filit . 2`u4v = c - '0';`

# id or kwd
  _ u1

# id / kwd /slit pfx
  af id1 . .prvc1 = c;

# string literal - no prefix
  qq slit0 . .Q=c;\
             2slit0

# float literal
  .nm flitfr . 2`u4v=0; fdp=0;`

# brackets
  { . co doco
  } . 3.cc docc

  [ . so doso
  ] . sc dosc

  ( . ro doro
  ) . rc dorc

  =  eq1

#  R21R30! fstrcnv . 1.sinfo(l,"goto fstrcnv on '%c'",c);
  :hs # fstr format f'{1:#{1}}'

# punctuators
  : col1
  ; . sepa 2.atrs[dn] = c;

  , . comma
  . dot

# operators @ % & ^ | * / ~
  o1 op11 . 2`atrs[dn] = c;` Q=c;

# < > !
  o2 op21 . 2`atrs[dn] = c;` Q=c;

# escaped newline
  \ foldnl

  EOF EOF
#  $$ EOF

# ---------------------
foldnl
  nl root . donl

# ---------------------
eq1
  = root op 2.atrs[dn] = '=' | Lxop2;
#  R21R30 fstrcnv . 1.sinfo(l,"goto fstrcnv on R21 && '%c'",c);
  ot -root das 2.atrs[dn] = '=';

# ---------------------
col1
#  R21R30 fstrcnv . 1`sinfo(l,"goto fstrcnv on R21 R30 '%c'",c);` .R3=0;
  = root das 2.atrs[dn] = ':';
  ot -root colon # 1.sinfo(l,"colon plus '%c' R2 %u R3 %u",c,R2,R3);

# ---------------------
fstrcnv**
  { . . doco
  } . . dofcc
  ( . . doro
  ) . . dorc
  [ . . doso
  ] . . dosc
  nl . . donl
  EOF . . .lxerror(l,n-nlcol,$L,c,"unterminated slit");
  ot . . .sinfo(l,"c %c pos %u R3 %u",c,n,R3);

# ---------------------
dot
 .. root ell
 ot -root dot

# ---------------------
# operators / augassign
# ---------------------
op11
  Q op12 . 2. atrs[dn] |= Lxop2;
  = root aas
  ot -root op # .info("op1 %c %c",Q,c);

op12
  = root aas
  ot -root op

op21
  = root op 2. atrs[dn] |= Lxoe;
  Q op22 .  2. atrs[dn] |= Lxop2;
#  afR21 -fstrcnv . 1`sinfo(l,"goto fstrcnv on '%c'",c);` .R3=0;
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
  nl . 3.ded donl
  hs cmt . .cmt0 = n-1;
  ot -root 3.ind # dodent

# ---------------------
# line comment
# ---------------------
cmt0
  R21 -root
  ot -cmt . .cmt0 = N;

cmt
  nl sol . 1`ncmt += 2;` 2`cmts[ncmt++] = cmt0; cmts[ncmt++] = n;` donl
  EOF EOF
  ot

# ---------------------
u1
  _ dun0 . .N=n;
  an id2
  ot -root 2.id 1`id1cnt++;` 2`atrs[dn] = La_id_; id1cnt++;`

dun0
  an dun1
  ot -root 2.id `prvc1 = prvc2 = '_';` id2u

dun1
  an dun2
  ot -root 2.id 1`id1cnt++;` 2`atrs[dn] = id1getadd(sp[N]) | La_id1 | La_idprv; id1cnt++;`

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
  ot -root 2.id 1`id1cnt++;` 2`atrs[dn] = id1getadd(prvc1) | La_id1; id1cnt++;`

id2s
  an id
  qq . . 1`N=n;` `Q=c;` dopfx2
  ot -root 3.id `prvc1 = sp[n-2]; prvc2 = sp[n-1];` id2

id2
  an id
  ot -root 3.id `prvc1 = sp[n-2]; prvc2 = sp[n-1];` id2

id
  an
  ot -root . id

# --------------
# sign or operator. Process tentatively for nlits
# --------------
pm
  pm . . 2. sign ^= 1; ## -- -> +  +- -> -
  0 zilit0 op   2. atrs[dn] = signc;
#  nm -root
  =   root aas  2. atrs[dn] = signc;
  ot -root op   2. atrs[dn] = signc;

# ---------------------
# float literal tramp
# ---------------------
flit
  ot -root 2.nlit flit

# ---------------------
# int   literal tramp
# ---------------------
ilit
  ot -root 3.nlit ilit

# ---------------------
# prefixed integer literal
# ---------------------
zilit0
  bb ilitb0 . 2.ilitctl=1;
  oO ilito0 . 2.ilitctl=2;
  xx ilitx0 . 2.ilitctl=3;
  _
  0
  nm filit . 2. u4v = c - '0'; ## decimal with leading zero
  . flitfr . 2. fdp=0; ## 0.
  j flit   . 2. litfpi = 1;
  ot -root 2.nlit 2`atrs[dn] = 0;` 1. ilit1cnt++;

# ---------------------
# float or int literal
# ---------------------
filit
  nm . . 2. if (litbin && u4v < D32max) u4v = u4v * 10 + c - '0'; else litbin = 0;
  _
  .  flitfr . 2. fdp=0;
  ee flitxp0 .   2. fxs = 0; fxp = 0;
  j  flit . 2. litfpi = 1;
  ot -ilit

# ---------------------
# float literal fraction
# ---------------------
flitfr
  nm . . doflitfr
  _
  ee flitxp0 . 2. fxs = 0; fxp = 0;
  j  flit    . 2. litfpi = 1;
  ot -flit

# ---------------------
# float literal exp
# ---------------------
flitxp0
  + flitxp . 2. fxs = 0;
  - flitxp . 2. fxs = 1;
  nm flitxp . 2. if (litbin && fxp < E16max) fxp = fxp * 10 + c - '0'; else litbin = 0;
  _
  j  flit    . 2. litfpi = 1;

flitxp
  nm . . 2. if (litbin && fxp < E16max) fxp = fxp * 10 + c - '0'; else litbin = 0;
  _
  j  flit  . 2. litfpi = 1;
  ot -flit

# ---------------------
# int literal binary
# ---------------------
ilitb0
  0 ilitb . 2. u4v = 0;
  1 ilitb . 2. u4v = 1;
  _
  ot -ilit

ilitb
  0 . . 2. if (litbin && u4v < B32max) u4v <<= 1; else litbin = 0;
  1 . . 2. if (litbin && u4v < B32max) u4v = u4v << 1 | 1; else litbin = 0;
  _
  ot -ilit

# ---------------------
# int literal octal
# ---------------------
ilito0
  8 . . .lxerror(l,n-nlcol,$L,c,"invalid digit in octal nlit");
  9 . . .lxerror(l,n-nlcol,$L,c,"invalid digit in octal nlit");
  nm ilito . 2. u4v = c - '0';
  _
  ot -ilit

ilito
  8 . . .lxerror(l,n-nlcol,$L,c,"invalid digit in octal nlit");
  9 . . .lxerror(l,n-nlcol,$L,c,"invalid digit in octal nlit");
  nm . . 2. if (litbin && u4v < D32max) u4v = u4v << 3 | (c - '0'); else litbin = 0;
  _
  ot -ilit

# ---------------------
# int literal hex
# ---------------------
ilitx0
  nm ilitx . 2. u4v = c - '0';
  hx ilitx . 2. u4v = (c | 0x20) + 10 - 'a';
  _
  ot -ilit

ilitx
  nm . . 2. if (litbin && u4v < X32max) u4v = u4v << 4 | (c - '0'); else litbin = 0;
  hx . . 2. if (litbin && u4v < X32max) u4v = u4v << 4 | ( (c | 0x20) + 10 - 'a'); else litbin = 0;
  _
  ot -ilit

# ---------------------
# string literal start
# ---------------------
slit0
  QQ slit . .R0=1; N=n; # start of long slit
  Q root 3.slit 1`slit0cnt++;` 2slit20 # empty short slit ''
  \ -slit
1 nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  ot slit . 2`slitpool[slitx++] = c;`

# ---------------------
# string literal
# ---------------------
slit string literal
  Q slitq
1 \ slitesc1
2 \ slitesc
1 nlR00 . . .info("Q '%c' R0 %u R2 %u",Q,R0,R2); /* info(" -- '%.*s' --",64,sp + n - 32); */ lxerror(l,n-nlcol,$L,c,"newline in slit");
  nlR01 . . 2`slitpool[slitx++] = '\n';` donl
1 EOF  . . .lxerror(l,n-nlcol,$L,c,"unterminated slit");
  ot . . 2` /* info("slitpos %u x %u c '%s'",slitpos,slitx,chprint(c)); */ slitpool[slitx++] = c;`

slitesc1*
  nl slit . donl
  ot slit

# possible end of slit
slitq
  R01QQ slit9 # end of long slit
  R01   slit . 2. slitpool[slitx++] = Q; # less than 3q in long slit
  ot   -slit9 # end of short slit

# end of slit
slit9
  ot -root 3.slit 2`tk = Tslit;` 1slit1\
                  2slit2

# ---------------------
# \ esc in slit or flit
# ---------------------
slitesc*
  ot -slit . 2doesc

# ---------------------
# raw string literal start
# ---------------------
rslit0*
  QQ rslit . .R0=1; # start of long slit
  Q root 3.slit 1`slit0cnt++;` 2slit20 # empty short slit ''
  \ -rslit
1 nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  ot rslit . 2`slitpool[slitx++] = c;` # .info("c '%s'",chprint(c));

# ---------------------
# raw string literal
# ---------------------
rslit string literal
  Q rslitq
  \ rslitesc
1 nlR00 . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  nlR01 . . 2`slitpool[slitx++] = c;` donl
1 EOF  . . .lxerror(l,n-nlcol,$L,c,"unterminated slit");
  ot . . 2`slitpool[slitx++] = c;` # .info("c '%s'",chprint(c));

rslitesc
  nl rslit . 2`slitpool[slitx++] = '\\'; slitpool[slitx++] = '\n';` donl
  Q  rslit . 2.slitpool[slitx++] = '\\'; slitpool[slitx++] = Q;
  \  rslit . 2.slitpool[slitx++] = '\\';
  ot rslit . 2.slitpool[slitx++] = '\\'; slitpool[slitx++] = c;

# possible end of raw slit
rslitq
  R01QQ slit9 # end of long slit
  R01   rslit  . 2. slitpool[slitx++] = Q; # less than 3q in long slit
  ot   -slit9 # end of short slit

# ---------------------
# fstring literal start
# ---------------------
fslit0*
  QQ fslit . .R0=1; # start of long slit
  Q root 3.slit 1`slit0cnt++;` 2slit20 # empty short slit ''
  { -fslit
  } -fslit
  \ -fslit
1 nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  ot fslit . 2.slitpool[slitx++] = c;

# ---------------------
# fstring literal
# ---------------------
fslit string literal
  Q fslitq
  {{ . . 2.slitpool[slitx++] = c;
  }} . . 2.slitpool[slitx++] = c;
  {} . . 2.slitpool[slitx++] = c; slitpool[slitx++] = '}';
  { fslitxp
1 \ fslitesc
2 \ slitesc
1 R00nl . . .info("%.64s",sp + n - 32); lxerror(l,n-nlcol,$L,c,"newline in slit");
  R01nl . . donl
1 EOF  . . .lxerror(l,n-nlcol,$L,c,"unterminated slit");
  ot . . 2`slitpool[slitx++] = c;` # .info("c '%c'",c);

fslitesc*
  nl fslit . donl
  N  fslitn
  ot fslit

fslitn
  } fslit
  nl . . donl
  ot

# fstr start of expr
fslitxp
  ot -root 3.flit 2`tk = Tflit;` .sinfo(l,"start of fstr expr at pos %u",n-N); fsxplvl = bolvl; R2=1; R3=0; orgslitctl = slitctl; orgR0 = R0; orgQ = Q;\
              1slit1\
              2slit2

# possible end of fslit
fslitq
  R01QQ slit9 . .fsxplvl = hi16; # end of long slit
  R01   fslit . 2. slitpool[slitx++] = Q; # less than 3q in long slit
  ot   -slit9 . .fsxplvl = hi16; # end of short slit

# ---------------------
# raw fstring literal start
# ---------------------
frslit0*
  QQ frslit . .R0=1; # start of long slit
  Q root 3.slit 1`slit0cnt++;` 2slit20 # empty short slit ''
  \ -frslit
  { -frslit
1 nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  ot frslit . 2.slitpool[slitx++] = c;

# ---------------------
# raw fstring literal
# ---------------------
frslit string literal
  Q frslitq
  {{ . . 2.slitpool[slitx++] = c;
  }} . . 2.slitpool[slitx++] = c;
  {} . . 2.slitpool[slitx++] = c; slitpool[slitx++] = '}';
  [{{ . . 2.slitpool[slitx++] = c;
  [{ . . .info("delim at pos %u '%.8s'",n-N,sp+n-4); # delim.py
  { fslitxp
  \ frslitesc
1 R00nl . . .lxerror(l,n-nlcol,$L,c,"newline in slit");
  R01nl . . donl
1 EOF  . . .lxerror(l,n-nlcol,$L,c,"unterminated slit");
  ot . . 2`slitpool[slitx++] = c;` # 1.info("frslit '%s'",chprint(c));

frslitesc
  nl frslit . 2`slitpool[slitx++] = '\\'; slitpool[slitx++] = '\n';` donl
  Q  frslit . 2.slitpool[slitx++] = '\\'; slitpool[slitx++] = Q;
  \  frslit . 2.slitpool[slitx++] = '\\';
  ot -frslit . 2.slitpool[slitx++] = '\\'; slitpool[slitx++] = c;

# possible end of frslit
frslitq
  R01QQ slit9  . .info("sllit len %u end",n-N); fsxplvl = hi16; # end of long slit
  R01   frslit . 2. slitpool[slitx++] = Q; # less than 3q in long slit
  ot   -slit9  . .info("slit len %u end",n-N); fsxplvl = hi16; # end of short slit

