// math.h

#if 0
#ifndef M_PI
  #define M_PI 3.141592655
#endif

struct range {
  ub4 lo,hi,hilo;
  ub4 lopos,hipos;
};

extern int minmax(ub4 *x, ub4 n, struct range *rp);
extern int minmax2(ub2 *x, ub4 n, struct range *rp);
extern int mkhist(ub4 callee,ub4 *data, ub4 n,struct range *rp, ub4 ivcnt,ub4 *bins, const char *desc,ub4 lvl);
extern int mkhist2(ub2 *data, ub4 n,struct range *rp, ub4 ivcnt,ub4 *bins, const char *desc,ub4 lvl);
#endif

extern ub4 rnd(ub4 range);
extern double frnd(ub4 range);

extern int inimath(void);
