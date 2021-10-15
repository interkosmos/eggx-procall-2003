/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-02-15 21:45:10 cyamauch> */

/*
  EGGX 用に作りあげた，Motif と NEXTSTEP のハイブリッドみたいな
  スクロールバー
 */

#include "_eggx_scrollbar.h"
#include <X11/Xutil.h>
#include <X11/keysym.h>
/* #include <stdio.h> */

/* 初期化 */
int eggx_scrollbar_init( eggx_scrollbar *ret, 
			 Bool horizontal, Display *dis, Window win, GC gc,
			 unsigned long bg_pix, unsigned long shadow_pix,
			 unsigned long trough_pix, unsigned long high_pix,
			 int data_len, int data_cliplen, 
			 int shadow_thickness, unsigned int keymask,
			 Window keyev_window )
{
    int status = -1;
    XWindowAttributes xvi;

    if ( ret == NULL ) goto quit;

    ret->horizontal = horizontal;
    ret->display = dis;
    ret->window = win;
    ret->gc = gc;
    ret->background_pixel = bg_pix;
    ret->shadow_pixel = shadow_pix;
    ret->trough_pixel = trough_pix;
    ret->highlight_pixel = high_pix;
    ret->data_length = data_len;
    ret->data_cliplength = data_cliplen;
    ret->data_cliporigin = (data_len - data_cliplen) / 2;
    ret->shadow_thickness = shadow_thickness;
    ret->key_mask = keymask;
    ret->keyev_window = keyev_window;

    //fprintf(stderr,"data_length = %d\n",ret->data_length);
    //fprintf(stderr,"data_cliplength = %d\n",ret->data_cliplength);
    //fprintf(stderr,"data_cliporigin = %d\n",ret->data_cliporigin);

    XGetWindowAttributes( ret->display, ret->window, &xvi );

    if ( ret->horizontal == True ) {	/* 横バー */
	ret->unit_length = xvi.height;
	ret->all_length = xvi.width;
    }
    else {				/* 縦バー */
	ret->unit_length = xvi.width;
	ret->all_length = xvi.height;
    }

    XSelectInput( ret->display, ret->window,
		  ExposureMask | StructureNotifyMask |
		  KeyPressMask |
		  ButtonPressMask | ButtonReleaseMask | PointerMotionMask );

    ret->upbutton_on = 0;
    ret->downbutton_on = 0;
    ret->prev_xbutton_pos = -1;
    ret->no_drag = False;

    status = 0;
 quit:
    return status;
}

static int x_fill_rect( const eggx_scrollbar *bar,
			int x, int y, unsigned int width, unsigned int height )
{
    if ( bar->horizontal ) 
	return XFillRectangle(bar->display, bar->window, bar->gc,
			      y, x, height, width);
    else
	return XFillRectangle(bar->display, bar->window, bar->gc,
			      x, y, width, height);
}

static int get_actual_shadow( const eggx_scrollbar *bar )
{
    int shadow_tk = bar->shadow_thickness;
    while ( bar->unit_length <= 2 * shadow_tk ||
	    bar->all_length <= 2 * shadow_tk ) {
	shadow_tk--;
	if ( shadow_tk == 0 ) break;
    }
    return shadow_tk;
}

int eggx_scrollbar_draw_upbutton( const eggx_scrollbar *bar )
{
    int ret = -1;
    int shadow_tk;
    int x0, w0, i;
    unsigned long pix0, pix1;

    if ( bar == NULL ) goto quit;

    if ( bar->upbutton_on ) {
	pix0 = bar->highlight_pixel;
	pix1 = bar->shadow_pixel;
    } else {
	pix1 = bar->highlight_pixel;
	pix0 = bar->shadow_pixel;
    }

    shadow_tk = get_actual_shadow(bar);
    x0 = 0 + shadow_tk;
    w0 = bar->unit_length - 2*shadow_tk;

    XSetForeground( bar->display, bar->gc, pix0 );
#define DRAW(_pos,_length) if ( i*2 < _length ) { \
	x_fill_rect( bar, x0 + w0 - 1 - i, _pos + i, 1, _length - i*2 ); \
	x_fill_rect( bar, x0 + i, _pos + _length - 1 - i, w0 - 1 - i*2, 1 ); \
}
    for ( i=0 ; i < shadow_tk ; i++ ) {
	/* ボタン1upの陰 */
	DRAW(bar->button1up_pos, bar->button1up_length);
	/* ボタン2upの陰 */
	DRAW(bar->button2up_pos, bar->button2up_length);
    }
