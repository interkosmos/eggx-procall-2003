#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "e_3d.h"
#include "rk4fix.h"
#include "green.h"

#define L 300

#define SC  40
#define N   5000

typedef struct {
  int *win;
  double *Bz;
  double *Ey;
  double *vy0;
  double *vz0;
  double *th;
  double *phi;
  double *t;
  double *h;
  double *run;
  double *keep;
  double *reset;
  double *r;
} param_set;


double Bz = 5;			/* rk4fixv6　内のパラメータ */
double Ey = 1.5;		/* global宣言せざるを得ない */
double QM = 1;

vec6 EBdrift(double t, double *r)
{
  vec6 ret;

  ret.f[0] = r[1];
  ret.f[2] = r[3];
  ret.f[4] = r[5];
  ret.f[1] = r[3] * Bz * QM;
  ret.f[3] = (Ey - r[1] * Bz) * QM;
  ret.f[5] = 0;

  return ret;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  static double trx[N], try[N], trz[N], zero[N];
  static int count = 0;

  if (*p->keep == 0) {
    for (; count >= 0; count--) {
      trx[count] = 0;
      try[count] = 0;
      trz[count] = 0;
    }
    *p->keep = 1;
  }
  trx[count] = SC * p->r[0];
  try[count] = SC * p->r[2];
  trz[count] = SC * p->r[4];
  zero[count] = 0;

  layer(*p->win, 0, 2);
  newlinewidth(*p->win, 1);
  newcolor(*p->win, "gray20");
  drawlines3d(*p->win, trx, try, zero, count);
  newcolor(*p->win, "green3");
  drawlines3d(*p->win, trx, try, trz, count);

  layer(*p->win, 0, 1);
  copylayer(*p->win, 2, 1);
  double th = torad(*p->th);
  double phi = torad(*p->phi);
  putimg24m3d(*p->win,
	      SC * (p->r[0]) - 6 * (-sin(phi) - cos(th) * cos(phi)),
	      SC * (p->r[2]) - 6 * (cos(phi) - cos(th) * sin(phi)),
	      SC * (p->r[4]) - 6 * (sin(th)), 12, 12, Xpm_image_green6);
  copylayer(*p->win, 1, 0);
  count++;
}

void redraw(void *_prms)
{
  param_set *p = (param_set *) _prms;

  g3dsetangle(torad(*p->th), torad(*p->phi));

  layer(*p->win, 0, 3);
  gclr(*p->win);
  newlinewidth(*p->win, 1);
  newcolor(*p->win, "lightsteelblue");
  drawarrow3d(*p->win, 0, 0, 0, 150, 0, 0, 10, 6, 12);
  drawarrow3d(*p->win, 0, 0, 0, 0, 150, 0, 10, 6, 12);
  drawarrow3d(*p->win, 0, 0, 0, 0, 0, 150, 10, 6, 12);
  drawstr3d(*p->win, 150 + 15, 0, -2, 16, 0, "x");

  newcolor(*p->win, "gold4");
  newlinewidth(*p->win, 3);
  drawarrow3d(*p->win, -30, 0, 0, -30, 0, Bz * 20, 10, 6, 12);
  drawstr3d(*p->win, -30, 0, Bz * 20 + 10, 16, 0, "Bz");
  newcolor(*p->win, "gold2");
  drawarrow3d(*p->win, -30, 0, 0, -30, Ey * 75, 0, 10, 6, 12);
  drawstr3d(*p->win, -30, Ey * 75 + 10, 0, 16, 0, "Ey");
  layer(*p->win, 0, 2);
  copylayer(*p->win, 3, 2);
  layer(*p->win, 0, 1);
  draw(p);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  p->r[0] = p->r[1] = p->r[2] = p->r[4] = 0;
  p->r[5] = *p->vz0;
  p->r[3] = *p->vy0;
  *p->run = 0;
  *p->t = 0;
  *p->keep = 0;
  *p->reset = 0;
  g3dsetoffset(70, -30);

  redraw(p);
}


int main()
{
  int win;
  double t = 0, h = 0.02, quit = 0, run = 0, th = 50, phi = 60;
  double r[6] = { 0, 0, 0, 0, 0, 0.5 }, vy0 = 1.5, vz0 = 0.5;
  double keep = 1, reset = 0;
  param_set prms = { &win, &Bz, &Ey, &vy0, &vz0,
    &th, &phi, &t, &h, &run, &keep, &reset, r
  };

  e_ctrl ctrls[] = {
    {"Theta", &th, 1, &redraw, &prms},	/* 結果を視点を変えてみたいので */
    {"Phi", &phi, -1, &redraw, &prms},	/* init しない                  */
    {"Vy0", &vy0, 0.1, &init, &prms},
    {"Vz0", &vz0, 0.1, &init, &prms},
    {"Ey", &Ey, 0.1, &init, &prms},
    {"Bz", &Bz, 0.1, &init, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &init, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin = init_ctrls(ctrls, 9);

  win = gopen(L, L);
  window(win, -L / 3, -L / 2, 2 * L / 3, L / 2);
  winname(win, "ExB drift");
  gsetbgcolor(win, ECTRL_BGCOLOR);
  layer(win, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 9, wx, wy, iscwin, type, button);
    if (run) {
      rk4fixv6(EBdrift, t, r, h);
      draw(&prms);
      t += h;
    }
    if (SC * r[4] > 190) {
      run = 0;
    }
    msleep(10);
  }
  gcloseall();
  return 0;
}
