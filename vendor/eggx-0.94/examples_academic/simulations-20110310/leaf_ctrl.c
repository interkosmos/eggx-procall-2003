#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tiny_complex.h"
#include "eggx.h"
#include "e_ctrls.h"

#define WD 400
#define HT 300
#define MG 4
#define SC 350

typedef complex double Cdbl;

typedef struct _param_set {
  int *win;
  double *order;
  double *br, *bi, *cr, *ci;
  Cdbl *a, *b, *c, *d;
  double *reset;
  int gr[256], gg[256], gb[256];
} param_set;


Cdbl f(Cdbl z, int rank, int parity, Cdbl a, Cdbl b, Cdbl c, Cdbl d)
{
  if (rank != 0) {
    if (parity % 2) {
      z = a * z + b * conj(z);
    } else {
      z = c * (z - 1) + d * (conj(z) - 1) + 1;
    }
    return f(z, rank - 1, parity / 2, a, b, c, d);
  } else {
    return z;
  }
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int range, i;
  Cdbl z;

  *p->b = *p->br + *p->bi * I;
  *p->c = *p->cr + *p->ci * I;
  range = (int) pow(2, *p->order);
  gclr(*p->win);
  for (i = 1; i <= range; i++) {
    newrgbcolor(*p->win, p->gr[i % 256], p->gg[i % 256], p->gb[i % 256]);
    z = f(0, *p->order, i, *p->a, *p->b, *p->c, *p->d);
    pset(*p->win, SC * creal(z), SC * cimag(z));
    if ( i % 40000 == 0 ) copylayer(*p->win, 1,0);
  }
  drawstr(*p->win, 0, HT - 10, 14, 0, "b = %+6f %+6fi", *p->br, *p->bi);
  drawstr(*p->win, WD / 2, HT - 10, 14, 0,
	  "c = %+6f %+6fi", *p->cr, *p->ci);
  copylayer(*p->win, 1,0);
}

void param_reset(void *_prms)
{
  param_set *p = (param_set *) _prms;

  *p->order = 13;
  *p->br = 0.4614;
  *p->bi = 0.4614;
  *p->cr = 0.662;
  *p->ci = -0.196;
  *p->reset = 0;
}

int main(void)
{
  int win, i;
  Cdbl a = 0, b = 0, c = 0, d = 0;
  double br = 0.4614, bi = 0.4614, cr = 0.662, ci = -0.196;
  double order = 13, reset = 0, quit = 0;

  param_set prms = {
    &win, &order, &br, &bi, &cr, &ci, &a, &b, &c, &d, &reset
  };

  e_ctrl ctrls[] = {
    {"Order", &order, 1, NULL, NULL},
    {"real(b)", &br, 0.01, NULL, NULL},
    {"imag(b)", &bi, 0.01, NULL, NULL},
    {"real(c)", &cr, 0.01, NULL, NULL},
    {"image(c)", &ci, 0.01, NULL, NULL},
    {"_Reset c,b", &reset, 0, &param_reset, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };

  int cwin;
  cwin = init_ctrls(ctrls, 7);

  gsetinitialbgcolor(ECTRL_FGCOLOR);
  win = gopen(WD + 2 * MG, HT + 2 * MG);
  window(win, -MG, -MG, WD + MG, HT + MG);
  winname(win, "Fractal Tree");
  layer(win,0,1);
  newpen(win, 13);
  param_reset(&prms);
  for (i = 0; i < 256; i++)
    makecolor(DS9_GREEN, 0, 255, i, prms.gr + i, prms.gg + i, prms.gb + i);
  draw(&prms);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    if (button) {
      display_ctrls(cwin, ctrls, 7, wx, wy, iscwin, type, button);
      if (quit) break;
      draw(&prms);
      button = 0;
    }
    msleep(100);
  }
  gcloseall();
  return 0;
}
