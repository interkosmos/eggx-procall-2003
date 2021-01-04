#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "orange.h"
#include "stone.h"

#define N  500
#define L  300

typedef struct _param_set {
  int *win;
  double *ratio;
  double *dt;
  double x[N];
  double y[N];
  double vx[N];
  double vy[N];

} param_set;

static void init(void *_prms)
{
  param_set *p = (param_set *) _prms;;
  int i;
  double velocity = 200;

  for (i = 0; i < N; i++) {
    p->x[i] = drand48() * L;
    p->y[i] = drand48() * L;
    p->vx[i] = (drand48() - 0.5) * velocity;
    p->vy[i] = (drand48() - 0.5) * velocity;
  }
}

static void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;;

  gclr(*p->win);
  int i;
  for (i = 0; i < N; i++) {
    p->x[i] += p->vx[i] * (*p->dt);
    p->y[i] += p->vy[i] * (*p->dt);
    if (p->x[i] <= 0.0 || p->x[i] >= L)
      p->vx[i] *= -1;
    if (p->y[i] <= 0.0 || p->y[i] >= L)
      p->vy[i] *= -1;
    if (i % (int) ((*p->ratio) + 1))
      putimg24m(*p->win, p->x[i] - 4, p->y[i] - 4, 8, 8,
		Xpm_image_orange4);
    else
      putimg24m(*p->win, p->x[i] - 6, p->y[i] - 6, 12, 12,
		Xpm_image_stone6);
  }
  copylayer(*p->win, 1, 0);
}

static void change_bgcolor(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int r, g, b;

  r = (int) (drand48() * 128);
  g = (int) (drand48() * 128);
  b = (int) (drand48() * 128);
  gsetbgcolor(*p->win, "#%02x%02x%02x", r, g, b);
}

int main(int argc, char **argv)
{
  int win;
  double ratio = 3, dt = 0.02;
  double junk = 0, quit = 0, run = 1;

  param_set prms = {
    &win, &ratio, &dt
  };

  e_ctrl ctrls[] = {
    {"Ratio n1/n2", &ratio, 1, &init, &prms},
    {"Step", &dt, 0.01, NULL, NULL},
    {"_Change bgcolor", &junk, 0, &change_bgcolor, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL}
  };

  int cwin;
  cwin = init_ctrls(ctrls, 5);

  srand48(time(NULL));
  win = gopen(L, L);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  layer(win, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int type, button, iscwin;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 5, wx, wy, iscwin, type, button);
    if (run)
      draw(&prms);
    if (ratio < 1) {
      ratio = 0;
      display_ctrls(cwin, ctrls, 5, P_BTN, 95, cwin, ButtonPress, 1);
    }
    msleep(40);
  }
  gcloseall();
  return 0;
}
