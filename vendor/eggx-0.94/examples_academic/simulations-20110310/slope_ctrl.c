#include <stdio.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "stone.h"
#include "slope.h"

#define WD  400
#define HT  250
#define L   200
#define G   9.8
#define X0  10

typedef struct {
  int *win;
  double *angle;
  double *ratio;
  double *t;
  double *h;
  double *run;
  double *reset;
  double *sx, *sy;		/* *sy always = 0 */
  double *bx, *by;
} param_set;

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  double maskx[4], masky[4];
  double rad;

  rad = *p->angle * M_PI / 180;
  maskx[0] = maskx[1] = *p->sx;
  maskx[2] = maskx[3] = *p->sx + 200;
  masky[0] = 0;
  masky[1] = masky[2] = 200;
  masky[3] = 200 * tan(*p->angle * M_PI / 180);

  gclr(*p->win);
  putimg24(*p->win, *p->sx, 0, 200, 200, Xpm_image_slope);
  newcolor(*p->win, ECTRL_BGCOLOR);
  fillpoly(*p->win, maskx, masky, 4, 0);
  putimg24m(*p->win, *p->bx - 10 * (sin(rad) + 1),
	    *p->by + 10 * (cos(rad) - 1), 20, 20, Xpm_image_stone10);
  newcolor(*p->win, ECTRL_FGCOLOR);
  drawline(*p->win, X0 + L, 0, X0 + L, 220);
  copylayer(*p->win, 1, 0);
}

void step(void *_prms)
{
  param_set *p = (param_set *) _prms;

  double th, dd, c, s, A, ax, ay, rr, denom;

  th = *p->angle * M_PI / 180;
  c = cos(th);
  s = sin(th);
  dd = 2 * (*p->t) * (*p->h) + (*p->h) * (*p->h);
  rr = 1.0 / (*p->ratio);
  denom = 1 + rr * s * s;

  A = rr * c * s * G / denom;
  ax = -c * s * G / denom;
  ay = -(1 + rr) * s * s * G / denom;

  *p->sx += 0.5 * A * dd;
  *p->bx += 0.5 * ax * dd;
  *p->by += 0.5 * ay * dd;
}


void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  if (*p->angle > 45)
    *p->angle = 45;
  *p->sx = X0;
  *p->bx = X0 + L;
  *p->by = L * tan(*p->angle * M_PI / 180);
  *p->run = 0;
  *p->t = 0;
  *p->reset = 0;
  draw(p);
}

int main()
{
  int win;
  double angle = 30, ratio = 5, run = 0, reset = 0, quit = 0;
  double t = 0, h = 0.1;
  double sx, sy, bx, by;

  param_set prms = {
    &win, &angle, &ratio, &t, &h, &run, &reset,
    &sx, &sy, &bx, &by
  };

  e_ctrl ctrls[] = {
    {"Angle", &angle, 1, &init, &prms},
    {"Ratio M/m", &ratio, 1, &init, &prms},
    {"Step", &h, 0.01, &init, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &init, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 6);

  win = gopen(WD, HT);
  layer(win, 0, 1);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  newcolor(win, ECTRL_BGCOLOR);
  init(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 6, wx, wy, iscwin, type, button);
    if (run) {
      t += h;
      step(&prms);
      draw(&prms);
      if (by < 0)
	run = 0;
    }
    msleep(40);
  }
  gcloseall();

  return 0;
}
