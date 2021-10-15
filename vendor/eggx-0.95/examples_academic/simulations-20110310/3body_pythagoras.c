#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "red.h"
#include "green.h"
#include "cyan.h"

#define N  2

typedef struct {
  double f[6 * N];
} vecN;

void rk4fixN(vecN f(), double t, double *r, double h)
{
  int i, n;
  double ts, rs[6 * N], k[4][6 * N];
  double c[4] = { 1, 0.5, 0.5, 1 };
  vecN diff;

  for (n = 0; n < 6 * N; n++) {
    rs[n] = r[n];
  }

  ts = t;
  for (i = 0; i < 4; i++) {
    if (i > 0) {
      for (n = 0; n < 6 * N; n++) {
	rs[n] = r[n] + k[i - 1][n] * c[i];
      }
      ts = t + c[i] * h;
    }
    diff = f(ts, rs);
    for (n = 0; n < 6 * N; n++) {
      k[i][n] = h * diff.f[n];
    }
  }

  for (n = 0; n < 6 * N; n++) {
    for (i = 0; i < 4; i++) {
      r[n] += k[i][n] / c[i] / 6;
    }
  }
}

#define L  500
#define G  1
#define M3 3.0
#define M4 4.0
#define M5 5.0
#define SC 50
#define E0 (12.8 + 1.0/60)
#define LNUM 100000

double rr[LNUM * 6];

typedef struct {
  int *win;
  int *count;
  int *icount;
  double *t;
  double *reset;
  double *r;
} param_set;

vecN motion(double t, double *r)
{
  int i, j;
  vecN ret;
  double r40, r51, r80, r91, r84, r95;
  double rc34, rc45, rc53;

  r40 = r[4] - r[0];
  r51 = r[5] - r[1];
  r80 = r[8] - r[0];
  r91 = r[9] - r[1];
  r84 = r[8] - r[4];
  r95 = r[9] - r[5];
  rc34 = pow(hypot(r40, r51), 3);
  rc45 = pow(hypot(r84, r95), 3);
  rc53 = pow(hypot(r80, r91), 3);

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 2; j++)
      ret.f[4 * i + j] = r[4 * i + j + 2];	/* r = dr/dt */
  }
  ret.f[2] = G * (M4 * r40 / rc34 + M5 * r80 / rc53);
  ret.f[3] = G * (M4 * r51 / rc34 + M5 * r91 / rc53);
  ret.f[6] = G * (M5 * r84 / rc45 + M3 * (-r40) / rc34);
  ret.f[7] = G * (M5 * r95 / rc45 + M3 * (-r51) / rc34);
  ret.f[10] = -G * (M3 * r80 / rc53 + M4 * r84 / rc45);
  ret.f[11] = -G * (M3 * r91 / rc53 + M4 * r95 / rc45);

  return ret;
}

