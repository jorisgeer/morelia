/* lexsyn.h - interface between lex and syn

   This file is part of Morelia, a subset of Python with emphasis on efficiency.

   Copyright © 2022 Joris van der Geer.

   Morelia is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   mpy is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program, typically in the file License.txt
   If not, see <http://www.gnu.org/licenses/>.
 */

#define Tkgrps 16

struct lexsyn {
  ub4 tkcnt;
  const ub1 *toks; // enum Token tok.h
  const ub4 *tkbits;
  const ub4 *tkpos;

  void *tkbas;

  ub1 *idnampool;

  cchar *src;
  ub4 srclen;

  const ub1 *slitpool;
  ub4 slittop;

  cchar *nlitpool;
  ub4 nlittop;

  ub2 colvlhi;
  ub4 idcnt;
  ub4 *colvlidhi;

  ub4 tkgrps[Tkgrps];

  ub2 incdircnt;
  const ub1 **incdirs;
};

#define Tkpad 10

#define Idctl_1   0xe0000000
#define Idctl_2   0xc0000000
#define Idctl_blt 0xa0000000
#define Idctl_dun 0x80000000
#define Idctl_cls 0x60000000

#define Litflt (1U << 31)
#define Litasc (1U << 30)

enum Inctype { Inone,Isys,Iuser };

extern int lexfile(ub4 fln,cchar *path,cchar *parpath,enum Inctype inc,struct lexsyn *lsp,ub8 T0);
extern int lexstr(const unsigned char * restrict str,ub2 slen,struct lexsyn *lsp,ub8 T0);
extern ub2 id2nam(ub2 id);

extern void inilex(void);
extern cchar *lex_info(void);

extern int syn(struct lexsyn *lsp);

extern void addmod(const ub1 *nam,ub4 len,bool isfile);

extern cchar *syn_info(void);
extern int inisyn(void);
