/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-02-19 21:19:45 cyamauch> */

#ifndef _EGGX_INTERNAL_DEFS_H
#define _EGGX_INTERNAL_DEFS_H 1

#define F_DIRECT (1<<0)

struct ds9_color {
    int flags ;
    struct {
	unsigned char r ;
	unsigned char g ;
	unsigned char b ;
    } c[613] ;
} ;

struct idl_color {
    int flags ;
    struct { 
	unsigned char r ;
	unsigned char g ;
	unsigned char b ;
    } c[256] ;
} ;

/* for FORTRAN */
typedef float real ;
typedef int integer ;

#endif
