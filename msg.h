/* msg.h -messages, logging and assertions

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

/* Defines for console, debug and assert messages
   Most entries are macros to add file and line to the functions
   Assertions are here for trivial inlining or low-overhead calling
 */

enum Msglvl { Fatal,Assert,Oserror,Error,Warn,Info,Vrb,Vrb2,Nolvl };

enum Msgopts {
  Msg_shcoord = 1,
  Msg_fno = 2,
  Msg_lno = 4,
  Msg_col = 8,
  Msg_lvl = 0x10,
  Msg_tim = 0x20
};

struct fnaminf {
  cchar *path;
  cchar *name;
  ub4 *lntab;
  ub4 lncnt;
  ub4 len;
  ub4 fpos0;
  ub4 parfpos;

  ub8 ino;
  ub8 dev;

  ub2 incdir;
  ub2 dirsep;
  ub2 pfxno;
};

extern struct fnaminf *getsrcmfile(void);
extern int chksrcmfile(struct fnaminf *mf);
extern void setsrcmfile(struct fnaminf *mf,ub4 *lintab,ub4 lincnt,ub4 len);

extern ub4 getsrcln(ub4 fpos);

extern ub4 msgwarncnt(void);
extern ub4 msgerrcnt(void);

// arrange file coords
#define FLN __LINE__|(msgfile<<16)

#define Lno 0x80000000

#define genmsg(lvl,fmt,...) genmsgfln(FLN,(lvl),(fmt),__VA_ARGS__)

#define vrb2(fmt,...) vrb2fln(FLN,fmt,__VA_ARGS__)

#define vrb(fmt,...) vrbfln(FLN,fmt,__VA_ARGS__)
#define svrb(fpos,fmt,...) svrbfln(FLN,fpos,fmt,__VA_ARGS__)

#define info(fmt,...) infofln(FLN,fmt,__VA_ARGS__)
#define sinfo(fpos,fmt,...) sinfofln(FLN,fpos,fmt,__VA_ARGS__)
#define sinfo0(fpos,s) sinfofln(FLN,fpos,"%s",s)

#define warning(fmt,...) warnfln(FLN,fmt,__VA_ARGS__)
#define swarn(fpos,fmt,...) swarnfln(FLN,fpos,fmt,__VA_ARGS__)

#define error(fmt,...) errorfln(FLN,0,fmt,__VA_ARGS__)
#define serror(fpos,fmt,...) serrorfln(FLN,fpos,fmt,__VA_ARGS__)

#define oserror(fmt,...) oserrorfln(FLN,fmt,__VA_ARGS__)
#define fatal(fln,fpos,fmt,...) fatalfln(FLN,fln,fpos,fmt,__VA_ARGS__)

#define ice(fln,fpos,fmt,...) icefln(FLN,fln,fpos,fmt,__VA_ARGS__)

#define assert(exp,msg) if (!(exp)) assertfln(FLN,"%s - %s",#exp,msg)

#define showcnt(n,c) showcntfln(FLN,n,c)
#define showsiz(n,c) showsizfln(FLN,n,c)

#define timeit(t,m)    timeitfln(FLN,t,m)
#define timeit2(t,n,m) timeit2fln(FLN,t,n,m)

extern void showcntfln(ub4 fln,cchar *nam,ub4 cnt);
extern void showsizfln(ub4 fln,cchar *nam,ub4 siz);

extern ub4 msgfln(char *dst,ub4 pos,ub4 len,ub4 fln,ub4 wid);
extern void msg_write(const char *buf,ub4 len);
extern void msg_errwrite(ub4 fln,ub4 fln2,const char *buf);

extern void inimsg(ub2 opts);
extern void eximsg(void);

extern void setmsglvl(enum Msglvl lvl,ub2 opts);
extern enum Msglvl getmsglvl(void);
extern int setmsglog(const char *dir,const char *logname,int newonly,int show);

#define msgopt(s) msgopt_fln(FLN,(s))

#ifdef va_start
 extern void   vmsg(ub4 shfln,enum Msglvl lvl,ub4 fpos,cchar *fmt,va_list ap);
 extern void  vpmsg(ub4 shfln,enum Msglvl lvl,cchar *srcnam,ub4 lno,ub4 col,cchar *fmt,va_list ap,cchar *sfx);
 extern void vmsgps(ub4 shfln,enum Msglvl lvl,ub4 fpos,cchar *fmt,va_list ap,cchar *pfx,cchar *sfx);
#endif

extern void genmsgfln(ub4 fln,enum Msglvl lvl,const char *fmt,...) __attribute__ ((format (printf,3,4)));

extern void vrb2fln(ub4 fln,const char *fmt,...) __attribute__ ((format (printf,2,3)));

extern void vrbfln(ub4 fln,const char *fmt,...) __attribute__ ((format (printf,2,3)));
extern void svrbfln(ub4 fln,ub4 fpos,const char *fmt,...) __attribute__ ((format (printf,3,4)));

extern void infofln(ub4 fln,const char *fmt, ...) __attribute__ ((format (printf,2,3)));
extern void sinfofln(ub4 fln,ub4 fpos,const char *fmt,...) __attribute__ ((format (printf,3,4)));

extern void warnfln(ub4 fln,const char *fmt,...) __attribute__ ((format (printf,2,3)));
extern void swarnfln(ub4 fln,ub4 fpos,const char *fmt,...) __attribute__ ((format (printf,3,4)));

Noret extern void serrorfln(ub4 shfln,ub4 fpos,const char *fmt,...) __attribute__ ((format (printf,3,4)));

extern void errorfln(ub4 fln,ub4 fln2,const char *fmt,...) __attribute__ ((format (printf,3,4)));

extern void oserrorfln(ub4 fln,const char *fmt, ...) __attribute__ ((format (printf,2,3)));
extern void oswarnfln(ub4 fln,const char *fmt, ...) __attribute__ ((format (printf,2,3)));
Noret extern void assertfln(ub4 fln,const char *fmt, ...) __attribute__ ((format (printf,2,3)));
Noret extern void fatalfln(ub4 fln,ub4 fln2,ub4 fpos,const char *fmt,...) __attribute__ ((format (printf,4,5)));
Noret extern void icefln(ub4 fln,ub4 fln2,ub4 fpos,const char *fmt,...) __attribute__ ((format (printf,4,5)));

extern ub4 limit_gt_fln(ub4 x,ub4 lim,ub4 arg,const char *sx,const char *slim,const char *sarg,ub4 fln);

extern void timeitfln(ub4 fln,ub8 *pt0,cchar *msg);
extern void timeit2fln(ub4 fln,ub8 *pt0,ub4 n,cchar *msg);

extern void msglog(cchar *fnam,cchar *fext,cchar *desc);

extern int msgopt_fln(ub4 fln,const char *name);
extern void msgfls(void);
extern void setmsgbuf(bool ena);
