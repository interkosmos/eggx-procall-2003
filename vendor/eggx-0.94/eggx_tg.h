/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2020-09-19 12:00:00 cyamauch> */

/*
  EGGX / ProCALL  version 0.94
                     eggx_tg.h
 */

#ifndef _EGGX_TG_H
#define _EGGX_TG_H 1

#include <eggx_base.h>

#ifdef __cplusplus
#define __EGGX_TG_SZNL (sizeof(*((char *)0)))
#else
#define __EGGX_TG_SZNL (sizeof(*((void *)0)))
#endif

#define __EGGX_TG_PARGTP(V1,V2)						      \
    __typeof__( (sizeof(*(V1)) != __EGGX_TG_SZNL) ? *(V1) : ((sizeof(*(V2)) != __EGGX_TG_SZNL) ? *(V2) : *(V1)) )

#define __EGGX_TG_PARGSZ(V1,V2)						      \
    ( (sizeof(*(V1)) != __EGGX_TG_SZNL) ? sizeof(*(V1)) : ((sizeof(*(V2)) != __EGGX_TG_SZNL) ? sizeof(*(V2)) : sizeof(*(V1))) )

#define __EGGX_TG_INTPx2_REALPx2_RETURNS_INT(Vl1, Vl2, Vl3, Vl4, Fct)         \
    (__extension__ ({ int (*__eggx_tgres)(int *,int *,			      \
					  __EGGX_TG_PARGTP(Vl3,Vl4) *,        \
					  __EGGX_TG_PARGTP(Vl3,Vl4) *);       \
		      if ( __EGGX_TG_PARGSZ(Vl3,Vl4) == sizeof(float) )	      \
			__eggx_tgres = (__typeof__(__eggx_tgres))(&Fct##f);   \
		      else						      \
			__eggx_tgres = (__typeof__(__eggx_tgres))(&Fct);      \
		      (*__eggx_tgres)(Vl1, Vl2, Vl3, Vl4); }))

#define __EGGX_TG_INT_CREALPx2_INT_RETURNS_VOID(Vl1, Vl2, Vl3, Vl4, Fct)      \
    (__extension__ ({ void (*__eggx_tgres)(int,				      \
					   const __EGGX_TG_PARGTP(Vl2,Vl3) *, \
					   const __EGGX_TG_PARGTP(Vl2,Vl3) *, \
					   int);			      \
		      if ( __EGGX_TG_PARGSZ(Vl2,Vl3) == sizeof(float) )	      \
			__eggx_tgres = (__typeof__(__eggx_tgres))(&Fct##f);   \
		      else						      \
			__eggx_tgres = (__typeof__(__eggx_tgres))(&Fct);      \
		      (*__eggx_tgres)(Vl1, Vl2, Vl3, Vl4); }))

#define __EGGX_TG_INT_CREALPx2_INTx2_RETURNS_VOID(Vl1, Vl2, Vl3, Vl4, Vl5, Fct) \
    (__extension__ ({ void (*__eggx_tgres)(int,				      \
					   const __EGGX_TG_PARGTP(Vl2,Vl3) *, \
					   const __EGGX_TG_PARGTP(Vl2,Vl3) *, \
					   int, int);			      \
		      if ( __EGGX_TG_PARGSZ(Vl2,Vl3) == sizeof(float) )	      \
			__eggx_tgres = (__typeof__(__eggx_tgres))(&Fct##f);   \
		      else						      \
			__eggx_tgres = (__typeof__(__eggx_tgres))(&Fct);      \
		      (*__eggx_tgres)(Vl1, Vl2, Vl3, Vl4, Vl5); }))

#define __EGGX_TG_INT_CREALPx2_INTx3_RETURNS_VOID(Vl1, Vl2, Vl3, Vl4, Vl5, Vl6, Fct) \
    (__extension__ ({ void (*__eggx_tgres)(int,				      \
					   const __EGGX_TG_PARGTP(Vl2,Vl3) *, \
					   const __EGGX_TG_PARGTP(Vl2,Vl3) *, \
					   int, int);			      \
		      if ( __EGGX_TG_PARGSZ(Vl2,Vl3) == sizeof(float) )	      \
			__eggx_tgres = (__typeof__(__eggx_tgres))(&Fct##f);   \
		      else						      \
			__eggx_tgres = (__typeof__(__eggx_tgres))(&Fct);      \
		      (*__eggx_tgres)(Vl1, Vl2, Vl3, Vl4, Vl5, Vl6); }))

#define eggx_drawsyms(v1,v2,v3,v4,v5,v6) __EGGX_TG_INT_CREALPx2_INTx3_RETURNS_VOID (v1,v2,v3,v4,v5,v6, eggx_drawsyms)
#define eggx_drawpts(v1,v2,v3,v4) __EGGX_TG_INT_CREALPx2_INT_RETURNS_VOID (v1,v2,v3,v4, eggx_drawpts)
#define eggx_drawlines(v1,v2,v3,v4) __EGGX_TG_INT_CREALPx2_INT_RETURNS_VOID (v1,v2,v3,v4, eggx_drawlines)
#define eggx_drawpoly(v1,v2,v3,v4) __EGGX_TG_INT_CREALPx2_INT_RETURNS_VOID (v1,v2,v3,v4, eggx_drawpoly)
#define eggx_fillpoly(v1,v2,v3,v4,v5) __EGGX_TG_INT_CREALPx2_INTx2_RETURNS_VOID (v1,v2,v3,v4,v5, eggx_fillpoly)
#define eggx_ggetevent(v1,v2,v3,v4) __EGGX_TG_INTPx2_REALPx2_RETURNS_INT(v1,v2,v3,v4, eggx_ggetevent)
#define eggx_ggetxpress(v1,v2,v3,v4) __EGGX_TG_INTPx2_REALPx2_RETURNS_INT(v1,v2,v3,v4, eggx_ggetxpress)

#endif	/* _EGGX_TG_H */
