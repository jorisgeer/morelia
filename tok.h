/* tok.h - lexer token defines

   generated by genlex 0.1.0-alpha  5 May 2022  0:09

   from minipython.lex 0.1.0  4 May 2022 23:07 minipython
   signature: @ aa45804987ede1b0 @ */

enum Packed8 Token {
  Tbreak    =  0, Telse     =  1, Tif       =  2, Twhile    =  3, T99_eof   =  3, 
  T99_kwd   =  4,
  Tid       =  4, Tnlit     =  5, Tslit     =  6, 
  T0grp     =  6,
  Tast      =  7, Tpm       =  8, Top       =  9, 
  T1grp     =  9,
  Tco       = 10, 
  T2grp     = 10,
  Tcc       = 11,
  
  T3grp     = 11,
  Tro       = 12, Trc       = 13, Tqas      = 14, Tcolon    = 15, Tsepa     = 16, 
  T4grp     = 16,
  
  T99_count = 17
};

#define Kwcnt 4
#define Tknamlen 7
#define Tkgrp 5

static const ub2 toknampos[14] = {
  0,3,8,13,17,20,23,26,29,32,35,39,45,50
};

static const char toknampool[50] = "id\0nlit\0slit\0ast\0pm\0op\0co\0cc\0ro\0rc\0qas\0colon\0sepa\0";

#define tkwnampool kwnampool
#define tkwnamposs kwnamposs
#define tkwnamlens kwnamlens
#define t99_count T99_kwd

#define token Token

static const ub1 tokhrctl[13] = { 0,0,0,1,0x11,0,1,1,1,1,2,1,1 };

static char hrtoknams[52] = "            *   +       {   }   (   )   :=  :   ;   "; // 13 * 4

enum Packed8 Bltin { B99_count = 1 };

enum Packed8 Dunder { D99_count = 1 };

static const char kwnampool[ 21] = "break   elseif  while";
static const ub2  kwnamposs[  4] = { 0,8,12,16 };
static const ub1  kwnamlens[  4] = { 5,4,2,5 };

static const ub4 kwnamhsh = 0xa31e56ac;

