// syntab.i - LL(1-2) parser tables

/* generated by gensyn 0.1.0-alpha  5 May 2022  0:09

   from grammar minipython.syn version 0.1.0   5 May 2022  1:14 lang minipython
 */

static const char synfname[] = "minipython.syn";
static const char syninfo[] = "minipython.syn 0.1.0   5 May 2022  1:14 minipython";

enum Packed8 Nterm { // 5
  Nstmts   =  0, Nstmt    =  1, Nblock   =  2, Natom    =  3, Nexpr    =  4, Ncount =  5
};

static ub2 ntnampos[5] = { 0,6,11,17,22 };

static const char ntnampool[32] = "stmts\0stmt\0block\0atom\0expr\0";

enum Packed8 Symbol { // 23
  Sbreak   =  0, Selse    =  1, Sif      =  2, Swhile   =  3, Sid      =  4, Snlit    =  5, Sslit    =  6, Sast     =  7, 
  Spm      =  8, Sop      =  9, Sco      = 10, Scc      = 11, Sro      = 12, Src      = 13, Sqas     = 14, Scolon   = 15, 
  Ssepa    = 16, 
  Stoken   = 17,

  Sstmts   = 17, Sstmt    = 18, Sblock   = 19, Satom    = 20, Sexpr    = 21, 
  Smrgset  = 22,

  SPmOpAs  = 22, Scount   = 23
};

typedef ub1 Mrgbits;

static Mrgbits tkmrgtab[17] = { 0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0 };

// < tablen = sentry < 1dirtok = argdir < laid = la

static const ub2 vprdmap[10] = { 0x0,0x0,0x101,0x102,0x103,0x104,0x205,0x206,0x307,0x408 };

// s0.1 rep.1 re1.1 si.2 len.4 
static const ub1 syntabeas[10] = {
  0x82, // *2
  0x92, // *1.2
  0x42, // >2
  1,
  0x46, // >6
  0x46, // >6
  0x42, // >2
  1,
  0x42, // >2
  0x82  // *2
};

static const enum Nterm startrule = Nstmts; // 0
static const ub1 startve = 0; // stmts_0_0
#define R0n 0x30
#define Rlp 0x38
#define R01 8
#define R02 0x10
#define R03 0x18

static const struct sentry syntab[9] = { // 9 * 20 = 180 B

//  s0       se       line     alt      rule     prod     atr
//  ̲s̲t̲m̲t̲ ̲ ̲ ̲ ̲ ̲0̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲3̲2̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲0̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲s̲t̲m̲t̲s̲ ̲ ̲ ̲ ̲s̲t̲m̲t̲s̲_̲0̲_̲0̲ ̲*̲2
{ { Sstmt   ,Ssepa   ,0       ,0       ,0       ,0       ,0,0,0,0 },
  { R01     ,Rlp     ,0       ,0       ,0       ,0       ,0,0,0,0 } },

//  ̲i̲d̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲1̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲3̲5̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲0̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲s̲t̲m̲t̲ ̲ ̲ ̲ ̲ ̲s̲t̲m̲t̲_̲0̲ ̲ ̲ ̲>̲2
{ { Sqas    ,Sexpr   ,0       ,0       ,0       ,0       ,0,0,0,0 },
  { 0       ,0       ,0       ,0       ,0       ,0       ,0,0,0,0 } },

//  ̲e̲x̲p̲r̲ ̲ ̲ ̲ ̲ ̲2̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲3̲6̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲1̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲s̲t̲m̲t̲ ̲ ̲ ̲ ̲ ̲s̲t̲m̲t̲_̲1̲ ̲ ̲ ̲1 la 0  stmt_0 stmt_1
{ { Sexpr   ,0       ,0       ,0       ,0       ,0       ,0,0,0,0 },
  { 1       ,0       ,0       ,0       ,0       ,0       ,0,0,0,0 } },

//  ̲i̲f̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲3̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲3̲8̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲3̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲s̲t̲m̲t̲ ̲ ̲ ̲ ̲ ̲i̲f̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲>̲6
{ { Sexpr   ,Scolon  ,Sblock  ,Selse   ,Scolon  ,Sblock  ,0,0,0,0 },
  { 1       ,0       ,0       ,R03     ,0       ,0       ,0,0,0,0 } },

//  ̲w̲h̲i̲l̲e̲ ̲ ̲ ̲ ̲4̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲3̲9̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲4̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲s̲t̲m̲t̲ ̲ ̲ ̲ ̲ ̲w̲h̲i̲l̲e̲ ̲ ̲ ̲ ̲>̲6
{ { Sexpr   ,Scolon  ,Sblock  ,Selse   ,Scolon  ,Sblock  ,0,0,0,0 },
  { 1       ,0       ,0       ,R03     ,0       ,0       ,0,0,0,0 } },

//  ̲c̲o̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲5̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲4̲2̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲0̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲b̲l̲o̲c̲k̲ ̲ ̲ ̲ ̲b̲l̲o̲c̲k̲ ̲ ̲ ̲ ̲>̲2
{ { Sstmts  ,Scc     ,0       ,0       ,0       ,0       ,0,0,0,0 },
  { 0       ,0       ,0       ,0       ,0       ,0       ,0,0,0,0 } },

//  ̲s̲t̲m̲t̲ ̲ ̲ ̲ ̲ ̲6̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲4̲3̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲1̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲b̲l̲o̲c̲k̲ ̲ ̲ ̲ ̲b̲l̲k̲_̲s̲t̲m̲t̲ ̲1
{ { Sstmt   ,0       ,0       ,0       ,0       ,0       ,0,0,0,0 },
  { 0       ,0       ,0       ,0       ,0       ,0       ,0,0,0,0 } },

//  ̲r̲o̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲7̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲5̲4̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲3̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲a̲t̲o̲m̲ ̲ ̲ ̲ ̲ ̲g̲r̲p̲e̲x̲p̲ ̲ ̲ ̲>̲2
{ { Sexpr   ,Src     ,0       ,0       ,0       ,0       ,0,0,0,0 },
  { 0       ,0       ,0       ,0       ,0       ,0       ,0,0,0,0 } },

//  ̲a̲t̲o̲m̲ ̲ ̲ ̲ ̲ ̲8̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲5̲7̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲0̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲ ̲e̲x̲p̲r̲ ̲ ̲ ̲ ̲ ̲e̲x̲p̲r̲_̲0̲ ̲ ̲ ̲*̲2
{ { Satom   ,SPmOpAs ,0       ,0       ,0       ,0       ,0,0,0,0 },
  { 1       ,Rlp|2   ,0       ,0       ,0       ,0       ,0,0,0,0 } }
};

