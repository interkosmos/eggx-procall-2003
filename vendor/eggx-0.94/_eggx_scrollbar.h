/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-02-19 21:19:35 cyamauch> */

/*
  EGGX �Ѥ˺�ꤢ������Motif �� NEXTSTEP �Υϥ��֥�åɤߤ�����
  ��������С�
 */

#ifndef __EGGX_SCROLLBAR_H
#define __EGGX_SCROLLBAR_H 1

#include <X11/Xlib.h>

/* ���� private �ʥ��Ф��ȻפäƤ������� */
typedef struct _eggx_scrollbar {
    Bool horizontal;			/* �������ΥС��ʤ� True */
    Display* display;			/* ����ϳ�������Ϳ������ */
    Window window;			/* ����ϳ�������Ϳ������ */
    GC gc;				/* ����ϳ�������Ϳ������ */
    unsigned long background_pixel;	/* ���顼���� */
    unsigned long shadow_pixel;
    unsigned long trough_pixel;
    unsigned long highlight_pixel;
    /* data */
    int data_length;			/* �ǡ�������Ĺ */
    int data_cliplength;		/* ����åפ���Ƥ�����ʬ��Ĺ�� */
    int data_cliporigin;		/* ����åפ����٤� */
    /* ɽ���� */
    int shadow_thickness;		/* �����礭��(�ԥ�����ñ��) */
    int unit_length;			/* ���������å����Τβ��� */
    int all_length;			/* ���������å����Τ���Ĺ */
    int widget_width;			/* �ܥ��������� */
    int button1up_pos;			/* �ʲ������줾������ʤΰ��֤�Ĺ�� */
    int button1up_length;
    int button1down_pos;
    int button1down_length;
    int bararea_pos;
    int bararea_length;
    int bar_pos;
    int bar_length;
    int button2up_pos;
    int button2up_length;
    int button2down_pos;
    int button2down_length;
    /* �ܥ���� on/off �α�/�ϥ��饤��ɽ���ѹ��ѤΥե饰 */
    int upbutton_on;
    int downbutton_on;
    /* �⡼����󥤥٥���� */
    int prev_xbutton_pos;
    Bool no_drag;
    /* �����ޥ��� */
    unsigned int key_mask;
    /* �������٥�Ȥμ�����(�⤷�����) */
    Window keyev_window;
} eggx_scrollbar;

extern int eggx_scrollbar_init( eggx_scrollbar *ret,
				Bool horizontal, Display *dis, Window win, GC gc,
				unsigned long bg_pix, unsigned long shadow_pix,
				unsigned long trough_pix, unsigned long high_pix,
				int data_len, int data_cliplen, 
				int shadow_thickness, unsigned int keymask,
				Window keyev_window );
extern int eggx_scrollbar_draw_upbutton( const eggx_scrollbar *bar );
extern int eggx_scrollbar_draw_downbutton( const eggx_scrollbar *bar );
extern int eggx_scrollbar_draw( eggx_scrollbar *bar );
extern int eggx_scrollbar_update_data_params( eggx_scrollbar *bar,
		   int data_length, int data_cliplength, int data_cliporigin );
extern int eggx_scrollbar_update_key_mask( eggx_scrollbar *bar, int keymask );
extern int eggx_scrollbar_get_cliporigin( const eggx_scrollbar *bar );
extern int eggx_scrollbar_handle_event( eggx_scrollbar *bar, const XEvent *ev );

#endif	/* __EGGX_SCROLLBAR_H */
