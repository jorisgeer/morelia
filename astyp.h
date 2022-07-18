/*  astyp.h - ast types

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

enum Astyp { Aid,Avar,Ailit,Aflit,Aslit,Ailits,Atru,Afal,Akwd,
  Aop,
  Asubscr,
  Afstr,
  Apexp,Auexp,Abexp,Aaexp,Agrpexp,
  Ablk,
  Aif,
  Awhile,
  Afndef,
  Aparam,
  Aasgnst,
  Aexpst,
  Astmt,
  Afstring,
  Arexp,Afstrlst,Aprmlst,Astmtlst,Acount };

enum Packed8 Uop { Oupls,Oumin,Onot,Oneg,Oucnt };
enum Packed8 Bop { Onil,Orelor,Oreland,One,Oeq,Oshl,Oshr,Oxor,Oor,Oand,Oadd,Osub,Omul,Odiv,ODiv,Omod,Omxm,Oexp,Olt,Ogt,Ole,Oge,Obcnt };

#define Aval Aslit
#define Aleaf Akwd
#define Arep Arexp

#define Atybit 26
#define Atymsk 0x3ffffff

extern cchar *atynam(enum Astyp t);