#undef DRAW

    XSetForeground( bar->display, bar->gc, pix1 );
#define DRAW(_pos,_length) if ( i*2 < _length ) { \
	x_fill_rect( bar, x0 + i, _pos + i, w0 - i*2, 1 ); \
	x_fill_rect( bar, x0 + i, _pos + i, 1, _length - i*2 ); \
}
    for ( i=0 ; i < shadow_tk ; i++ ) {
	/* ボタン1upのハイライト */
	DRAW(bar->button1up_pos, bar->button1up_length);
	/* ボタン2upのハイライト */
	DRAW(bar->button2up_pos, bar->button2up_length);
    }
#undef DRAW

    ret = 0;
 quit:
    return ret;
}

int eggx_scrollbar_draw_downbutton( const eggx_scrollbar *bar )
{
    int ret = -1;
    int shadow_tk;
    int x0, w0, i;
    unsigned long pix0, pix1;

    if ( bar == NULL ) goto quit;

    if ( bar->downbutton_on ) {
	pix0 = bar->highlight_pixel;
	pix1 = bar->shadow_pixel;
    } else {
	pix1 = bar->highlight_pixel;
	pix0 = bar->shadow_pixel;
    }

    shadow_tk = get_actual_shadow(bar);
    x0 = 0 + shadow_tk;
    w0 = bar->unit_length - 2*shadow_tk;

    XSetForeground( bar->display, bar->gc, pix0 );
#define DRAW(_pos,_length) if ( i*2 < _length ) { \
	x_fill_rect( bar, x0 + w0 - 1 - i, _pos + i, 1, _length - i*2 ); \
	x_fill_rect( bar, x0 + i, _pos + _length - 1 - i, w0 - 1 - i*2, 1 ); \
}
    for ( i=0 ; i < shadow_tk ; i++ ) {
	/* ボタン1downの陰 */
	DRAW(bar->button1down_pos, bar->button1down_length);
	/* ボタン2downの陰 */
	DRAW(bar->button2down_pos, bar->button2down_length);
    }
#undef DRAW

    XSetForeground( bar->display, bar->gc, pix1 );
#define DRAW(_pos,_length) if ( i*2 < _length ) { \
	x_fill_rect( bar, x0 + i, _pos + i, w0 - i*2, 1 ); \
	x_fill_rect( bar, x0 + i, _pos + i, 1, _length - i*2 ); \
}
    for ( i=0 ; i < shadow_tk ; i++ ) {
	/* ボタン1downのハイライト */
	DRAW(bar->button1down_pos, bar->button1down_length);
	/* ボタン2downのハイライト */
	DRAW(bar->button2down_pos, bar->button2down_length);
    }
#undef DRAW

    ret = 0;
 quit:
    return ret;
}

