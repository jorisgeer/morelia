/*  synast.h - common defines between parser and ast builder

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

enum Astyp { Aid,Ailit,Aflit,Aslit,Ailits,
  Aop,
  Apexp,Auexp,Abexp,Aaexp,
  Aasgnst,
  Awhile,
  Afndef,
  Ablk,
  Aparam,
  Arexp,Aprmlst,Astmts,Acount };

#define Aval Aslit
#define Aleaf Ailits
#define Arep Arexp
#define Aback 0x20

// #define Repcnt 0x8000
#define Explen 1024

#define Atybit 26
#define Atymsk 0x3ffffff

#define Aopbit 21
#define Aopmsk 0x1f

#define Acntlim (1U << Aopbit)

// #define Ndcnt 26
// #define Ndcntmsk 0xff

struct synast {
  struct rnode *nodes;
  ub4 *args;
  ub8 *vals;
  ub4 idcnt,uidcnt;
  ub4 ndcnt,argcnt;
  ub4 nscid;
  ub2 hiblklvl;
  ub4 ndcnts[Ptablen];
};

#define Nodarg 4

struct rnode {
  enum Production ve;

  ub1 lvl;
  ub1 blklvl;
  ub2 amask;

  ub4 ni;
//  ub4 ai;  // argndx
  ub4 sib; // sibling for rep
};

extern int syn(struct lexsyn *lsp,struct synast *sa);

extern void *mkast(struct synast *sa,struct lexsyn *lsp);
extern cchar *atynam(enum Astyp t);

extern enum Astyp prd2nod[];
