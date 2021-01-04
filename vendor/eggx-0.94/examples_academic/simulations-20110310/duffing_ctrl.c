#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
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
    for (i = 0; i < 4; i++) {
      r[n] += k[i][n] / c[i] / 6;
    }
  }
}

// end{3D 4th Runge-Kutta}

#define WD 290
#define HT 290
#define MG 10

double SCY = 30;
double SCV = 15;

// parameters for duffing
double D = 0.05;
double B = 1;
double F = 7.5;

typedef struct {
  int *win;
  int *win2;
  int *win3;
  double *F;
  double *D;
  double *B;
  double *t;
  double *h;
  double *clr2;
  double *clr3;
  double *run;
  double *r;
} param_set;

vec6 duffing(double t, double *r)
{
  vec6 ret;

  ret.f[0] = r[1];
  ret.f[1] = -D * r[1] - B * pow(r[0], 3) + F * cos(t);

  return ret;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  static double y[2 * WD], v[2 * WD], rt[2 * WD];
  int i;
  static int poincare = 0;

  for (i = 0; i < 2 * WD - 1; i++) {
    y[i] = y[i + 1];
    v[i] = v[i + 1];
    rt[i] = i + MG;
  }
  y[2 * WD - 1] = p->r[0] * SCY;
  v[2 * WD - 1] = p->r[1] * SCV;

  // y-t chart
  gclr(*p->win);
  newcolor(*p->win, "LightSteelBlue");
  newlinestyle(*p->win, LineOnOffDash);
  drawline(*p->win, MG, 0, 2 * WD + MG, 0);
  newcolor(*p->win, ECTRL_FGCOLOR);
  newlinestyle(*p->win, LineSolid);
  drawlines(*p->win, rt, y, 2 * WD - 1);
  drawstr(*p->win, 2 * WD - 70, -HT / 2 + 10, 14, 0, "t = %.3f", *p->t);
  copylayer(*p->win, 1, 0);

  // phase chart
  if (*p->clr2 == 1) {
    gclr(*p->win2);
    *p->clr2 = 0;
  }
  for (i = 0; i < 2 * WD; i++) {
    pset(*p->win2, y[i], v[i]);
  }
  copylayer(*p->win2, 1, 0);

  // poincare chart
  if (*p->clr3 == 1) {
    gclr(*p->win3);
    *p->clr3 = 0;
  }
  if (poincare % 50 == 0) {
    pset(*p->win3, y[2 * WD - 1], v[2 * WD - 1]);
    copylayer(*p->win3, 1, 0);
  }
  poincare++;
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  *p->t = 0;
  gclr(*p->win2);
  gclr(*p->win3);
  p->r[0] = 0;
  p->r[1] = 0;
  *p->run = 0;
}

int main(int argc, char **argv)
{
  int win, win2, win3;
  double t = 0, h = 2 * M_PI / 100, quit = 0, run = 1;
  double clr2 = 0, clr3 = 0;
  double r[6] = { 0 };

  param_set prms = { &win, &win2, &win3, &F, &D, &B, &t, &h,
    &clr2, &clr3, &run, r
  };
  e_ctrl ctrls[] = {
    {"Force", &F, 0.1, &init, &prms},
    {"Dumping", &D, 0.01, &init, &prms},
    {"Nonlinear", &B, 0.1, &init, &prms},
    {"_Clear Phase", &clr2, 0, NULL, NULL},
    {"_Clear Poincare", &clr3, 0, NULL, NULL},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL},
  };
  int cwin;
  cwin = init_ctrls(ctrls, 7);

  win = gopen(2 * WD + 2 * MG, HT);
  win2 = gopen(WD + MG, HT);
  win3 = gopen(WD + MG, HT);
  window(win, 0, -HT / 2, 2 * WD + 2 * MG, HT / 2 - 1);
  window(win2, -(WD + MG) / 2, -HT / 2, (WD + MG) / 2 - 1, HT / 2 - 1);
  window(win3, -(WD + MG) / 2, -HT / 2, (WD + MG) / 2 - 1, HT / 2 - 1);
  winname(win2, "Phase chart");
  winname(win3, "Poincare chart");
  layer(win, 0, 1);
  layer(win2, 0, 1);
  layer(win3, 0, 1);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  gsetbgcolor(win2, ECTRL_FGCOLOR);
  gsetbgcolor(win3, ECTRL_FGCOLOR);
  newcolor(win, ECTRL_FGCOLOR);
  newcolor(win2, ECTRL_BGCOLOR);
  newcolor(win3, "red3");
  gsetnonblock(ENABLE);
  init(&prms);
  run = 1;

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 7, wx, wy, iscwin, type, button);
    if (run) {
      rk4fixv6(duffing, t, r, h);
      t += h;
      draw(&prms);
    }
    msleep(5);
  }
  gcloseall();
  return 0;
}