int eggx_scrollbar_draw( eggx_scrollbar *bar )
{
    int ret = -1;
    int unit_len;
    int all_len;
    int shadow_tk;
    int i, min_l;

    if ( bar == NULL ) goto quit;

    unit_len = bar->unit_length;
    all_len = bar->all_length;

    /* 各部品の長さの計算 */
    shadow_tk = get_actual_shadow(bar);

    bar->widget_width = unit_len - 2 * shadow_tk;
    bar->button1up_length = bar->widget_width;
    bar->button1down_length = bar->widget_width;
    bar->button2up_length = bar->widget_width;
    bar->button2down_length = bar->widget_width;
 
    /* 表示エリアが十分長いときは，4つボタンを表示する */
    min_l = 2 * shadow_tk +
	bar->button1up_length + bar->button1down_length +
	bar->button2up_length + bar->button2down_length;
    /* 表示エリアが狭いときは，2つボタンを表示する */
    if ( all_len <= min_l + 2 * bar->widget_width ) {
	bar->button1down_length = 0;
	bar->button2up_length = 0;
	min_l = 2 * shadow_tk +
	    bar->button1up_length + bar->button2down_length;
    }
    /* 表示エリアが狭すぎるときは，ボタンを表示しない */
    if ( all_len <= min_l + 2 * bar->widget_width ) {
	bar->button1up_length = 0;
	bar->button2down_length = 0;
	min_l = 2 * shadow_tk;
    }
    if ( all_len <= min_l ) {
	bar->bararea_length = 0;
    }
    else {
	bar->bararea_length = all_len - min_l;
    }

    if ( bar->data_cliplength < bar->data_length ) {
	bar->bar_length = bar->bararea_length * 
	    ((double)(bar->data_cliplength) / bar->data_length);
    } else {
	bar->bar_length = bar->bararea_length;
    }
    if ( bar->bar_length < bar->widget_width ) {
	if ( 1.5 * bar->widget_width <= bar->bararea_length ) {
	    bar->bar_length = bar->widget_width;
	}
    }
    if ( bar->bar_length < 4 + 2 * shadow_tk ) {
	if ( 8 + 2 * shadow_tk < bar->bararea_length ) {
	    bar->bar_length = 4 + 2 * shadow_tk;
	}
    }
    //fprintf(stderr,"bar_length=%d\n",bar->bar_length);

    /* 各部品のポジションの計算 */
    bar->button1up_pos = 0 + shadow_tk;
    bar->button1down_pos = bar->button1up_pos + bar->button1up_length;
    bar->bararea_pos = bar->button1down_pos + bar->button1down_length;
    bar->bar_pos = bar->button1down_pos + bar->button1down_length
  + (bar->bararea_length - bar->bar_length /* 動かせる長さ */)
  * (bar->data_cliporigin / (double)(bar->data_length - bar->data_cliplength));
    bar->button2up_pos = bar->bararea_pos + bar->bararea_length;
    bar->button2down_pos = bar->button2up_pos + bar->button2up_length;

    //fprintf(stderr,"bararea_length: %d\n",bar->bararea_length);
    //fprintf(stderr,"bar_length: %d\n",bar->bar_length);
    //fprintf(stderr,"bar_pos: %d\n",bar->bar_pos);

    /* 描画 */ 
    {
	int x0, w0;
	x0 = 0 + shadow_tk;
	w0 = unit_len - 2*shadow_tk;

	XSetForeground( bar->display, bar->gc, bar->background_pixel );
#define DRAW(_pos,_length) if ( shadow_tk*2 < _length ) { \
	x_fill_rect( bar, x0 + shadow_tk, _pos + shadow_tk, \
		     w0 - shadow_tk*2, _length - shadow_tk*2 ); \
}
	/* ボタン1upの表面 */
	DRAW(bar->button1up_pos, bar->button1up_length);
	/* ボタン1downの表面 */
	DRAW(bar->button1down_pos, bar->button1down_length);
	/* バーの表面 */
	DRAW(bar->bar_pos, bar->bar_length);
	/* ボタン2upの表面 */
	DRAW(bar->button2up_pos, bar->button2up_length);
	/* ボタン2downの表面 */
	DRAW(bar->button2down_pos, bar->button2down_length);
#undef DRAW

	XSetForeground( bar->display, bar->gc, bar->trough_pixel );
#define DRAW(_pos,_length) if ( 0 < _length ) { \
	x_fill_rect( bar, x0, _pos, w0, _length ); \
}
	DRAW(bar->bararea_pos, bar->bar_pos - bar->bararea_pos);
	DRAW(bar->bar_pos + bar->bar_length,
	     bar->button2up_pos - bar->bar_pos - bar->bar_length);
#undef DRAW

	XSetForeground( bar->display, bar->gc, bar->shadow_pixel );
#define DRAW(_pos,_length) if ( i*2 < _length ) { \
	x_fill_rect( bar, x0 + w0 - 1 - i, _pos + i, 1, _length - i*2 ); \
	x_fill_rect( bar, x0 + i, _pos + _length - 1 - i, w0 - 1 - i*2, 1 ); \
}
	for ( i=0 ; i < shadow_tk ; i++ ) {
	    /* 枠の陰 */
	    if ( i*2 < all_len ) {
		x_fill_rect( bar, 0 + i, 0 + i, unit_len - i*2, 1 );
		x_fill_rect( bar, 0 + i, 0 + i, 1, all_len - i*2 );
	    }
	    /* ボタン1upの陰 */
	    if ( bar->upbutton_on == 0 ) {
		DRAW(bar->button1up_pos, bar->button1up_length);
	    }
	    /* ボタン1downの陰 */
	    if ( bar->downbutton_on == 0 ) {
		DRAW(bar->button1down_pos, bar->button1down_length);
	    }
	    /* バーの陰 */
	    DRAW(bar->bar_pos, bar->bar_length);
	    /* ボタン2upの陰 */
	    if ( bar->upbutton_on == 0 ) {
		DRAW(bar->button2up_pos, bar->button2up_length);
	    }
	    /* ボタン2downの陰 */
	    if ( bar->downbutton_on == 0 ) {
		DRAW(bar->button2down_pos, bar->button2down_length);
	    }
#undef DRAW
	}
	/* ▲ */