double calc_energy(void *_prms)
{
  param_set *p = (param_set *) _prms;
  double K, U;

  K = (M3 * (pow(p->r[2], 2) + pow(p->r[3], 2)) +
       M4 * (pow(p->r[6], 2) + pow(p->r[7], 2)) +
       M5 * (pow(p->r[10], 2) + pow(p->r[11], 2))) / 2;
  U = G * M3 * M4 / hypot(p->r[0] - p->r[4], p->r[1] - p->r[5]) +
      G * M4 * M5 / hypot(p->r[4] - p->r[8], p->r[5] - p->r[9]) +
      G * M5 * M3 / hypot(p->r[8] - p->r[0], p->r[9] - p->r[1]);

  return K - U;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i;

  layer(*p->win, 0, 1);
  newlinestyle(*p->win, LineSolid);
  newcolor(*p->win, "red3");
  for (i = 0; i < *p->icount; i++) {
    pset(*p->win, SC * rr[6 * i + 0], SC * rr[6 * i + 1]);
  }
  newcolor(*p->win, "green4");
  for (i = 0; i < *p->icount; i++) {
    pset(*p->win, SC * rr[6 * i + 2], SC * rr[6 * i + 3]);
  }
  newcolor(*p->win, "cyan4");
  for (i = 0; i < *p->icount; i++) {
    pset(*p->win, SC * rr[6 * i + 4], SC * rr[6 * i + 5]);
  }
  newcolor(*p->win, ECTRL_FGCOLOR);
  fillrect(*p->win, L / 2 - 250, L / 2 - 20, L / 2, 12);
  newcolor(*p->win, ECTRL_BGCOLOR);
  drawstr(*p->win, L / 2 - 250, L / 2 - 20, 14, 0,
	  "e = %15.9e, t = %.4f", calc_energy(p), *p->t);
  copylayer(*p->win, 1, 0);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i;

  for (i = 0; i < 6 * N; i++) {
    p->r[i] = 0;
  }
  p->r[0] = 1.0;
  p->r[1] = 8.0 / 3;
  p->r[4] = -2.0;
  p->r[5] = -4.0 / 3;;
  p->r[8] = 1.0;
  p->r[9] = -4.0 / 3;
  *p->t = 0;
  *p->count = 0;
  *p->icount = 0;
  *p->reset = 0;
  gclr(*p->win);
  newcolor(*p->win, "lightsteelblue");
  newlinestyle(*p->win, LineOnOffDash);
  for (i = -4; i < 5; i++) {
    drawline(*p->win, SC * i, -L / 2, SC * i, L / 2);
    drawline(*p->win, -L / 2, SC * i, L / 2, SC * i);
  }
  newlinestyle(*p->win, LineSolid);
  drawline(*p->win, 0, -L / 2, 0, L / 2);
  drawline(*p->win, -L / 2, 0, L / 2, 0);
  putimg24m(*p->win, SC * 1.0 - 4, SC * 8.0 / 3 - 4, 8, 8, Xpm_image_red4);
  putimg24m(*p->win, SC * (-2) - 6, SC * (-4.0 / 3) - 6, 12, 12,
	    Xpm_image_green6);
  putimg24m(*p->win, SC * 1.0 - 6, SC * (-4.0 / 3) - 6, 12, 12,
	    Xpm_image_cyan6);
  copylayer(*p->win, 1, 0);
  draw(p);
}

int main()
{
  int win, count = 0, icount = 0;
  double r[6 * N], t = 0, h = 0.01;
  double quit = 0, run = 1, e0 = 0, reset = 0;

  param_set prms = { &win, &count, &icount, &t, &reset, r };

  e_ctrl ctrls[] = {
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &init, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin = init_ctrls(ctrls, 3);

  win = gopen(L, L);
  window(win, -L / 2, -L / 2, L / 2, L / 2);
  gsetbgcolor(win, ECTRL_FGCOLOR);
  layer(win, 0, 1);
  gsetnonblock(ENABLE);
  init(&prms);
  e0 = calc_energy(&prms);

  while (!quit) {
    int iscwin, type, button, i;
    double wx, wy, r34, r45, r53, rmin;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 3, wx, wy, iscwin, type, button);

    if (run) {
      r34 = hypot(r[0] - r[4], r[1] - r[5]);
      r45 = hypot(r[4] - r[8], r[5] - r[9]);
      r53 = hypot(r[8] - r[0], r[9] - r[1]);
      rmin = r34;
      if (r45 < rmin)
	rmin = r45;
      if (r53 < rmin)
	rmin = r53;

      h = rmin / 3000;
      if (rmin < 0.1)
	h = rmin / 6000;
      if (rmin < 0.01)
	h = rmin / 24000;
      rk4fixN(motion, t, r, h);
      t += h;
      if (count % 10 == 0) {
	for (i = 0; i < 3; i++) {
	  rr[icount * 6 + 2 * i] = r[4 * i];
	  rr[icount * 6 + 2 * i + 1] = r[4 * i + 1];
	}
	icount++;
      }
      count++;
    }
    if (count % 10000 == 0) {
      draw(&prms);
      msleep(5);
    }
    if (t > 69) {
      run = 0;
    }
    if (!run) {
      msleep(100);
    }
  }
  gcloseall();
  return 0;
}
