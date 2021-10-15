#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "stone.h"
#include "e_ctrls.h"

// begin{3D 4-th Runge-Kutta} 
typedef struct {
  double f[6];
} vec6;

void rk4fixv6(vec6 f(), double t, double *r, double h)
{
  int i, n;
  double ts, rs[6], k[4][6];
  double c[4] = { 1, 0.5, 0.5, 1 };
  vec6 diff;

  for (n = 0; n < 6; n++)
    rs[n] = r[n];

  ts = t;
  for (i = 0; i < 4; i++) {
    if (i > 0) {
      for (n = 0; n < 6; n++)
	rs[n] = r[n] + k[i - 1][n] * c[i];
      ts = t + c[i] * h;
    }
    diff = f(ts, rs);
    for (n = 0; n < 6; n++)
      k[i][n] = h * diff.f[n];
  }

  for (n = 0; n < 6; n++) {
    for (i = 0; i < 4; i++)
      r[n] += k[i][n] / c[i] / 6;
  }
}

// end{3D 4th Runge-Kutta}

#define WD     320
#define HT     24
#define BOX    10		/* 固定端箱の幅，高さはその2倍 */
#define R      10		/* 球の半径（画像に依存） */
#define L0     100		/* (WD-2*BOX)/3  */

double K1 = 4.0;		/* 両端のバネ定数2 global にせざるを得ない */
double K2 = 0.5;		/* 中央のバネ定数1 global にせざるを得ない */

typedef struct _param_set {
  int *win;
  double *K2;
  double *x0;
  double *t;
  double *h;
  double *run;
  double r[6];
} param_set;

vec6 mov(double t, double r[6])
{
  vec6 ret;
  ret.f[0] = r[1];
  ret.f[1] = -K1 * r[0] + K2 * (r[2] - r[0]);
  ret.f[2] = r[3];
  ret.f[3] = -K2 * (r[2] - r[0]) - K1 * r[2];

  return ret;
}

extern void spring(int, double, double, double, double,
		   double, double, double);

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;

  gclr(*p->win);
  fillrect(*p->win, -BOX, HT / 2 - BOX, BOX, 2 * BOX);
  fillrect(*p->win, WD - 2 * BOX, HT / 2 - BOX, BOX, 2 * BOX);
  spring(*p->win, 0, HT / 2, L0 + p->r[0] - BOX, 8, 6, 8, 0);
  spring(*p->win, L0 + p->r[0] + R, HT / 2, L0 + p->r[2] - p->r[0] - 2 * R,
	 8, 6, 8, 0);
  spring(*p->win, 2 * L0 + p->r[2] + R, HT / 2, L0 - p->r[2] - R, 8, 6, 8,
	 0);
  putimg24m(*p->win, L0 + p->r[0] - 10, HT / 2 - 10, 20, 20,
	    Xpm_image_stone10);
  putimg24m(*p->win, 2 * L0 + p->r[2] - 10, HT / 2 - 10, 20, 20,
	    Xpm_image_stone10);
  copylayer(*p->win, 1, 0);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  p->r[1] = p->r[2] = p->r[3] = p->r[4] = p->r[5] = 0.0;
  p->r[0] = *p->x0;
  *p->t = 0;
  draw(p);
  *p->run = 0;
}

int main(int argc, char **argv)
{
  int win;
  double x0 = 30, t = 0, h = 0.05;
  double run = 0, quit = 0;

  param_set prms = {
    &win, &K2, &x0, &t, &h, &run
  };

  e_ctrl ctrls[] = {
    {"K2", &K2, 0.1, &init, &prms},
    {"X_init", &x0, 10, &init, &prms},
    {"Step", &h, 0.01, NULL, NULL},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 5);

  win = gopen(WD, HT);
  window(win, -BOX, 0, WD - BOX - 1, HT - 1);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  newcolor(win, "LightSlateGray");
  winname(win, "Harmonic Oscillator: RK4");
  layer(win, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 5, wx, wy, iscwin, type, button);
    if (run) {
      rk4fixv6(mov, t, prms.r, h);
      t += h;
      draw(&prms);
    }
    msleep(40);
  }
  gcloseall();
  return 0;
}