#define DRAW(_pos,_length) if ( shadow_tk*2 < _length ) { \
	int wd = w0 - shadow_tk*2, ht = _length - shadow_tk*2; \
	int xcen = x0 + shadow_tk + wd/2, ycen = _pos + shadow_tk + ht/2; \
	int j, ht3 = ht * 0.8; \
	for ( j=0 ; 2*j < ht3 ; j++ ) { \
	    x_fill_rect( bar, xcen + j, ycen - ht3/2 + 2*j, 1, ht3 - 2*j ); \
	    x_fill_rect( bar, xcen - j - (wd+1)%2, ycen - ht3/2 + 2*j, \
			      1, ht3 - 2*j ); \
	} \
}
	DRAW(bar->button1up_pos, bar->button1up_length);
	DRAW(bar->button2up_pos, bar->button2up_length);
#undef DRAW
	/* ▼ */
#define DRAW(_pos,_length) if ( shadow_tk*2 < _length ) { \
	int wd = w0 - shadow_tk*2, ht = _length - shadow_tk*2; \
	int xcen = x0 + shadow_tk + wd/2, ycen = _pos + shadow_tk + (ht+1)/2; \
	int j, ht3 = ht * 0.8; \
	for ( j=0 ; 2*j < ht3 ; j++ ) { \
	    x_fill_rect( bar, xcen + j, ycen - ht3/2, 1, ht3 - 2*j ); \
	    x_fill_rect( bar, xcen - j - (wd+1)%2, ycen - ht3/2, \
			      1, ht3 - 2*j ); \
	} \
}
	DRAW(bar->button1down_pos, bar->button1down_length);
	DRAW(bar->button2down_pos, bar->button2down_length);
#undef DRAW

	XSetForeground( bar->display, bar->gc, bar->highlight_pixel );
#define DRAW(_pos,_length) if ( i*2 < _length ) { \
	x_fill_rect( bar, x0 + i, _pos + i, w0 - i*2, 1 ); \
	x_fill_rect( bar, x0 + i, _pos + i, 1, _length - i*2 ); \
}
	for ( i=0 ; i < shadow_tk ; i++ ) {
	    /* 枠のハイライト */
	    if ( (1 + i*2) < all_len ) {
		x_fill_rect( bar, unit_len - 1 - i, 0 + i,
			     1, all_len - (1 + i*2) );
		x_fill_rect( bar, 0 + 1 + i, all_len - 1 - i,
			     unit_len - 1 - i*2, 1 );
	    }
	    /* ボタン1upのハイライト */
	    if ( bar->upbutton_on == 0 ) {
		DRAW(bar->button1up_pos, bar->button1up_length);
	    }
	    /* ボタン1downのハイライト */
	    if ( bar->downbutton_on == 0 ) {
		DRAW(bar->button1down_pos, bar->button1down_length);
	    }
	    /* バーのハイライト */
	    DRAW(bar->bar_pos, bar->bar_length);
	    /* ボタン2upのハイライト */
	    if ( bar->upbutton_on == 0 ) {
		DRAW(bar->button2up_pos, bar->button2up_length);
	    }
	    /* ボタン2downのハイライト */
	    if ( bar->downbutton_on == 0 ) {
		DRAW(bar->button2down_pos, bar->button2down_length);
	    }
#undef DRAW
	}

	if ( bar->upbutton_on ) {
	    eggx_scrollbar_draw_upbutton(bar);
	}
	if ( bar->downbutton_on ) {
	    eggx_scrollbar_draw_downbutton(bar);
	}
    }

    ret = 0;
 quit:
    return ret;
}

