/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2020-09-19 12:00:00 cyamauch> */

/*
  EGGX / ProCALL  version 0.94
                   eggx_base.h
*/

#ifndef _EGGX_BASE_H
#define _EGGX_BASE_H 1

#include <stddef.h>
#include <X11/X.h>

#ifdef __cplusplus
extern "C" {
#endif

/* サポートするウィンドゥ属性 (1<<0は使わない) */
#define SCROLLBAR_INTERFACE (1<<1)
#define DOCK_APPLICATION (1<<3)
#define OVERRIDE_REDIRECT (1<<4)
#define BOTTOM_LEFT_ORIGIN (1<<7)

#define FONTSET 0

#define ENABLE -1
#define DISABLE 0

/* */
#define PENDOWN 2
#define PENUP   3
#define PSET    1

/* 互換用 */
#define DOCKAPP DOCK_APPLICATION
#define OVERRIDE OVERRIDE_REDIRECT
#define BOTTOMLEFTORIGIN BOTTOM_LEFT_ORIGIN

extern void eggx_gsetnonflush( int flag ) ;
extern int eggx_ggetnonflush( void ) ;
extern void eggx_gflush( void ) ;
extern int eggx_ggetdisplayinfo( int *rt_depth,
				 int *rt_root_width, int *rt_root_height ) ;
extern int eggx_gopen( int xsize, int ysize ) ;
extern int eggx_winname( int wn, const char *argsformat, ... ) ;
extern void eggx_window( int wn, double xs, double ys, double xe, double ye ) ;
extern void eggx_coordinate( int wn, int xref, int yref,
			 double xs, double ys, double xscale, double yscale ) ;
extern void eggx_tclr( void );
extern void eggx_gclose( int wn ) ;
extern void eggx_gcloseall( void ) ;
extern void eggx_newrgbcolor( int wn, int r, int g, int b ) ;
extern void eggx_newhsvcolor( int win, int h, int s, int v ) ;
extern void eggx_newcolor( int wn, const char *argsformat, ... ) ;
extern void eggx_newpen( int wn, int n ) ;
extern void eggx_newlinewidth( int wn, int width ) ;
extern void eggx_newlinestyle( int wn, int style ) ;
extern void eggx_newgcfunction( int wn, int fnc ) ;
extern void eggx_gclr( int wn ) ;
extern void eggx_drawline( int wn,
			   double xg0, double yg0, double xg1, double yg1 ) ;
extern void eggx_line( int wn, double xg, double yg, int mode ) ;
extern void eggx_moveto( int wn, double xg, double yg ) ;
extern void eggx_lineto( int wn, double xg, double yg ) ;
extern void eggx_pset( int wn, double xg, double yg ) ;
extern int eggx_newfontset( int wn, const char *argsformat, ... ) ;
extern int eggx_gsetfontset( int wn, const char *argsformat, ... ) ;
extern int eggx_drawstr( int wn, double x, double y, int size, double theta,
			 const char *argsformat, ... ) ;
extern void eggx_drawsym( int wn, double xg, double yg, int size, int sym ) ;
extern void eggx_drawsyms( int wn, const double x[], const double y[], int n,
			   int size, int sym ) ;
extern void eggx_drawsymsf( int wn, const float x[], const float y[], int n,
			    int size, int sym ) ;
extern void eggx_drawarrow( int wn, double xs, double ys, double xt, double yt, 
			    double s, double w, int shape ) ;
extern void eggx_drawarc( int wn, double xcen, double ycen, double xrad, double yrad, 
			  double sang, double eang, int idir ) ;
extern void eggx_fillarc( int wn, double xcen, double ycen, double xrad, double yrad,
			  double sang, double eang, int idir ) ;
extern void eggx_drawcirc( int wn, double xcen, double ycen, double xrad, double yrad ) ;
extern void eggx_circle( int wn, double xcen, double ycen, double xrad, double yrad ) ;
extern void eggx_fillcirc( int wn, double xcen, double ycen, double xrad, double yrad ) ;
extern void eggx_drawrect( int wn, double x, double y, double w, double h ) ;
extern void eggx_fillrect( int wn, double x, double y, double w, double h ) ;
extern void eggx_drawpts( int wn, const double x[], const double y[], int n ) ;
extern void eggx_drawptsf( int wn, const float x[], const float y[], int n ) ;
extern void eggx_drawlines( int wn, const double x[], const double y[], int n ) ;
extern void eggx_drawlinesf( int wn, const float x[], const float y[], int n ) ;
extern void eggx_drawpoly( int wn, const double x[], const double y[], int n ) ;
extern void eggx_drawpolyf( int wn, const float x[], const float y[], int n ) ;
extern void eggx_fillpoly( int wn, const double x[], const double y[], int n, int shape ) ;
extern void eggx_fillpolyf( int wn, const float x[], const float y[], int n, int shape ) ;
extern void eggx_gresize( int wn, int xsize, int ysize );
extern void eggx_copylayer( int wn, int src, int dst ) ;
extern void eggx_layer( int wn, int sl, int wl ) ;
extern void eggx_gscroll( int wn, int x, int y, int clr ) ;
extern void eggx_gputarea( int wn, double x, double y,
	   int src_wn, int src_ly, double xs, double ys, double xe, double ye );
extern int eggx_putimg24( int wn, double x, double y,
			  int width, int height, unsigned char *buf ) ;
extern int eggx_putimg24m( int wn, double x, double y,
			   int width, int height, unsigned char *buf ) ;
extern int eggx_gputimage( int wn, double x, double y,
		    unsigned char *buf, int width, int height, int msk ) ;
extern unsigned char *eggx_ggetimage( int wn, int ly, 
				    double xs, double ys, double xe, double ye,
				    int *r_width, int *r_height ) ;
extern void eggx_gsetnonblock( int flag ) ;
extern void eggx_gsetscrollbarkeymask( int wn, unsigned int key_mask );
extern int eggx_ggetevent( int *type_r, int *code_r, double *x_r, double *y_r ) ;
extern int eggx_ggeteventf( int *type_r, int *code_r, float *x_r, float *y_r ) ;
extern int eggx_ggetxpress( int *type_r, int *code_r, double *x_r, double *y_r ) ;
extern int eggx_ggetxpressf( int *type_r, int *code_r, float *x_r, float *y_r ) ;
extern int eggx_ggetch( void ) ;
extern unsigned char *eggx_readimage( const char *filter, const char *filename,
				      int *r_width, int *r_height, int *r_msk );
extern int eggx_writeimage( const unsigned char *buf, int width, int height, int msk,
		  const char *filter, int depth, const char *argsformat, ... ) ;
extern int eggx_gsaveimage( int wn, int ly, double xs,double ys, double xe,double ye,
		  const char *filter, int depth, const char *argsformat, ... ) ;
extern int eggx_saveimg( int wn, int ly, double xs, double ys, double xe, double ye,
		  const char *filter, int depth, const char *argsformat, ... ) ;
extern void eggx_gsetborder( int wn, int width, const char *argsformat, ... ) ;
extern void eggx_gsetbgcolor( int wn, const char *argsformat, ... ) ;

extern void eggx_gsetinitialattributes( int values, int att_msk ) ;
extern int eggx_ggetinitialattributes( void ) ;
extern void eggx_gsetinitialborder( int width, const char *argsformat, ... ) ;
extern void eggx_gsetinitialbgcolor( const char *argsformat, ... ) ;
extern void eggx_gsetinitialgeometry( const char *argsformat, ... ) ;
extern void eggx_gsetinitialparsegeometry( const char *argsformat, ... ) ;
extern void eggx_gsetinitialwinname( const char *storename, const char *iconname,
				  const char *resname, const char *classname ) ;
extern void eggx_msleep( unsigned long msec ) ;

#ifdef __cplusplus
}
#endif

#endif	/* _EGGX_BASE_H */
