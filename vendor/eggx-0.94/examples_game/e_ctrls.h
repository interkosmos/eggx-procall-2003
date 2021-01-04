#include "btns.h"

#define HTSTR 20
#define M_BTN 280
#define P_BTN 290


typedef struct _e_ctrl 
{
  char    name[30];
  double* rval;
  double  dr;
  void (*usr_fnc)(void *);
  void *usr_ptr;
} e_ctrl;

int display_ctrls(int w, e_ctrl c[], int num, float x, float y, 
                  int isme, int type, int button)
{
  int i, ri, j;
  unsigned char *sw[2]={Xpm_image_off, Xpm_image_on};

  if ( type < 0 && w == isme ) { // type が負なら初期化とする
    layer(w, 0, 1); // 1回めは値を変化させずに，無条件に描画する．
    j = -1;
  }
  else { // 制御窓上でのマウスクリックかどうかのチェック
    j = (num-1)-(int)(y/HTSTR);
    if (isme != w) return 0;
    if (type != ButtonPress) return 0;
  }
  gclr(w);
  for (ri = 0; ri < num; ri++) {
    i = (num-1)-ri;
    if (c[ri].name[0] != '_') {
      if ( ri == j) {
        if ( 285.0 < x) {
          if (button == 3) *c[ri].rval += c[ri].dr/10.0;
          else if (button == 2) *c[ri].rval += c[ri].dr*10.0;
          else *c[ri].rval += c[ri].dr;
        } else
        if ( 270.0 < x && x < 282.0 ) {
          if (button == 3) *c[ri].rval -= c[ri].dr/10.0;
          else if (button == 2) *c[ri].rval -= c[ri].dr*10.0;
          else *c[ri].rval -= c[ri].dr;
        }
      }
      newcolor(w, "SteelBlue4");
      putimg24m(w, 4, HTSTR*i, 130, 19, Xpm_image_flat);
      putimg24m(w, 136, HTSTR*i, 130, 19, Xpm_image_flat);
      newcolor(w, "Beige");
      drawstr(w, 8, HTSTR*i+4, FONTSET, 0, c[ri].name);
      drawstr(w, 142, HTSTR*i+4, FONTSET, 0, "%.6g", *c[ri].rval);
      putimg24m(w, 270, HTSTR*i+1, 14, 16, Xpm_image_l);
      putimg24m(w, 285, HTSTR*i+1, 14, 16, Xpm_image_r);
      if ( ri == j){if (c[ri].usr_fnc == NULL){;} else c[ri].usr_fnc(c[ri].usr_ptr);} 
    } else {
      if (ri == j) {
        *c[ri].rval = 1 - (int)(*c[ri].rval);
        if (c[ri].usr_fnc == NULL){;} else c[ri].usr_fnc(c[ri].usr_ptr);
      }
      newcolor(w, "SteelBlue4");
      drawstr(w, 140, HTSTR*i+4, FONTSET, 0, c[ri].name+1);
      putimg24m(w, 269, HTSTR*i+2, 31,18, sw[(int)(*c[ri].rval)]);
    }
  }
  copylayer(w, 1, 0);
  return 1;
}

int init_ctrls(e_ctrl c[], int num)
{
  int w = gopen(300, num*HTSTR);
  window(w, 0, 0, 300, num*HTSTR);
  gsetbgcolor(w, "ivory");
  gsetfontset(w,"-*-helvetica-bold-r-*-*-%d-*-*-*-*-*-*-*",14);
  gclr(w);
  display_ctrls(w, c, num, 0, 0, w, -1 /* 初期化フラグ */, 0);
  return w;
}

