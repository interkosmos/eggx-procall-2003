#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "e_3d.h"

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
    for (i = 0; i < 4; i++) {
      r[n] += k[i][n] / c[i] / 6;
    }
  }
}

// end{3D 4th Runge-Kutta}

#define WD 290
#define HT 290
#define MG 10
#define N  2000

double SCX = 10;
double SCY = 10;
double SCZ = 5;

/* parameters for duffing */
double A = 0.2;
double B = 0.2;
double C = 5.6;

typedef struct {
  int *win;
  int *win3;
  double *A;
  double *B;
  double *C;
  double *t;
  double *h;
  double *phi;
  double *run;
  double *r;
  double *xg, *yg, *zg;
} param_set;

vec6 rossler(double t, double *r)
{
  vec6 ret;

  ret.f[0] = -r[1] - r[2];
  ret.f[1] = r[0] + A * r[1];
  ret.f[2] = B + r[0] * r[2] - C * r[2];

  return ret;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  static double x[2 * WD], y[2 * WD], z[2 * WD], rt[2 * WD];
  int i;

  for (i = 0; i < 2 * WD - 1; i++) {
    x[i] = x[i + 1];
    y[i] = y[i + 1];
    z[i] = z[i + 1];
    rt[i] = i + MG;
  }
  for (i = 0; i < N - 1; i++) {
    p->xg[i] = p->xg[i + 1];
    p->yg[i] = p->yg[i + 1];
    p->zg[i] = p->zg[i + 1];
  }
  p->xg[N - 1] = x[2 * WD - 1] = p->r[0] * SCX;
  p->yg[N - 1] = y[2 * WD - 1] = p->r[1] * SCY;
  z[2 * WD - 1] = p->r[2] * SCZ;
  p->zg[N - 1] = 3 * z[2 * WD - 1];

  /* y-t chart */
  gclr(*p->win);
  newcolor(*p->win, "LightSteelBlue");
  newlinestyle(*p->win, LineOnOffDash);
  drawline(*p->win, MG, 0, 2 * WD + MG, 0);
  newlinestyle(*p->win, LineSolid);
  newcolor(*p->win, ECTRL_FGCOLOR);
  drawlines(*p->win, rt, x, 2 * WD - 1);
  drawstr(*p->win, 2 * WD - 70, -HT / 2 + 10, 14, 0, "t = %.3f", *p->t);
  newcolor(*p->win, "green3");
  drawlines(*p->win, rt, y, 2 * WD - 1);
  newcolor(*p->win, "red3");
  drawlines(*p->win, rt, z, 2 * WD - 1);
  copylayer(*p->win, 1, 0);

  /* bird-eye */
  gclr(*p->win3);
  newcolor(*p->win3, "LightSteelBlue");
  drawarrow3d(*p->win3, 0, 0, 0, WD / 2, 0, 0, 10, 6, 12);
  drawarrow3d(*p->win3, 0, 0, 0, 0, WD / 2, 0, 10, 6, 12);
  drawarrow3d(*p->win3, 0, 0, 0, 0, 0, WD / 2, 10, 6, 12);
  drawstr3d(*p->win3, WD / 2 + 15, 0, 0, 14, 0, "x");
  drawstr3d(*p->win3, 0, WD / 2 + 10, 0, 14, 0, "y");
  drawstr3d(*p->win3, 0, 0, WD / 2 + 10, 14, 0, "1.5z");
  newcolor(*p->win3, "red3");
  drawlines3d(*p->win3, p->xg, p->yg, p->zg, N);
  copylayer(*p->win3, 1, 0);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i;

  for (i = 0; i < N; i++) {
    p->xg[i] = p->yg[i] = p->zg[i] = 0;
  }
  *p->t = 0;
  p->r[0] = 1.0;
  p->r[1] = 0;
  p->r[2] = 0;
  g3dsetangle(torad(45), torad(*p->phi));
  g3dsetoffset(20, -50);
}

int main()
{
  int win, win3;
  double t = 0, h = 0.03, quit = 0, run = 1;
  double phi = 40;
  double r[6] = { 0 }, xg[N] = {
  0}, yg[N] = {
  0}, zg[N] = {
  0};

  param_set prms = { &win, &win3, &A, &B, &C, &t, &h, &phi,
    &run, r, xg, yg, zg
  };
  e_ctrl ctrls[] = {
    {"A", &A, 0.1, &init, &prms},
    {"B", &B, 0.1, &init, &prms},
    {"C", &C, 1, &init, &prms},
    {"Phi", &phi, 1, &init, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL},
  };
  int cwin;
  cwin = init_ctrls(ctrls, 6);

  win = gopen(2 * WD + 2 * MG, HT);
  win3 = gopen(WD + MG, HT + MG);
  window(win, 0, -HT / 2, 2 * WD + 2 * MG, HT / 2 - 1);
  window(win3, -(MG + WD) / 2, -(HT + MG) / 2, (WD + MG) / 2 - 1,
	 (HT + MG) / 2 - 1);
  winname(win, "x-t, y-t, z/2-t chart");
  winname(win3, "x-z/2 chart");
  layer(win, 0, 1);
  layer(win3, 0, 1);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  gsetbgcolor(win3, ECTRL_FGCOLOR);
  newcolor(win, ECTRL_FGCOLOR);
  newcolor(win3, "red3");
  gclr(win3);
  gsetnonblock(ENABLE);
  init(&prms);
  run = 1;

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 6, wx, wy, iscwin, type, button);
    if (run) {
      rk4fixv6(rossler, t, r, h);
      t += h;
      draw(&prms);
    }
    msleep(3);
  }
  gcloseall();
  return 0;
}
