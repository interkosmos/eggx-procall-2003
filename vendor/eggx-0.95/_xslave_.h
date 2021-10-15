/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-03-06 19:38:53 cyamauch> */

#ifndef __XSLAVE_H
#define __XSLAVE_H 1

/* EGGX �Ǥϡ�minor_code �˥��ޥ�ɤ�ܤ� */
/* GraphicsExpose ���٥�Ȥǿƻ��̿����� */

/* major_code -> windows index of eggx */

/* minor_code */
#define MCODE_DUMMY            0	/* �Ҥ���ƤؤΡִ�λ�פ����ǻ��� */
#define MCODE_NEEDS_REPLY      1	/* �Ƥ��ֿ���褳�� */

#define MCODE_ENABLE           2	/* �ʲ����Ƥ���Ҥ�����������˻��� */
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

#define MCODE_HANDLE_BTN_EV   13	/* �ܥ��󥤥٥�Ȥΰ����򳫻� */
#define MCODE_NOHANDLE_BTN_EV 14	/* �ܥ��󥤥٥�Ȥΰ�������� */

#endif	/* __XSLAVE_H */
