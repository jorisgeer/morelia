/* ast.h - ast defines

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

#define Depth 256

enum Uop { Uinc,Udec };

enum Bop { Badd,Bmul };

enum Role { Lhs,Rhs };

struct ilit { // term ilit
  ub4 val;
};

struct id { // term id
  ub4 id;
  ub4 def;  // defining var, self if none
  ub4 ofs;  // frame offset
  ub4 scid; // scope id
  ub1 lvl;
  enum Role rol;
};

// nonterms
struct uexp {
  ub4 e;
  enum Uop op;
};

struct bexp {
  ub4 l,r;
  enum Bop op;
};

struct aexp {
  ub4 id;
  ub4 e;
};

struct pexp { // partial binexp
  ub4 l;
  enum Bop op;
};

struct asgnst {
  ub4 tgt;
  ub4 r;
};

struct blk {
  ub4 s;
};

struct param {
  ub4 id;
  ub4 t;
  ub4 def;
};

struct fndef {
  ub4 id;
  ub4 scid0,scid1;
  ub4 plst;
  ub4 blk;
  ub4 ret;
};

struct witer {
  ub4 e;
  ub4 tb,fb;
};

struct rexp {
  ub4 pos;
  ub2 cnt;
  ub2 len;
};

struct prmlst {
  ub4 pos;
  ub2 cnt;
};

struct stmtlst {
  ub4 pos;
  ub2 cnt;
};

struct ast {
  ub4 len;
  ub4 root;

  struct stmtlst *stmtls;
  struct id *ids;
  struct ilit *ilits;
  struct uexp *uexps;
  struct pexp *pexps;
  struct bexp *bexps;
  struct aexp *aexps;

  struct rexp *rexps;

  struct asgnst *asgnsts;

  struct blk *blks;

  struct fndef *fndefs;
  struct param *prms;

  struct witer *witers;

  ub4 nid;
  ub4 uidcnt;

  ub4 *idfpos;

  ub4 *repool;

  ub8 *scids;
  ub2 *scidns;
  ub4 nscid;

  ub1 blkbit;
};
