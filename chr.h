/* chr.h - char utilities

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

extern ub2 dochprint(ub1 c,ub1 *p);
extern bool is_print(ub1 c);
extern cchar *chprint(ub1 c);
extern const ub1 *chprints(const ub1 *s,ub4 n);
extern const ub1 *chprintn(const ub1 *s,ub4 n,ub4 *pnw,ub1 ctl);
extern ub2 underline(char *buf,ub2 len);
extern char upcase(char c);

enum Packed8 Ctype { Csq=1,Cdq,Cbs,Calpha,Cnum,Cdot,Cws,Cnl,Chsh,Cpls,Cmin,Cast,Cqst,Ccomma,Crdo,Crdc,Cbtk,Cor,Ceq,Cother };

extern const enum Ctype Ctab[256];
