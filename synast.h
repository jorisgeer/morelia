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

struct synast {
  struct rnode *nodes;
  ub4 *args;
  ub8 *vals;
  ub4 *nhs;
  ub4 *tis;
  ub4 idcnt,uidcnt,aidcnt;
  ub4 ndcnt,argcnt,valcnt;
  ub4 nscid;
  ub2 hiblklvl;
  ub4 ndcnts[Acount+1];
  ub4 rep2cnts[Acount+1];
};

#define Nodarg 4

struct rnode {
  enum Production ve;

  ub1 lvl;
//  ub1 blklvl;
  ub2 amask; // count for reps

  ub4 ni;
};

extern int syn(struct lexsyn *lsp,struct synast *sa);

extern void *mkast(struct synast *sa,struct lexsyn *lsp);