// max len 6 for 3
#define Syn_maxlen 6

#undef R0n
#undef Rlp
#undef R01
#undef R01
#undef R02
#undef R03
static const struct seinfo stinfo[9] = { // 9 * 16 = 144

// lno  alt s0       desc
 {  32,  0, Sstmt    , "stmt?  ;"                       }, //  0 stmts
 {  35,  0, Sid      , " id  := expr"                   }, //  1 stmt
 {  36,  1, Sexpr    , "expr`1"                         }, //  2 stmt
 {  38,  3, Sif      , " if expr`1  : block  else  : block" }, //  3 stmt
 {  39,  4, Swhile   , " while expr`1  : block  else  : block" }, //  4 stmt
 {  42,  0, Sco      , " { stmts  }"                    }, //  5 block
 {  43,  1, Sstmt    , "stmt"                           }, //  6 block
 {  54,  3, Sro      , " ( expr?  )"                    }, //  7 atom
 {  57,  0, Satom    , "atom`1 PmOpAs`2"                }  //  8 expr
};

#define Lacnt 1

static const ub1 lasetn[1] = { 2 };

#define x 255
static const ub1 lasets[4] = {  2, 3,x,x };
//                            1  2 . .
#undef x

#define Lasets 1
typedef ub4 lasec_t;

static const lasec_t laseclst[Lasets * Laset] = { // 1 * 4 * 4 = 16
// laid 0 tk id
  0x4000,0x1b3fc,0,0 //se 1 ln 35 qas se 2 ln 36 ~ break else co cc qas . . 

};

typedef ub1 lasecs_t;
#define x 255
static const lasecs_t lasecmap[Lacnt * T99_count] = { // 1 * 17 = 17
  x,x,x,x,0,x,x,x,x,x,x,x,x,x,x,x,x
};
#undef x

#if 0
typedef ub4 lasec_t;
static const lasec_t laseconds[Lacnt * T99_count* Laset] = { // 1 * 17 * 4 * 4 = 272 B
  0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 ,0x4000,0x1b3fc, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0
  , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0 , 0, 0, 0, 0
  , 0, 0, 0, 0 , 0, 0, 0, 0
};
#endif
/* mrg sets

  0  PmOpAss  ast pm op  ln 57
*/

#define x Pcount
#define X Pendrep
#define Stbl_tklen 17

static const enum Production prdsel[Ncount * Stbl_tklen] = { // 5 * 17 = 85
// ------ line  32 stmts ------
   Pstmts_0_0,      X,               Pstmts_0_0,      Pstmts_0_0,      
//  0  break         .               0  if           0  while       
   Pstmts_0_0,      Pstmts_0_0,      Pstmts_0_0,      X,               
//  0  id           0  nlit         0  slit        
   X,               X,               X,               X,               
   Pstmts_0_0,      X,               X,               X,               
//  0  ro          
   Pstmts_0_1,      
//  1 1sepa        

// ------ line  35 stmt ------
   Pbreak_stmt,     x,               Pif,             Pwhile,          
//    0.break         .               4  if           5  while       
   Plaid_0,         Pstmt_1,         Pstmt_1,         x,               
//    0?id           3  nlit         3  slit        
   x,               x,               x,               x,               
   Pstmt_1,         x,               x,               x,               
//  3  ro          
   x,               

// ------ line  42 block ------
   Pbreak_stmt,     x,               Pblk_stmt,       Pblk_stmt,       
//    0.break         .               7  if           7  while       
   Pblk_stmt,       Pblk_stmt,       Pblk_stmt,       x,               
//  7  id           7  nlit         7  slit        
   x,               x,               Pblock,          x,               
//   .                .               6  co          
   Pblk_stmt,       x,               x,               x,               
//  7  ro          
   x,               

// ------ line  51 atom ------
   x,               x,               x,               x,               
   Pid_atom,        Pnlit_atom,      Pslit_atom,      x,               
//    0.id             0,nlit           0,slit        
   x,               x,               x,               x,               
   Pgrpexp,         x,               x,               x,               
//  8  ro          
   x,               

// ------ line  57 expr ------
   X,               X,               X,               X,               
   Pexpr_0,         Pexpr_0,         Pexpr_0,         X,               
//  9  id           9  nlit         9  slit        
   X,               X,               X,               X,               
   Pexpr_0,         X,               X,               X,               
//  9  ro          
   X                

};

#undef x
#undef X

// 25 ve entries  5 dir entries

static const ub2 poolsizes = 206;
#define Latabsizes 38

