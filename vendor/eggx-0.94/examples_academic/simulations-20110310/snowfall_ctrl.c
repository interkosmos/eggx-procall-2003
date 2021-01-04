#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <eggx.h>
#include "tiny_complex.h"
#include "e_ctrls.h"

#define L 300
#define MAXORDER 7
#define MAXSNOW  700

typedef complex double Cdbl;

/* コールバック関数で使うパラメータ・セット */

typedef struct _param_set {
  int *win;
  double *order;
  double *rt;
  double *ra;
  double *rnd;
  double *chbg;
  int *bgcolor_r;
  int *bgcolor_g;
  int *bgcolor_b;
  float snowx[MAXSNOW];
  float snowy[MAXSNOW];
} param_set;

/* 一般の関数 */

static void btree(int win, Cdbl z1, Cdbl z2,
		  int dim, float rt, double ra, double rnd)
{
  Cdbl z3, z4, z5;

  if (dim == -1)
    return;
  z3 = z1 * rt + z2 * (1 - rt);
  z4 = z3 + rt * (z2 -
		  z1) * cexp(I * ra / 180 * M_PI * (rnd * drand48() +
						    (1 - rnd / 2)));
  z5 = z3 + rt * (z2 -
		  z1) * cexp(-I * ra / 180 * M_PI * (rnd * drand48() +
						     (1 - rnd / 2)));
  moveto(win, creal(z1), cimag(z1));
  newlinewidth(win, dim);
  lineto(win, creal(z3), cimag(z3));
  btree(win, z3, z5, dim - 1, rt, ra, rnd);
  btree(win, z3, z4, dim - 1, rt, ra, rnd);
}

static void draw(int win, float snowx[], float snowy[], int nsnow)
{
  int i;
  float *snowx_p;
  float *snowy_p;

  /* 1番は背景専用レイヤ */
  layer(win, 0, 1);
  /* 背景色レイヤをコピー */
  copylayer(win, 2, 1);

  /* 遠くの雪 */
  newrgbcolor(win, 0x0bf, 0x0bf, 0x0bf);
  snowx_p = snowx;
  snowy_p = snowy;
  for (i = 0; i < nsnow / 2; i++) {
    snowx_p[i] += (2.0 * (drand48() - 0.5)) * 0.5;
    snowy_p[i] -= (2.5 + drand48()) * 0.5;
    if (snowy_p[i] <= 0.0)
      snowy_p[i] = L;
  }
  drawpts(win, snowx_p, snowy_p, nsnow / 2);

  /* 木(マスクつき)をコピー */
  newgcfunction(win, GXand);
  gputarea(win, -L / 2, 0, win, 4, -L / 2, 0, L / 2 - 1, L - 1);
  newgcfunction(win, GXor);
  gputarea(win, -L / 2, 0, win, 3, -L / 2, 0, L / 2 - 1, L - 1);
  newgcfunction(win, GXcopy);

  /* 近くの雪 */
  newrgbcolor(win, 0x0ff, 0x0ff, 0x0ff);
  snowx_p = snowx + nsnow / 2;
  snowy_p = snowy + nsnow / 2;
  for (i = 0; i < nsnow / 2; i++) {
    snowx_p[i] += 2.0 * (drand48() - 0.5);
    snowy_p[i] -= 2.5 + drand48();
    if (snowy_p[i] <= 0.0)
      snowy_p[i] = L;
  }
  drawpts(win, snowx_p, snowy_p, nsnow / 2);

  copylayer(win, 1, 0);
}

/* コールバック関数から呼ばれる関数 */

static void remake_tree(int win, double order, double rt, double ra,
			double rnd, int bgcolor_r, int bgcolor_g,
			int bgcolor_b)
{
  Cdbl z2 = 0 + 0.9 * L / (1 - pow(rt, order + 1)) * I;
  Cdbl z1 = 0;
  int i;
  /* 背景色専用レイヤ */
  layer(win, 0, 2);
  for (i = 0; i < L; i++) {
    newrgbcolor(win, bgcolor_r + 128.0 * i / L,
		bgcolor_g + 128.0 * i / L, bgcolor_b + 128.0 * i / L);
    drawline(win, -L / 2, i, L / 2 - 1, i);
  }
  /* treeのマスク専用レイヤ */
  layer(win, 0, 4);
  gsetbgcolor(win, "#ffffff");
  gclr(win);
  newcolor(win, "#000000");
  btree(win, z1, z2, order, rt, ra, rnd);
  /* treeの専用レイヤ */
  layer(win, 0, 3);
  gsetbgcolor(win, "#003300");
  gclr(win);
  newgcfunction(win, GXandInverted);
  gputarea(win, -L / 2, 0, win, 4, -L / 2, 0, L / 2 - 1, L - 1);
  newgcfunction(win, GXcopy);
  newcolor(win, "white");
  drawstr(win, L / 2 - 180, 4, 14, 0, "Background Color: "
	  "#%02x%02x%02x", bgcolor_r, bgcolor_g, bgcolor_b);
}

/* コールバック関数 */

static void change_bgcolor(void *_prms)
{
  param_set *p = (param_set *) _prms;
  *p->bgcolor_r = (int) (drand48() * 96);
  *p->bgcolor_g = (int) (drand48() * 96);
  *p->bgcolor_b = (int) (drand48() * 96);
  remake_tree(*p->win, *p->order, *p->rt, *p->ra, *p->rnd,
	      *p->bgcolor_r, *p->bgcolor_g, *p->bgcolor_b);
  *p->chbg = 0;
}

static void remake_all(void *_prms)
{
  param_set *p = (param_set *) _prms;

  int i;
  for (i = 0; i < MAXSNOW; i++) {
    p->snowx[i] = L * (drand48() - 0.5);
    p->snowy[i] = L * (1 + drand48());
  }
  remake_tree(*p->win, *p->order, *p->rt, *p->ra, *p->rnd,
	      *p->bgcolor_r, *p->bgcolor_g, *p->bgcolor_b);
}

int main(int argc, char **argv)
{
  int win;
  double chbg = 0, quit = 0;
  double order = 8, rt = 0.8, ra = 15, rnd = 0.1;
  int bgcolor_r = 0x036 - 48, bgcolor_g = 0x064 - 48, bgcolor_b =
      0x088 - 48;

  param_set prms = {
    &win, &order, &rt, &ra, &rnd, &chbg,
    &bgcolor_r, &bgcolor_g, &bgcolor_b
  };

  e_ctrl ctrls[] = {
    {"Order", &order, 1, &remake_all, &prms},
    {"Ratio", &rt, 0.01, &remake_all, &prms},
    {"Angle", &ra, 1, &remake_all, &prms},
    {"Randomness", &rnd, 0.01, &remake_all, &prms},
    {"_Change bgcolor", &chbg, 0, &change_bgcolor, &prms},
    {"_Quit", &quit, 0, NULL, NULL},
  };

  int cwin;
  cwin = init_ctrls(ctrls, 6);

  srand48(time(NULL));
  win = gopen(L, L);
  window(win, -L / 2, 0, L / 2 - 1, L - 1);
  winname(win, "Snow falls on fractal tree");
  layer(win, 0, 1);
  remake_all(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int type, button, iscwin;
    double wx, wy;
    if (order > 11)
      order = 12;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 6, wx, wy, iscwin, type, button);
    draw(win, prms.snowx, prms.snowy, MAXSNOW);
    msleep(40);
  }
  gcloseall();
  return 0;
}
