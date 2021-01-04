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


/* begin{3D 4-th Runge-Kutta} */
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

/* end{3D 4th Runge-Kutta} */



#define WD   240
#define HT   450
#define MR   30
#define CMAX 32
#define NMAX 10000

typedef struct {
  int *win;
  int *win2;
  double *t;
  double *h;
  double *reset;
  double *r;
} param_set;

double B = 0.5;
int cr[CMAX], cg[CMAX], cb[CMAX];

vec6 mov(double t, double *r)
{
  vec6 ret;

  ret.f[0] = -r[0] / hypot(r[0], r[1]);
  ret.f[1] = B - r[1] / hypot(r[0], r[1]);

  return ret;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  static double shipx0[5] = { 0, 12, 30, 30, 12 }, shipy0[5] = {
  0, -5, -3, 3, 5};
  double shipx[5], shipy[5], th;
  int i, n;
  static double posx[NMAX], posy[NMAX];
  static int pi = 0;

  th = atan2(p->r[1], p->r[0]);
  posx[pi] = p->r[0] * WD;
  posy[pi] = p->r[1] * WD;

  for (i = 0; i < 5; i++) {	/* 船の向きに合わせた回転 */
    shipx[i] = cos(th) * shipx0[i] - sin(th) * shipy0[i];
    shipy[i] = sin(th) * shipx0[i] + cos(th) * shipy0[i];
  }
  for (i = 0; i < 5; i++) {
    shipx[i] += p->r[0] * WD;
    shipy[i] += p->r[1] * WD;
  }

  layer(*p->win, 0, 2);
  gclr(*p->win);
  putimg24m(*p->win, -MR - 4, 6 * pow(sin(8 * M_PI * (*p->t)), 4) - 10,
	    33, 36, Xpm_image_linux_penguin);
  newlinestyle(*p->win, LineSolid);
  newlinewidth(*p->win, 2);
  for (i = -WD; i < HT / 2; i++) {
    n = (int) CMAX *(sin(-0.02 * WD * B * M_PI * (*p->t) + i / 2.0)) + 170;
    newhsvcolor(*p->win, 190, 255, n);
    drawline(*p->win, 0, 2 * i, WD, 2 * i);
  }
  newcolor(*p->win, "gold");
  if (pi >= NMAX)
    pi = NMAX;			/* 船の軌跡 */
  for (i = 0; i < pi; i++)
    fillrect(*p->win, posx[i], posy[i], 2, 2);
  pi++;
  layer(*p->win, 0, 1);
  gclr(*p->win);
  copylayer(*p->win, 2, 1);
  newcolor(*p->win, "OrangeReD2");
  fillpoly(*p->win, shipx, shipy, 5, 0);	/* 目的地を向いた船 */
  newcolor(*p->win, ECTRL_FGCOLOR);
  newlinestyle(*p->win, LineOnOffDash);
  newlinewidth(*p->win, 1);
  drawline(*p->win, 0, 0, WD, WD * tan(th));	/* 船と目的地を結ぶ直線 */
  drawstr(*p->win, WD - 3 * MR, HT - 2 * MR, 14, 0, "t = %.4f", *p->t);
  copylayer(*p->win, 1, 0);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  p->r[0] = 1;
  p->r[1] = 0;
  *p->t = 0;
  *p->reset = 0;
  layer(*p->win, 0, 2);
  newcolor(*p->win, ECTRL_BGCOLOR);
  fillrect(*p->win, 0, -MR, WD, HT);
  newcolor(*p->win, "#999999");
  putimg24m(*p->win, -MR - 4, -10, 33, 36, Xpm_image_linux_penguin);
  draw(p);
}

int main()
{
  int win, win2;
  double t = 0, h = 0.005, reset = 0, run = 0, quit = 0;
  double r[6];

  param_set prms = { &win, &win2, &t, &h, &reset, r };

  e_ctrl ctrls[] = {
    {"Vr/Vs", &B, 0.1, &init, &prms},
    {"Step", &h, 0.001, NULL, NULL},
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &init, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 5);

  win = gopen(WD + 2 * MR, HT);
  window(win, -MR, -MR, WD + MR - 1, HT - MR - 1);
  gsetbgcolor(win, ECTRL_FGCOLOR);
  layer(win, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int type, button, iscwin;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 5, wx, wy, iscwin, type, button);
    if (run) {
      rk4fixv6(mov, t, r, h);
      t += h;
      draw(&prms);
      if (r[0] < 1e-2 && r[1] < 1e-2)
	run = 0;
    }
    msleep(40);
  }
  gcloseall();
  return 0;
}
