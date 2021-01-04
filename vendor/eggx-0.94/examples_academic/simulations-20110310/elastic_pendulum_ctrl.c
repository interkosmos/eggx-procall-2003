#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "stone.h"


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


void spring(int, double, double, double, double, double, double, double);

#define M  0.1
#define L0 0.25
#define G  9.8
#define WD  400
#define HT  300
#define SC  500

double K = 12;			/* ばね定数　globalにせざるを得ない */

typedef struct _param_set {
  int *win;
  int *win2;
  double *Q0;
  double *r0;
  double *K;
  double *t;
  double *h;
  double *run;
  double r[6];
} param_set;


vec6 epend(double t, double r[6])
{
  vec6 ret;

  ret.f[0] = r[3];		/* dr/dt = r[3] */
  ret.f[1] = r[4];		/* dq/dt = r[4] */
  ret.f[3] = r[0] * r[4] * r[4] + G * cos(r[1]) - K * (r[0] - L0) / M;
  ret.f[4] = -(G * sin(r[1]) + 2 * r[3] * r[4]) / r[0];

  return ret;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  double x, y;

  x = SC * (p->r[0]) * sin(p->r[1]);
  y = HT - SC * (p->r[0]) * cos(p->r[1]);
  gclr(*p->win);
  spring(*p->win, 0, HT, SC * (p->r[0]), 12, 6, 8,
	 180 * (p->r[1]) / M_PI - 90);
  putimg24m(*p->win, x - 10, y - 10, 20, 20, Xpm_image_stone10);
  copylayer(*p->win, 1, 0);
  pset(*p->win2, x - 10, y - 10);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  *p->t = 0;
  p->r[0] = L0 + *p->r0;
  p->r[1] = (M_PI / 180) * (*p->Q0);
  winname(*p->win2, "ph0 = %.1f, r0 = %.4f", *p->Q0, *p->r0);
  winname(*p->win, "Elastic pendulum");
  gclr(*p->win2);
  draw(p);
  *p->run = 0;
}

int main(int argc, char **argv)
{
  int win, win2;
  double Q0 = 60, r0 = 0.05, t = 0, h = 0.01;
  double quit = 0, run = 0;

  param_set prms = {
    &win, &win2, &Q0, &r0, &K, &t, &h, &run
  };

  e_ctrl ctrls[] = {
    {"Init. Angle", &Q0, 1, &init, &prms},
    {"Init. Elongation", &r0, 0.01, &init, &prms},
    {"Elastic const.", &K, 1, &init, &prms},
    {"Step", &h, 0.01, NULL, NULL},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL}
  };

  int cwin;
  cwin = init_ctrls(ctrls, 6);

  gsetinitialbgcolor(ECTRL_BGCOLOR);
  win = gopen(WD, HT);
  window(win, -WD / 2, 0, WD / 2 - 1, HT - 1);
  newcolor(win, "LightSlateGray");
  win2 = gopen(WD, HT);
  window(win2, -WD / 2, 0, WD / 2 - 1, HT - 1);
  newcolor(win2, ECTRL_FGCOLOR);
  layer(win, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 6, wx, wy, iscwin, type, button);
    if (run) {
      rk4fixv6(epend, t, prms.r, h);
      t += h;
      draw(&prms);
    }
    msleep(40);
  }
  gcloseall();
  return 0;
}
