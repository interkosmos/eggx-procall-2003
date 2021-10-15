/*
　壁での弾性散乱により，速度のx成分がM-B分布に近づく？
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "stone.h"
#include "slope.h"

#define R      150
#define N      40000
#define DN     300
#define Hmax   50
#define WCOLOR "#303040"

double x[N], y[N], vx[N], vy[N];
int vdist[Hmax + 1];

typedef struct {
  int *win;
  int *win2;
  double *x0;
  double *IR;
  double *h;
  double *run;
  double *quit;
} param_set;

void getstatis()
{
  int i, j;

  for (i = 0; i < Hmax; i++)
    vdist[i] = 0;
  for (i = 0; i < N; i++) {
    j = (int) (vx[i] + Hmax) / 2;
    vdist[j]++;
  }
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i;
  double psc = 2 * R / Hmax, vdistmax;

  gclr(*p->win);
  copylayer(*p->win, 2, 1);
  for (i = 0; i < DN; i++) {
    putimg24m(*p->win, x[i] - 4, y[i] - 4, 8, 8, Xpm_image_stone4);
  }
  copylayer(*p->win, 1, 0);

  gclr(*p->win2);
  vdistmax = vdist[0];
  for (i = 0; i < Hmax; i++) {
    if (vdistmax < vdist[i])
      vdistmax = vdist[i];
  }
  newcolor(*p->win2, "steelblue4");
  for (i = 0; i < Hmax; i++) {
    fillrect(*p->win2, psc * i, 0, psc, vdist[i] / vdistmax * R);
  }
  newcolor(*p->win2, "steelblue");
  for (i = 0; i < Hmax; i++) {
    drawrect(*p->win2, psc * i, 0, psc, vdist[i] / vdistmax * R);
  }
  copylayer(*p->win2, 1, 0);
}

void step(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i;
  double xp, yp, xc, yc, xm, ym, phi, th, psi, rr, vv;

  for (i = 0; i < N; i++) {	/* temporay position */
    x[i] += vx[i] * (*p->h);
    y[i] += vy[i] * (*p->h);
    if (hypot(x[i], y[i]) > R) {
      /* search collision point on the outer wall */
      /* with bisection method                    */
      xc = x[i];
      xp = x[i] - vx[i] * (*p->h);
      yc = y[i];
      yp = y[i] - vy[i] * (*p->h);
      do {
	xm = (xp + xc) / 2;
	ym = (yp + yc) / 2;
	if (hypot(xm, ym) > R) {
	  xc = xm;
	  yc = ym;
	} else {
	  xp = xm;
	  yp = ym;
	}
      } while (fabs(hypot(xm, ym) - R) > 1e-2);
      phi = atan2(ym, xm);
      goto RET;
    } else if (hypot(x[i] - (*p->x0), y[i]) < *p->IR) {
      /* search collision point on the inner wall */
      /* with bisection method                    */
      xc = x[i];
      xp = x[i] - vx[i] * (*p->h);
      yc = y[i];
      yp = y[i] - vy[i] * (*p->h);
      do {
	xm = (xp + xc) / 2;
	ym = (yp + yc) / 2;
	if (hypot(xm - (*p->x0), ym) < *p->IR) {
	  xc = xm;
	  yc = ym;
	} else {
	  xp = xm;
	  yp = ym;
	}
      } while (fabs(hypot(xm - (*p->x0), ym) - (*p->IR)) > 1e-2);
      phi = atan2(ym, xm - (*p->x0));
      goto RET;
    } else
      continue;

  RET:
    th = atan2(vy[i], vx[i]) - phi;
    psi = phi - th;
    rr = hypot(x[i] - xm, y[i] - ym);
    vv = hypot(vx[i], vy[i]);
    x[i] = xm - rr * cos(psi);
    y[i] = ym - rr * sin(psi);
    vx[i] = -vv * cos(psi);
    vy[i] = -vv * sin(psi);
  }
  getstatis();
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i, j;
  double th;

  layer(*p->win, 0, 4);		/* mask */
  gsetbgcolor(*p->win, "black");
  gclr(*p->win);
  newcolor(*p->win, "white");
  fillcirc(*p->win, 0, 0, R, R);
  newcolor(*p->win, "black");
  fillcirc(*p->win, *p->x0, 0, *p->IR, *p->IR);


  layer(*p->win, 0, 3);		/* image-mapped wall and colored vacuum */
  gsetbgcolor(*p->win, ECTRL_BGCOLOR);
  gclr(*p->win);
  for (i = 0; i < 2; i++)
    for (j = 0; j < 2; j++)
      putimg24(*p->win, (i - 1) * 200, (j - 1) * 200, 200, 200,
	       Xpm_image_slope);
  newgcfunction(*p->win, GXandInverted);
  gputarea(*p->win, -R, -R, *p->win, 4, -R, -R, R, R);

  layer(*p->win, 0, 2);		/* compose background image */
  gclr(*p->win);
  newgcfunction(*p->win, GXand);
  gputarea(*p->win, -R, -R, *p->win, 4, -R, -R, R, R);
  newgcfunction(*p->win, GXor);
  gputarea(*p->win, -R, -R, *p->win, 3, -R, -R, R, R);
  newgcfunction(*p->win, GXcopy);

  layer(*p->win, 0, 1);
  copylayer(*p->win, 2, 1);	/* copy background image */
  for (i = 0; i < N; i++) {
    th = drand48() * 2 * M_PI;
    x[i] = 0.95 * R * cos(th);
    y[i] = 0.95 * R * sin(th);
    vx[i] = (drand48() - 0.5) * Hmax;
    vy[i] = drand48() * Hmax * sin(drand48() * 2 * M_PI);
  }
  *p->run = 0;
  getstatis();
  draw(p);
}

int main(int argc, char **argv)
{
  int win, win2;
  double quit = 0, run = 1, t = 0, h = 0.05;
  double x0 = -40, IR = R / 3;

  param_set prms = { &win, &win2, &x0, &IR, &h, &run, &quit };
  e_ctrl ctrls[] = {
    {"Step", &h, 0.01, NULL, NULL},
    {"X0", &x0, 1, &init, &prms},
    {"Inner radius", &IR, 1, &init, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 5);

  srand48(time(NULL));
  win = gopen(2 * R, 2 * R);
  win2 = gopen(2 * R, R);
  window(win, -R, -R, R - 1, R - 1);
  gsetbgcolor(win, WCOLOR);
  gsetbgcolor(win2, ECTRL_FGCOLOR);
  newcolor(win, ECTRL_FGCOLOR);
  layer(win, 0, 1);
  layer(win2, 0, 1);
  gsetnonblock(ENABLE);
  init(&prms);
  run = 1;
  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 5, wx, wy, iscwin, type, button);
    if (run) {
      step(&prms);
      draw(&prms);
      t += h;
    }
    msleep(40);
  }
  gcloseall();
  return 0;
}
