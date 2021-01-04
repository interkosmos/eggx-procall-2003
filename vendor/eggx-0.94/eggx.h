/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2020-09-19 12:00:00 cyamauch> */

/*
  EGGX / ProCALL  version 0.94
                        eggx.h
 */

#ifndef _EGGX_H
#define _EGGX_H 1

#include <eggxlib.h>

#define gsetnonflush eggx_gsetnonflush
#define ggetnonflush eggx_ggetnonflush
#define gflush eggx_gflush
#define ggetdisplayinfo eggx_ggetdisplayinfo
#define gopen eggx_gopen
#define winname eggx_winname
#define window eggx_window
#define coordinate eggx_coordinate
#define tclr eggx_tclr
#define gclose eggx_gclose
#define gcloseall eggx_gcloseall
#define newrgbcolor eggx_newrgbcolor
#define newhsvcolor eggx_newhsvcolor
#define newcolor eggx_newcolor
#define newpen eggx_newpen
#define newlinewidth eggx_newlinewidth
#define newlinestyle eggx_newlinestyle
#define newgcfunction eggx_newgcfunction
#define gclr eggx_gclr
#define drawline eggx_drawline
#define line eggx_line
#define moveto eggx_moveto
#define lineto eggx_lineto
#define pset eggx_pset
#define newfontset eggx_newfontset
#define gsetfontset eggx_gsetfontset
#define drawstr eggx_drawstr
#define drawsym eggx_drawsym
#define drawsyms eggx_drawsyms
#define drawarrow eggx_drawarrow
#define drawarc eggx_drawarc
#define fillarc eggx_fillarc
#define drawcirc eggx_drawcirc
#define circle eggx_circle
#define fillcirc eggx_fillcirc
#define drawrect eggx_drawrect
#define fillrect eggx_fillrect
#define drawpts eggx_drawpts
#define drawlines eggx_drawlines
#define drawpoly eggx_drawpoly
#define fillpoly eggx_fillpoly
#define gresize eggx_gresize
#define copylayer eggx_copylayer
#define layer eggx_layer
#define gscroll eggx_gscroll
#define gputarea eggx_gputarea
#define putimg24 eggx_putimg24
#define putimg24m eggx_putimg24m
#define gputimage eggx_gputimage
#define ggetimage eggx_ggetimage
#define gsetnonblock eggx_gsetnonblock
#define gsetscrollbarkeymask eggx_gsetscrollbarkeymask
#define ggetevent eggx_ggetevent
#define ggetxpress eggx_ggetxpress
#define ggetch eggx_ggetch
#define readimage eggx_readimage
#define writeimage eggx_writeimage
#define saveimg eggx_saveimg
#define gsaveimage eggx_gsaveimage
#define gsetborder eggx_gsetborder
#define gsetbgcolor eggx_gsetbgcolor
#define gsetinitialattributes eggx_gsetinitialattributes
#define ggetinitialattributes eggx_ggetinitialattributes
#define gsetinitialborder eggx_gsetinitialborder
#define gsetinitialbgcolor eggx_gsetinitialbgcolor
#define gsetinitialgeometry eggx_gsetinitialgeometry
#define gsetinitialparsegeometry eggx_gsetinitialparsegeometry
#define gsetinitialwinname eggx_gsetinitialwinname
#define msleep eggx_msleep

#define color_prms eggx_color_prms
#define generatecolor eggx_generatecolor
#define makecolor eggx_makecolor

#define inkeydollar eggx_ggetch

#endif	/* _EGGX_H */
