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

struct stmtlst {
  ub4 pos;
  ub2 cnt;
};

struct ilit { // term ilit
  ub4 val;
};

struct id { // term id
  ub4 id;
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

struct pexp {
  ub4 l;
  enum Bop op;
};

struct rexp {
  ub4 pos;
  ub2 cnt;
  ub2 len;
};

struct args {
  ub4 pos;
  ub2 cnt;
};

struct loop {
  ub4 iter;
  ub4 body;
};

struct ast {
  ub4 len;
  ub4 root;

  struct stmtlst *stmtls;
  struct id *ids;
  struct ilit *ilits;
  struct uexp *uexps;
  struct pexp *pexps;
  struct rexp *rexps;

  ub4 *repool;
};
