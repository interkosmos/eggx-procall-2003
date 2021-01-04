#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include "eggx.h"
#include "e_ctrls.h"

#define L  301			/* should be odd */
#define dR 10

typedef struct {
  int *win;
  double *R;
  double *agg_prob;
  double *d_inc;
  double *run;
  double *reset;
  double *mid;
  double *src_d, *new_d, *rem_d;
} param_set;

int grid[L + 2][L + 2];		/* global にしなくてもよいのだが */

void print_density(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i, j, numt = 0, numa = 0;

  numt = (*p->R) * (*p->src_d);
  for (i = 0; i < L; i++)
    for (j = 0; j < L; j++)
      numa += grid[i][j];

  newcolor(*p->win, ECTRL_FGCOLOR);
  drawstr(*p->win, 100, 10, 14, 0, "Density = %.4f", numa / (double) numt);
  copylayer(*p->win, 1, 0);
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int id, aggr = 0;
  int x, y, xnt, ynt, tk, key = 0;
  double tx[50000], ty[50000];
  double amax, amin;

  amax = *p->mid + *p->R / 2;
  amin = *p->mid - *p->R / 2;

  for (;;) {
    if (*p->new_d) {
      *p->new_d = 0;
      *p->src_d += *p->d_inc;
      *p->rem_d = *p->src_d + *p->d_inc;
      if (*p->src_d >= *p->R) {
	*p->run = 0;
	print_density(p);
	break;
      }
    }

    x = amin + drand48() * (*p->R);
    y = amin + *p->src_d;
    tk = 0;

    while (*p->run) {
      id = floor(drand48() * 4.0);
      xnt = (id == 0) ? x - 1 : (id == 1) ? x + 1 : x;
      ynt = (id <= 1) ? y : (id == 2) ? y - 1 : y + 1;
      tx[tk] = xnt;
      ty[tk] = ynt;

      if ((ynt <= amin) || (ynt >= *p->rem_d + amin)) {
	newcolor(*p->win, "red4");
	break;
      }
      /* seems to escape */
      if (xnt == amin)
	xnt = amin + 1;		/* reflect       */
      if (xnt == amax)
	xnt = amax - 1;		/* at x-boundary */
      if (grid[ynt - 1][xnt] == 1 || grid[ynt][xnt - 1] == 1 ||
	  grid[ynt][xnt + 1] == 1 || grid[ynt + 1][xnt] == 1) {
	aggr = 1;
	if (drand48() <= *p->agg_prob) {
	  grid[ynt][xnt] = 1;
	  if (ynt >= (*p->src_d + amin))
	    *p->new_d = 1;
	  layer(*p->win, 0, 2);
	  newcolor(*p->win, ECTRL_FGCOLOR);
	  pset(*p->win, xnt, ynt);
	  layer(*p->win, 0, 1);
	  newcolor(*p->win, "green3");
	}
	break;
      }				/* aggregate */
      if (aggr == 0) {
	x = xnt;
	y = ynt;
      }
      aggr = 0;
      tk++;
    }
    gclr(*p->win);
    copylayer(*p->win, 2, 1);
    newlinestyle(*p->win, LineSolid);
    drawpts(*p->win, tx, ty, tk - 1);
    newcolor(*p->win, "gray50");
    newlinestyle(*p->win, LineOnOffDash);
    drawline(*p->win, amin, amin + *p->src_d, amax, amin + *p->src_d);
    copylayer(*p->win, 1, 0);
    msleep(5);
    key = ggetch();
    if (key == 'q') {
      *p->run = 0;
      goto RET;
    }
  }
RET:
  ;				/* do nothing just return */
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i, j;
  double amin, amax;

  if (*p->R >= L) {
    while (*p->R >= L - 1) {
      *p->R -= dR;
    }
  }
  *p->mid = (L + 1) / 2;	/* screen mid */
  amin = *p->mid - *p->R / 2;
  amax = *p->mid + *p->R / 2;
  for (i = 0; i < L + 2; i++)
    for (j = 0; j < L + 2; j++)
      grid[i][j] = 0;
  for (j = 0; j < L + 2; j++)
    grid[(int) amin][j] = 1;

  *p->src_d = 10;
  *p->new_d = 1;
  *p->rem_d = 0;
  *p->reset = 0;
  layer(*p->win, 0, 2);
  gsetbgcolor(*p->win, ECTRL_BGCOLOR);
  newcolor(*p->win, "#303030");
  gclr(*p->win);
  fillrect(*p->win, *p->mid - *p->R / 2, *p->mid - *p->R / 2, *p->R,
	   *p->R);
  layer(*p->win, 0, 1);
  copylayer(*p->win, 2, 1);
  copylayer(*p->win, 1, 0);
}

int main(int argc, char *argv[])
{
  int win;
  double R = 2 * L / 3, d_inc = 10, run = 0, reset = 0, quit = 0;
  double agg_prob = 1, mid, src_d = 0, new_d = 1, rem_d = 0;

  param_set prms = { &win, &R, &agg_prob, &d_inc, &run, &reset,
    &mid, &src_d, &new_d, &rem_d
  };

  e_ctrl ctrls[] = {
    {"Size", &R, dR, &init, &prms},
    {"Aggr. prob.", &agg_prob, 0.1, &init, &prms},
    {"Distance Inc.", &d_inc, 1, &init, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &init, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 6);

  srand48(time(NULL));
  win = gopen(L, L);
  gsetnonblock(ENABLE);
  init(&prms);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 6, wx, wy, iscwin, type, button);
    if (run)
      draw(&prms);
  }
  gcloseall();
  return 0;
}
