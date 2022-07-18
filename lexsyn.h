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

#define Tkgrps 8

struct lexsyn {
  ub4 tkcnt,tbcnt,cmtcnt;
  const ub1 *toks; // enum Token tok.h
  const ub2 *atrs;
  const ub8 *bits;
  const ub4 *fpos;
  const ub4 *cmts;

  void *tkbas;

  ub1 *idnampool;

  cchar *orgsrc; // as allocated for read
  cchar *src;    // moved for bom / shebang
  ub4 srclen;

  const ub1 *slitpool;
  ub4 slitcnt,slittop;

  ub4 nlitcnt;

  ub2 hidepth;
  ub4 idcnt,uidcnt;
  ub2 uid1cnt,uid2cnt;

  ub4 tkgrps[Tkgrps];

  cchar *name;

  ub2 incdircnt;
  const ub1 **incdirs;
};

#define Idlen 96

#define Tkpad 10

#define Idpool    0x3fffffff

#define Lxop2 0x200
#define Lxoe  0x100

#define La_bit 12
#define La_msk 0xfff

//      La_id4s  0
#define La_idprv 0x1000

#define La_id1   0x2000
#define La_id2   0x4000
#define La_id4   0x6000

#define La_idblt 0x8000
#define La_iddun 0xa000
#define La_id_   0xc000

//      La_ilit2  0
#define La_ilit2n 0x1000
#define La_ilit4  0x2000
#define La_ilit4n 0x3000
#define La_flit8  0x4000
#define La_flit8n 0x5000
#define La_ilita  0x6000
#define La_flita  0x8000

//      La_slit1 0
#define La_slit0 0x2000
#define La_slit2 0x4000
#define La_slit3 0x6000
#define La_slits 0x8000
#define La_slit  0xa000

#define Lxabit 26
#define Lxamsk 0x3ffffff
#define Lx_sign 0x80000000

enum Inctype { Inone,Isys,Iuser };

extern int lexfile(ub4 fln,cchar *path,cchar *parpath,enum Inctype inc,struct lexsyn *lsp,ub8 T0);
extern int lexstr(const unsigned char * restrict str,ub2 slen,struct lexsyn *lsp,ub8 T0);
extern ub1 *idnam(ub4 id);

extern void inilex(void);
extern cchar *lex_info(void);

extern void addmod(const ub1 *nam,ub4 len,bool isfile);

extern cchar *syn_info(void);
extern int inisyn(void);
