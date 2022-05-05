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

enum Astyp { Aid,Ailit,Ailits,Aflit,Aslit,Atoken,Aop,Apexp,Auexp,Abexp,Aiter,Arep,Arexp,Aargs,Astmts,Acount };

// #define Repcnt 0x8000
#define Explen 1024

#define Atybit 27
#define Atymsk 0x7ffffff

#define Aopbit 23
#define Aopmsk 0xf

#define Acntlim (1U << Aopbit)

// #define Ndcnt 26
// #define Ndcntmsk 0xff

struct synast {
  struct rnode *nodes;
  ub4 *args;
  ub4 ndcnt,argcnt;
  ub4 ndcnts[Ptablen];
};

struct rnode {
  enum Production ve;

  ub1 lvl;
  ub1 blklvl;
  ub2 amask;

  ub4 ni;
  ub4 ai;  // argndx
  ub4 sib; // sibling for rep
};

extern int syn(struct lexsyn *lsp,struct synast *sa);

extern void *mkast(struct synast *sa,struct lexsyn *lsp);
extern cchar *atynam(enum Astyp t);

extern enum Astyp prd2nod[];
