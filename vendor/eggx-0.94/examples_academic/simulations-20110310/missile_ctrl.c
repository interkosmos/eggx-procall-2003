/* 
　河を渡る船の軌道：
　船と河の流れの速度比をBとおく．
　船は常に目的地を向いている．
　船が向こう岸にたどり着くためには B < 1 が必要．
　変数分離型の常微分方程式なので解析的に解くことができる．
*/
#include <stdio.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "penguin.h"


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



#define WD   800
#define HT   240
#define MR   25
#define CMAX 32
#define NMAX 10000

typedef struct {
  int *win;
  int *win2;
  double *t;
  double *h;
  double *run;
  double *shoot;
  double *reset;
  double *r;
} param_set;

double B = 2;
int cr[CMAX], cg[CMAX], cb[CMAX];

vec6 mov(double t, double *r)
{
  vec6 ret;
  double th;
  if (t == 0)
    th = M_PI / 2;
  else
    th = atan2(1 - r[1], t - r[0]);

  ret.f[0] = B * cos(th);
  ret.f[1] = B * sin(th);

  return ret;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  static double shipx0[5] = { 0, -12, -25, -25, -12 }, shipy0[5] = {
  0, -4, -3, 3, 4};
  double shipx[5], shipy[5], th;
  int i;
  static double posx[NMAX], posy[NMAX];
  static int pi = 0;

  th = atan2(1 - p->r[1], *p->t - p->r[0]);
  posx[pi] = p->r[0] * HT;
  posy[pi] = p->r[1] * HT;

  for (i = 0; i < 5; i++) {	/* ミサイルの向きに合わせた回転 */
    shipx[i] = cos(th) * shipx0[i] - sin(th) * shipy0[i];
    shipy[i] = sin(th) * shipx0[i] + cos(th) * shipy0[i];
  }
  for (i = 0; i < 5; i++) {
    shipx[i] += p->r[0] * HT;
    shipy[i] += p->r[1] * HT;
  }

  layer(*p->win, 0, 2);
  gclr(*p->win);
  newlinestyle(*p->win, LineSolid);
  newlinewidth(*p->win, 2);
  newcolor(*p->win, "gold");
  if (pi >= NMAX)
    pi = NMAX;			/* ミサイルの軌跡 */
  for (i = 0; i < pi; i++)
    fillrect(*p->win, posx[i], posy[i], 2, 2);
  pi++;
  layer(*p->win, 0, 1);
  gclr(*p->win);
  copylayer(*p->win, 2, 1);
  putimg24m(*p->win, HT * (*p->t) - 16, HT - 10, 33, 36,
	    Xpm_image_linux_penguin);
  newcolor(*p->win, "OrangeReD2");
  fillpoly(*p->win, shipx, shipy, 5, 0);	/* ターゲットを向いたミサイル */
  newcolor(*p->win, ECTRL_FGCOLOR);
  fillrect(*p->win, -MR, -3 * MR / 2, WD, MR / 2);
  newlinestyle(*p->win, LineOnOffDash);
  newlinewidth(*p->win, 1);
  drawline(*p->win, posx[pi - 1], posy[pi - 1], HT * (*p->t), HT);
  drawstr(*p->win, WD - 4 * MR, -MR + 10, 14, 0, "t = %.4f", *p->t);
  copylayer(*p->win, 1, 0);
  if (fabs(posx[pi - 1] - HT * (*p->t)) < 1 && fabs(posy[pi - 1] - HT) < 1) {
    *p->run = 0;
  }
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  p->r[0] = 1.5;
  p->r[1] = 0;
  *p->t = 0;
  *p->reset = 0;
  *p->shoot = 0;
  layer(*p->win, 0, 2);
  newcolor(*p->win, ECTRL_FGCOLOR);
  draw(p);
}

int main()
{
  int win, win2;
  double t = 0, h = 0.0025, reset = 0, run = 0, quit = 0;
  double shoot = 0, r[6];

  param_set prms = { &win, &win2, &t, &h, &run, &shoot, &reset, r };

  e_ctrl ctrls[] = {
    {"Vm/Va", &B, 0.1, &init, &prms},
    {"Step", &h, 0.0001, NULL, NULL},
    {"_Run", &run, 0, NULL, NULL},
    {"_Shoot", &shoot, 0, NULL, NULL},
    {"_Reset", &reset, 0, &init, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 6);

  win = gopen(WD, HT + 5 * MR / 2);
  window(win, -MR, -3 * MR / 2, WD - 1 - MR, HT + MR - 1);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  layer(win, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int type, button, iscwin;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 6, wx, wy, iscwin, type, button);
    if (run) {
      if (shoot) {
	rk4fixv6(mov, t, r, h);
      }
      t += h;
      draw(&prms);
    }
    msleep(40);
  }
  gcloseall();
  return 0;
}
