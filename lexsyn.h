/* lexsyn.h - interface between lex and syn

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

#define Tkgrps 16

struct lexsyn {
  ub4 tkcnt,tbcnt;
  const ub1 *toks; // enum Token tok.h
  const ub1 *atrs;
  const ub8 *bits;
  const ub2 *dfps;

  void *tkbas;

  ub1 *idnampool;

  cchar *src;
  ub4 srclen;

  const ub1 *slitpool;
  ub4 slitcnt,slittop;

  cchar *nlitpool;
  ub4 nlitcnt,nlittop;

  ub2 hidepth;
  ub4 idcnt,uidcnt;
  ub2 uid1cnt,uid2cnt;

  ub4 tkgrps[Tkgrps];

  cchar *name;

  ub2 incdircnt;
  const ub1 **incdirs;
};

#define Tkpad 10

#define Idlen_2   0x80
#define Idlen_n   0x81

#define Idpool    0x3fffffff

#define Idctl_blt 0x82
#define Idctl_dun 0x83
#define Idctl_cls 0x84

#define Slit_len 0x80

#define Litflt 0x85
#define Litasc 0x86

enum Packed8 Lop { Lolit,Lorelor,Loreland,Lone,Loeq,Loshl,Loshr,Lonot,Loxor,Loneg,Loor,Loand,Loumin,Loupls,Lomin,Lopls,Lomul,Lodiv,Lomod,Loas,Lolt,Logt,Lole,Loge,Loqst,Locol,Locom,Locnt };

enum Inctype { Inone,Isys,Iuser };

extern int lexfile(ub4 fln,cchar *path,cchar *parpath,enum Inctype inc,struct lexsyn *lsp,ub8 T0);
extern int lexstr(const unsigned char * restrict str,ub2 slen,struct lexsyn *lsp,ub8 T0);
extern ub1 *idnam(ub4 id);

extern void inilex(void);
extern cchar *lex_info(void);

extern void addmod(const ub1 *nam,ub4 len,bool isfile);

extern cchar *syn_info(void);
extern int inisyn(void);
