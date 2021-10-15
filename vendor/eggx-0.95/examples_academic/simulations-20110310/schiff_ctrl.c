#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tiny_complex.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "colormode.h"

#define  GRIDS  (512+1)
#define  GW     512
#define  HT     256
#define  WD     512
#define  CMAX   64
#define  EPS    1e-12

typedef struct {
  int *win;
  double *vh;
  double *run;
  double *reset;
  double *cmap;
  double *h2;
  double *dt;
  int *r, *g, *b;
  double *V;
  double complex *phi;
} param_set;

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i, n;
  static double pot_x[6] =
      { -GW / 2, -GW / 128, -GW / 128, GW / 128, GW / 128, GW / 2 };
  static double pot_y[6] = { 0, 0, 50, 50, 0, 0 };
  double x;

  pot_y[2] = pot_y[3] = 50 * (*p->vh);
  gclr(*p->win);
  for (i = 0; i < GW; i++) {
    x = (i - GW / 2);
    n = (int) (carg(p->phi[i]) + M_PI) / (2 * M_PI + EPS) * CMAX;
    newrgbcolor(*p->win, p->r[n], p->g[n], p->b[n]);
    fillrect(*p->win, x, 0, 1, cabs(p->phi[i]) * 90);
  }
  newcolor(*p->win, ECTRL_FGCOLOR);
  drawlines(*p->win, pot_x, pot_y, 6);
  copylayer(*p->win, 1, 0);
}

void step(void *_prms)
{
  param_set *p = (param_set *) _prms;
  double complex k[4][GRIDS];
  double F[3] = { 0.5, 0.5, 1.0 };
  int i, j;

  for (j = 1; j < GW; j++) {
    k[0][j] = I * (*p->dt) *
	((p->phi[j + 1] - 2.0 * p->phi[j] + p->phi[j - 1]) / (*p->h2) -
	 p->V[j] * p->phi[j]);
  }
  for (i = 0; i < 3; i++) {
    for (j = 1; j < GW; j++) {
      k[i + 1][j] = I * (*p->dt) * ((p->phi[j + 1] + k[i][j + 1] * F[i]
				     - 2.0 * (p->phi[j] + k[i][j] * F[i])
				     + p->phi[j - 1] + k[i][j -
							    1] * F[i]) /
				    (*p->h2)
				    - p->V[j] * (p->phi[j] +
						 k[i][j] * F[i]));
    }
  }
  for (j = 0; j < GRIDS; j++) {
    p->phi[j] += (k[0][j] + 2.0 * k[1][j] + 2.0 * k[2][j] + k[3][j]) / 6.0;
  }
}

void change_cmap(void *_prms)
{
  param_set *p = (param_set *) _prms;

  int icmap;
  if ((*p->cmap) < 0 || (*p->cmap) > 59)
    (*p->cmap) = 0;
  icmap = (int) (*p->cmap) % 59;
  int i;
  for (i = 0; i < CMAX; i++)
    makecolor(icmap, 0, CMAX - 1, i, &p->r[i], &p->g[i], &p->b[i]);
  winname(*p->win, "%s", colormodename[icmap]);
  draw(p);
}


void initparam(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i;
  double x;
  const double LEN = 8.0;
  const double X0 = -2.0;	// initial postion
  const double P0 = 24.0;	// initila momentum
  const double VW = 1.0 / 8;	// barrier width
  const double DX = 1.0 / 4;	// initial width
  static double H2, DT;
  double h, V0;

  p->h2 = &H2;
  p->dt = &DT;
  h = LEN / GW;
  H2 = h * h;
  DT = H2 / 6.0;
  V0 = P0 * P0 * 1.25 * (*p->vh);

  for (i = 0; i < GRIDS; i++) {
    x = (i - GW / 2) * h;
    if (fabs(x) > VW / 2)
      p->V[i] = 0.0;
    else
      p->V[i] = V0;
    p->phi[i] = cexp(-pow(x - X0, 2) / (4.0 * DX * DX) + I * P0 * x)
	/ sqrt(sqrt(2.0 * M_PI * DX * DX));
  }
  p->phi[0] = 0;
  p->phi[GW] = p->phi[0];
  *p->run = 0;
  *p->reset = 0;
  draw(p);
}

int main(int argc, char *argv[])
{
  int win, i, count = 0;
  double run = 0, quit = 0, reset = 0, speed = 10;
  double vh = 1.0, cmap = IDL1_RGN_WHT_EXPONENTIAL, h2, dt;
  int r[CMAX], g[CMAX], b[CMAX];
  double V[GRIDS];
  double complex phi[GRIDS];

  param_set prms = { &win, &vh, &run, &reset, &cmap, &h2, &dt,
    r, g, b, V, phi
  };

  e_ctrl ctrls[] = {
    {"Potential", &vh, 0.1, &initparam, &prms},
    {"Speed", &speed, 1, NULL, NULL},
    {"Colormode", &cmap, 1, &change_cmap, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &initparam, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 6);

  win = gopen(WD, HT);
  window(win, -GRIDS / 2, -HT / 4, GRIDS / 2 - 1, 3 * HT / 4 - 1);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  layer(win, 0, 1);
  for (i = 0; i < CMAX; i++)
    makecolor((int) cmap, 0, CMAX - 1, i, &prms.r[i], &prms.g[i],
	      &prms.b[i]);
  initparam(&prms);
  gsetnonblock(ENABLE);

  draw(&prms);
  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 6, wx, wy, iscwin, type, button);
    if (speed <= 0)
      speed = 1;
    if (run) {
      step(&prms);
      if (count % 10 == 0)
	draw(&prms);
    }
    msleep((int) (40 / (speed + 1)));
    count++;
  }
  ggetch();
  gcloseall();

  return 0;
}