int eggx_scrollbar_update_data_params( eggx_scrollbar *bar,
		    int data_length, int data_cliplength, int data_cliporigin )
{
    int ret = -1;

    if ( bar == NULL ) goto quit;

    bar->data_length = data_length;
    bar->data_cliplength = data_cliplength;
    bar->data_cliporigin = data_cliporigin;

    if ( bar->data_cliplength < bar->data_length ) {
	if ( bar->data_cliporigin < 0 ) bar->data_cliporigin = 0;
	if ( bar->data_length - bar->data_cliplength < 
	     bar->data_cliporigin ) {
	    bar->data_cliporigin = bar->data_length - bar->data_cliplength;
	}
    }

    eggx_scrollbar_draw(bar);

    ret = 0;
 quit:
    return ret;
}

int eggx_scrollbar_update_key_mask( eggx_scrollbar *bar, int keymask )
{
    int ret = -1;

    if ( bar == NULL ) goto quit;

    bar->key_mask = keymask;

    ret = 0;
 quit:
    return ret;
}

int eggx_scrollbar_get_cliporigin( const eggx_scrollbar *bar )
{
    return bar->data_cliporigin;
}

int eggx_scrollbar_handle_event( eggx_scrollbar *bar, const XEvent *ev )
{
    int ret = -1;
    int org_cliporigin;
    int needs_flush = 0;

    int mov_medium = bar->data_cliplength * 0.10;	/* wheel */
    int mov_small = bar->data_cliplength * 0.04;	/* button */
    int mov_large = bar->data_cliplength;		/* trough */

    if ( bar == NULL ) goto quit;
    if ( ev == NULL ) goto quit;
    if ( bar->display != ev->xany.display ) {
	ret = 0;
	goto quit;
    }

    org_cliporigin = bar->data_cliporigin;

    if ( ev->type == Expose ) {
	Window ev_win = ev->xexpose.window;
	if ( ev_win == bar->window ) {
	    eggx_scrollbar_draw(bar);
	    needs_flush = 1;
	}
    }
    else if ( ev->type == ConfigureNotify ) {
	if ( ev->xconfigure.window == bar->window ) {
	    int new_width = ev->xconfigure.width;
	    int new_height = ev->xconfigure.height;
	    if ( bar->horizontal == True ) {	/* 横バー */
		bar->unit_length = new_height;
		bar->all_length = new_width;
	    }
	    else {				/* 縦バー */
		bar->unit_length = new_width;
		bar->all_length = new_height;
	    }
	    eggx_scrollbar_draw(bar);
	    needs_flush = 1;
	}
    }
    else if ( ev->type == KeyPress ) {
	Window ref_win;
	if ( bar->keyev_window != None ) ref_win = bar->keyev_window;
	else ref_win = bar->window;
	if ( ev->xkey.window == ref_win &&
	     (ev->xkey.state & bar->key_mask) == bar->key_mask ) {
	    XKeyEvent xkev = ev->xkey;
	    char string[8] ;
	    KeySym key;
	    XLookupString(&xkev,string,sizeof(string),&key,NULL) ;
	    if ( bar->horizontal == True ) {
		if ( key == XK_Right || key == XK_KP_Right ) {
		    bar->data_cliporigin += mov_small;
		} else if ( key == XK_Left || key == XK_KP_Left ) {
		    bar->data_cliporigin -= mov_small;
		}
	    }
	    else {
		if ( key == XK_Down || key == XK_KP_Down ) {
		    bar->data_cliporigin += mov_small;
		} else if ( key == XK_Up || key == XK_KP_Up ) {
		    bar->data_cliporigin -= mov_small;
		} else if ( key == XK_Page_Down || key == XK_KP_Page_Down ) {
		    bar->data_cliporigin += mov_large;
		} else if ( key == XK_Page_Up || key == XK_KP_Page_Up ) {
		    bar->data_cliporigin -= mov_large;
		} else if ( key == XK_Home || key == XK_KP_Home ) {
		    bar->data_cliporigin = 0;
		} else if ( key == XK_End || key == XK_KP_End ) {
		    bar->data_cliporigin = bar->data_length - bar->data_cliplength;
		}
	    }
	}
    }
    else if ( ev->type == ButtonPress ) {
	Bool old_no_drag = bar->no_drag;
	bar->prev_xbutton_pos = -1;
	bar->no_drag = False;
	if ( ev->xbutton.window == bar->window ) {
	    int button = ev->xbutton.button;
	    int pos;
	    if ( mov_small < 1 ) mov_small = 1;
	    if ( bar->horizontal == True ) pos = ev->xbutton.x;
	    else pos = ev->xbutton.y;
	    /* normal button */
	    if ( 1 <= button && button <= 3 ) {
		if ( (bar->button1up_pos <= pos && 
		      pos < bar->button1up_pos + bar->button1up_length) ||
		     (bar->button2up_pos <= pos && 
		      pos < bar->button2up_pos + bar->button2up_length) ) {
		    bar->upbutton_on = 1;
		    if ( button == 3 ) bar->data_cliporigin -= mov_medium;
		    else bar->data_cliporigin -= mov_small;
		}
		else if ( (bar->button1down_pos <= pos && 
		      pos < bar->button1down_pos + bar->button1down_length) ||
		      (bar->button2down_pos <= pos && 
		      pos < bar->button2down_pos + bar->button2down_length) ) {
		    bar->downbutton_on = 1;
		    if ( button == 3 ) bar->data_cliporigin += mov_medium;
		    else bar->data_cliporigin += mov_small;
		}
		else if ( bar->bararea_pos <= pos && pos < bar->bar_pos ) {
		    bar->data_cliporigin -= mov_large;
		}
		else if ( bar->bar_pos + bar->bar_length <= pos &&
			  pos < bar->bararea_pos + bar->bararea_length ) {
		    bar->data_cliporigin += mov_large;
		}
		else if ( bar->bar_pos <= pos && 
			  pos < bar->bar_pos + bar->bar_length ) {
		    bar->prev_xbutton_pos = pos;
		    if ( button == 3 ) {
			if ( old_no_drag == False ) bar->no_drag = True;
		    }
		    else if ( button == 2 ) {	/* スナップ */
			bar->data_cliporigin = ((double)((pos - bar->bararea_pos) - bar->bar_length / 2) / (bar->bararea_length - bar->bar_length)) * (double)(bar->data_length - bar->data_cliplength);
		    }
		}
	    }
	    /* wheel */
	    else if ( button == 4 ) bar->data_cliporigin -= mov_medium;
	    else if ( button == 5 ) bar->data_cliporigin += mov_medium;
	}
    } else if ( ev->type == ButtonRelease ) {
	if ( bar->upbutton_on ) {
	    bar->upbutton_on = 0;
	    eggx_scrollbar_draw_upbutton(bar);
	    needs_flush = 1;
	}
	if ( bar->downbutton_on ) {
	    bar->downbutton_on = 0;
	    eggx_scrollbar_draw_downbutton(bar);
	    needs_flush = 1;
	}
    } else if ( ev->type == MotionNotify ) {
	if ( bar->upbutton_on ) {
	    bar->upbutton_on = 0;
	    eggx_scrollbar_draw_upbutton(bar);
	    needs_flush = 1;
	}
	if ( bar->downbutton_on ) {
	    bar->downbutton_on = 0;
	    eggx_scrollbar_draw_downbutton(bar);
	    needs_flush = 1;
	}
	if ( ( ( ev->xmotion.window == bar->window &&
		 (ev->xmotion.state & (Button1Mask|Button2Mask)) != 0 ) ||
	       bar->no_drag == True ) &&
	     0 <= bar->prev_xbutton_pos ) {
	    int pos, diff;
	    if ( bar->horizontal == True ) pos = ev->xmotion.x;
	    else pos = ev->xmotion.y;
	    diff = pos - bar->prev_xbutton_pos;
	    bar->data_cliporigin +=
	      (bar->data_length - bar->data_cliplength) * 
	      ((double)diff / (double)(bar->bararea_length - bar->bar_length));
	    bar->prev_xbutton_pos = pos;
	}
    }

    /* 規定外のクリップposを修正 */
    if ( bar->data_cliplength < bar->data_length ) {
	if ( bar->data_cliporigin < 0 ) bar->data_cliporigin = 0;
	if ( bar->data_length - bar->data_cliplength < 
	     bar->data_cliporigin ) {
	    bar->data_cliporigin = bar->data_length - bar->data_cliplength;
	}
    }
    else {
	bar->data_cliporigin = org_cliporigin;
    }

    /* 更新フラグのセット */
    if ( bar->data_cliporigin != org_cliporigin ) {
	eggx_scrollbar_draw(bar);
	needs_flush = 1;
	ret = 1;	/* shows update */
    } else {
	ret = 0;
    }

    if ( needs_flush ) XFlush(bar->display);

 quit:
    return ret;
}
