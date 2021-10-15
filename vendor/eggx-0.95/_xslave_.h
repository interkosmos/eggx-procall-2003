/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-03-06 19:38:53 cyamauch> */

#ifndef __XSLAVE_H
#define __XSLAVE_H 1

/* EGGX では，minor_code にコマンドを載せ */
/* GraphicsExpose イベントで親子通信する */

/* major_code -> windows index of eggx */

/* minor_code */
#define MCODE_DUMMY            0	/* 子から親への「完了」の報告で使用 */
#define MCODE_NEEDS_REPLY      1	/* 親へ返信をよこせ */

#define MCODE_ENABLE           2	/* 以下，親から子に送信する時に使用 */
#define MCODE_DISABLE          3

#define MCODE_PWIN_ID          4
#define MCODE_CLIPWIN_ID       5
#define MCODE_HSBARWIN_ID      6
#define MCODE_VSBARWIN_ID      7
#define MCODE_WIN_ID           8
#define MCODE_ICONWIN_ID       9
#define MCODE_BL_ORIGIN       10
#define MCODE_PIXMAP_ID       11
#define MCODE_KEYMASK         12

#define MCODE_HANDLE_BTN_EV   13	/* ボタンイベントの扱いを開始 */
#define MCODE_NOHANDLE_BTN_EV 14	/* ボタンイベントの扱いを中止 */

#endif	/* __XSLAVE_H */
