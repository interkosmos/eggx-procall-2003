/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-02-19 21:19:35 cyamauch> */

/*
  EGGX 用に作りあげた，Motif と NEXTSTEP のハイブリッドみたいな
  スクロールバー
 */

#ifndef __EGGX_SCROLLBAR_H
#define __EGGX_SCROLLBAR_H 1

#include <X11/Xlib.h>

/* 全部 private なメンバだと思ってください */
typedef struct _eggx_scrollbar {
    Bool horizontal;			/* 横方向のバーなら True */
    Display* display;			/* これは外部から与えられる */
    Window window;			/* これは外部から与えられる */
    GC gc;				/* これは外部から与えられる */
    unsigned long background_pixel;	/* カラー情報 */
    unsigned long shadow_pixel;
    unsigned long trough_pixel;
    unsigned long highlight_pixel;
    /* data */
    int data_length;			/* データの全長 */
    int data_cliplength;		/* クリップされている部分の長さ */
    int data_cliporigin;		/* クリップされるべき */
    /* 表示用 */
    int shadow_thickness;		/* 陰の大きさ(ピクセル単位) */
    int unit_length;			/* ウィジェット全体の横幅 */
    int all_length;			/* ウィジェット全体の全長 */
    int widget_width;			/* ボタン等の幅 */
    int button1up_pos;			/* 以下，それぞれの部品の位置と長さ */
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
    /* ボタンの on/off の陰/ハイライト表示変更用のフラグ */
    int upbutton_on;
    int downbutton_on;
    /* モーションイベント用 */
    int prev_xbutton_pos;
    Bool no_drag;
    /* キーマスク */
    unsigned int key_mask;
    /* キーイベントの取得元(もしあれば) */
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
