/* exp.h - expressions and operators

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

enum Packed8 Op { Olit,Orelor,Oreland,One,Oeq,Oshl,Oshr,Onot,Oxor,Oneg,Oor,Oand,Oumin,Oupls,Omin,Opls,Omul,Odiv,Omod,Oas,Olt,Ogt,Ole,Oge,Oqst,Ocol,Ocom,Ocnt };
