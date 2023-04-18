/* base.h - base definitions

   This file is part of Morelia a variation on a Python theme.

   Copyright © 2023 Joris van der Geer.

   Morelia is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Morelia is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program, typically in the file License.txt
   If not, see http://www.gnu.org/licenses.
 */

#ifndef __STDC__
 #error "missing __STDC__ : require iso c 99+"
#endif
#ifndef __STDC_VERSION__
 #error "missing __STDC_VERSION__ : require iso c 99+"
#endif
#if __STDC_VERSION__ < 199901L
 #error "require iso c 99+"
#endif

// basic types
typedef _Bool          bool;
typedef unsigned char  ub1;
typedef unsigned short ub2;
typedef unsigned int   ub4;
typedef unsigned long  ub8;
typedef unsigned long  long ub16;

typedef signed char sb1;
typedef short  sb2;
typedef int    sb4;
typedef long   sb8;
typedef long   long sb16;

typedef const char cchar;

// versioning
#define Lang "morelia"

#define Version_maj 0
#define Version_min 1
#define Version_pat 2
#define Version_phase "dev-a"
#define Version_str "0.1.2-dev-a"

// handful of useful macros
#define hi15 0x7fff
#define hi16 0xffffU
#define hi20 0xfffffU
#define hi24 0xffffffU
#define hi28 0xfffffffU
#define hi31 0x7fffffffU
#define hi32 0xffffffffU
#define hi56 0xffffffffffffffUL
#define hi64 0xffffffffffffffffUL

#define Bit15 0x8000U
#define Bit30 0x40000000U
#define Bit31 0x80000000U
#define Bit63 0x8000000000000000UL

#ifndef max
 #define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
 #define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef nil
 #define nil (void*)0
#endif

// c11 langage only
#if __STDC_VERSION__ >= 201101L
 #define sassert(expr,msg) _Static_assert((expr),msg)
 #define Func __func__
 #define Noret _Noreturn

#else
 #define sassert(expr,msg) assert((expr),msg)
 #define Func ""
 #define Noret
 #define quick_exit(c) _Exit(c)
#endif

// enable extensions conditionally
#if defined __clang__ && defined __clang_major__ && !defined D_BetterC
 #define Isclang 1
 #define Isgcc 0
 #define Packed8 __attribute__((packed))
 #define Fallthrough __attribute__ ((fallthrough));
 #define Mallike __attribute__ ((malloc))
 #define Printf(fmt,ap) __attribute__ ((format (printf,fmt,ap)))

 #pragma clang diagnostic error "-Wimplicit-function-declaration"
 #pragma clang diagnostic error "-Wincompatible-pointer-types"
 #pragma clang diagnostic error "-Wconditional-uninitialized"
 #pragma clang diagnostic error "-Wuninitialized"
 #pragma clang diagnostic error "-Wmissing-field-initializers"
 #pragma clang diagnostic error "-Wmissing-prototypes"
 #pragma clang diagnostic error "-Wint-conversion"

// #pragma clang diagnostic ignored "-Wunused-variable"
// #pragma clang diagnostic ignored "-Wunused-parameter"
 #pragma clang diagnostic ignored "-Wunused-but-set-variable"

 #pragma clang diagnostic ignored "-Wgnu-label-as-value"
 #pragma clang diagnostic ignored "-Wdeclaration-after-statement"
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wunused-macros"

 #pragma clang diagnostic ignored "-Wchar-subscripts"
 #pragma clang diagnostic ignored "-Wimplicit-int-conversion"
 #pragma clang diagnostic ignored "-Wpadded"
 #pragma clang diagnostic ignored "-Wpointer-sign"
 #pragma clang diagnostic ignored "-Wempty-translation-unit"
 #pragma clang diagnostic ignored "-Wformat-pedantic"

 // temporary
 #pragma clang diagnostic ignored "-Wcast-qual"
 #pragma clang diagnostic ignored "-Wassign-enum"
 #pragma clang diagnostic ignored "-Wswitch-enum"
 #pragma clang diagnostic ignored "-Wshorten-64-to-32"

 #ifdef __has_feature
  #if __has_feature(address_sanitizer)
   #define Asan
  #endif
 #endif

#elif defined __GNUC__ && __GNUC__ > 3 && !defined D_BetterC
 #define Isclang 0
 #define Isgcc 1
 #define Packed8 __attribute__((packed))
 #define Fallthrough __attribute__ ((fallthrough));
 #define Mallike __attribute__ ((malloc))
 #define Inline __attribute__ ((always_inline))
 #define Printf(fmt,ap) __attribute__ ((format (printf,fmt,ap)))

// #pragma GCC diagnostic ignored "-Wattributes"
// #pragma GCC diagnostic ignored "-Wconversion"
// #pragma GCC diagnostic ignored "-Wunused-variable"
// #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
 #pragma GCC diagnostic ignored "-Wpointer-sign"

 #pragma GCC diagnostic warning "-Wdisabled-optimization"

 #pragma GCC diagnostic error "-Wshadow"
 #pragma GCC diagnostic error "-Wmaybe-uninitialized"
 #pragma GCC diagnostic error "-Wimplicit-function-declaration"
 #pragma GCC diagnostic error "-Wmissing-field-initializers"
 #pragma GCC diagnostic error "-Wvla"
 #pragma GCC diagnostic error "-Warray-bounds"

 #ifdef __SANITIZE_ADDRESS__
  #define Asan
 #endif

#else
 #define Isclang 0
 #define Isgcc 0
 #define Packed8
 #define Attribute(name)
 #define Fallthrough
 #define Inline inline
 #define Mallike
#endif

// program-wide global vars go here
struct globs {
  cchar *prgnam;
};
extern struct globs globs;
