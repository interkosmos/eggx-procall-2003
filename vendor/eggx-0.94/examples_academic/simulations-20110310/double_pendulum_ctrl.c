#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "stone.h"
#include "green.h"


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


#define WD 300
#define HT 320
#define SC 150
#define G  9.8
#define L  0.5
#define X0 150			/* WD/2 */
#define Y0 220			/* ほぼ 2*HT/3 */

double Eta = 5;

typedef struct _param_set {
  int *win;
  int *win2;
  double *Th;
  double *h;
  double *t;
  double *run;
  double r[6];
} param_set;

static vec6 mov(double t, double *r)
{
  vec6 ret;
  double C, S, D, GG;
  double Df = r[0] - r[1];

  C = cos(Df);
  S = sin(Df);
  D = Eta - C * C;
  GG = S * (r[3] - C * r[4]) / D * (Eta * r[4] - C * r[3]) / D;

  ret.f[0] = (r[3] - C * r[4]) / D;
  ret.f[1] = (Eta * r[4] - C * r[3]) / D;
  ret.f[3] = -Eta * G * sin(r[0]) + GG;
  ret.f[4] = -G * sin(r[1]) - GG;
  return ret;
}

static void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;

  double x1, y1, x2, y2;

  x1 = X0 + sin(p->r[0]) * L * SC;
  y1 = Y0 - cos(p->r[0]) * L * SC;
  x2 = x1 + sin(p->r[1]) * L * SC;
  y2 = y1 - cos(p->r[1]) * L * SC;
  gclr(*p->win);
  moveto(*p->win, X0, Y0);
  lineto(*p->win, x1, y1);
  lineto(*p->win, x2, y2);
  putimg24m(*p->win, x1 - 6, y1 - 6, 12, 12, Xpm_image_stone6);
  putimg24m(*p->win, x2 - 4, y2 - 4, 8, 8, Xpm_image_green4);
  pset(*p->win2, x2, y2);
  copylayer(*p->win, 1, 0);
  copylayer(*p->win2, 1, 0);
}

static void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  p->r[1] = p->r[2] = p->r[3] = p->r[4] = p->r[5] = 0.0;
  p->r[0] = *p->Th / 180 * M_PI;
  *p->t = 0;
  winname(*p->win2, "ph0 = %.1f, h = %.4f", *p->Th, *p->h);
  gclr(*p->win2);
  draw(p);
  *p->run = 0;
}

int main(int argc, char **argv)
{
  int win, win2;
  double Th = 60, h = 0.02, t = 0;
  double quit = 0, run = 0;

  param_set prms = {
    &win, &win2, &Th, &h, &t, &run
  };

  e_ctrl ctrls[] = {		// 制御窓の定義
    {"Initial Angle", &Th, 1, &init, &prms},
    {"Ratio:m1/m2", &Eta, 1, &init, &prms},
    {"Step", &h, 0.01, NULL, NULL},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 5);	// 制御窓の初期化

  gsetinitialbgcolor(ECTRL_BGCOLOR);
  win = gopen(WD, HT);
  win2 = gopen(WD, HT);
  window(win2, 0, 0, WD, HT);
  newcolor(win, "LightSlateGray");
  newcolor(win2, ECTRL_FGCOLOR);
  winname(win, "Double pendulum");
  winname(cwin, "Control Box");
  layer(win, 0, 1);
  layer(win2, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int type, button, iscwin;
    double wx, wy;
    // 制御窓のイベントを監視して状態変化を反映
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 5, wx, wy, iscwin, type, button);
    ///////////////////////////////////////////
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
