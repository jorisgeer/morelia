/* os.h - operating system specifics

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

struct osstat {
  ub8 mtime;
  ub8 len;
  ub8 ino;
  ub8 dev;
};

extern ub8 gettime_usec(void);
extern int osclock_gettime(ub8 *pusec);
extern void osmillisleep(ub4 msec);

#define osmmap(cnt,el) (el *)osmmapfln(FLN,(cnt),sizeof(el))
#define osmremap(p,el,ocnt,ncnt) (el *)osmremapfln(FLN,(p),sizeof(el),(ocnt),(ncnt))
#define osmunmap(p,n) osmunmapfln(FLN,(p),(n))

extern char *getoserr(ub4 *perrno);

extern int osopen(const char *name);
extern int osopenseq(const char *name,int *pfd);
extern int osappend(const char *name);

extern int osread(int fd,char *buf,ub4 len,ub4 *nread);
extern int oswrite(int fd, const char *buf,ub4 len);
extern int osread8(int fd,char *buf,ub8 len,unsigned long *nread);
extern int oswrite8(int fd, const char *buf,ub8 len,unsigned long *nwrit);

extern int oscreate(const char *name);
extern int osclose(int fd);

extern int osfdinfo(struct osstat *mf,int fd);
extern ub8 osfiltim(cchar *nam);

extern int osremove(const char *name);
extern int osmkdir(const char *dir);
extern int osexists(const char *name);
extern int osrename(const char *old,const char *new);

extern int osdup2(int oldfd,int newfd);
extern int osrewind(int fd);

extern Noret void doexit(int code);

extern void *osmmapfln(ub4 fln,ub8 nel,ub8 elsiz);
extern void *osmmapfd(ub8 len,int fd);
extern void *osmremapfln(ub4 fln,void *p,ub8 elsiz,ub4 oldel,ub4 newel);
extern int osmunmapfln(ub4 fln,void *p,ub8 len);

// extern int osmemrdonly(void *p,size_t len);
// extern int osmlock(void);
// extern int osmunlock(void);

extern void setsigs(void);
extern int oslimits(void);

extern ub4 osmeminfo(void);

extern int inios(void);
extern void exios(bool show);
