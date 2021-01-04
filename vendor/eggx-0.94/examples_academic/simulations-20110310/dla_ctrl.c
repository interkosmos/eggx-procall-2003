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
  double *r_inc;
  double *run;
  double *reset;
  double *mid;
  double *src_r, *new_r, *rem_r;
} param_set;

int grid[L + 2][L + 2];		/* globalにしなくてもよいが 2次元配列は厄介 */

void print_density(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i, j, numt = 0, numa = 0;

  for (i = 0; i < L; i++) {
    for (j = 0; j < L; j++) {
      if (hypot(i - *p->mid, j - *p->mid) <= *p->src_r)
	numt++;
      numa += grid[i][j];
    }
  }
  newcolor(*p->win, "cornsilk");
  drawstr(*p->win, 100, 10, 14, 0, "Density = %.4f", numa / (double) numt);
  copylayer(*p->win, 1, 0);
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int id, aggr = 0, this_r;
  int x, y, xnt, ynt, tk, key = 0;
  float tx[50000], ty[50000], th;

  for (;;) {
    if (*p->new_r) {
      *p->new_r = 0;
      *p->src_r += *p->r_inc;
      *p->rem_r = *p->src_r + *p->r_inc;
      if (*p->src_r >= (*p->R / 2)) {
	*p->run = 0;
	print_density(p);
	break;
      }
    }

    th = drand48() * 2.0 * M_PI;
    x = *p->src_r * cos(th) + *p->mid;
    y = *p->src_r * sin(th) + *p->mid;
    tk = 0;

    while (*p->run) {
      id = floor(drand48() * 4.0);
      xnt = (id == 0) ? x - 1 : (id == 1) ? x + 1 : x;
      ynt = (id <= 1) ? y : (id == 2) ? y - 1 : y + 1;
      tx[tk] = xnt;
      ty[tk] = ynt;
      this_r = hypot(xnt - *p->mid, ynt - *p->mid);

      if ((xnt < *p->mid - *p->R) || (xnt >= *p->mid + *p->R) ||
	  (ynt < *p->mid - *p->R) || (ynt >= *p->mid + *p->R) ||
	  (this_r > *p->rem_r)) {
	newcolor(*p->win, "red4");
	break;
      }
      /* seems to escape */
      if (grid[xnt - 1][ynt] == 1 || grid[xnt][ynt - 1] == 1 ||
	  grid[xnt][ynt + 1] == 1 || grid[xnt + 1][ynt] == 1) {
	aggr = 1;
	if (drand48() <= *p->agg_prob) {
	  grid[xnt][ynt] = 1;
	  if (this_r >= *p->src_r)
	    *p->new_r = 1;
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
    drawarc(*p->win, *p->mid, *p->mid, *p->src_r, *p->src_r, 0, 360, 0);
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

  *p->mid = (L + 1) / 2;	/* screen mid */
  for (i = 0; i < L + 2; i++)
    for (j = 0; j < L + 2; j++)
      grid[i][j] = 0;
  grid[(int) *p->mid][(int) *p->mid] = 1;

  if (*p->R >= L) {
    while (*p->R >= L - 1) {
      *p->R -= dR;
    }
  }
  *p->src_r = 0;
  *p->new_r = 1;
  *p->rem_r = 0;
  *p->reset = 0;
  layer(*p->win, 0, 2);
  gsetbgcolor(*p->win, ECTRL_BGCOLOR);
  newcolor(*p->win, "#303030");
  gclr(*p->win);
  fillarc(*p->win, *p->mid, *p->mid, *p->R / 2, *p->R / 2, 0, 360, 0);
  layer(*p->win, 0, 1);
  copylayer(*p->win, 2, 1);
  copylayer(*p->win, 1, 0);
}

int main(int argc, char *argv[])
{
  int win;
  double R = 2 * L / 3, r_inc = 10, run = 0, reset = 0, quit = 0;
  double agg_prob = 1, mid, src_r = 0, new_r = 1, rem_r = 0;

  param_set prms = { &win, &R, &agg_prob, &r_inc, &run, &reset,
    &mid, &src_r, &new_r, &rem_r
  };

  e_ctrl ctrls[] = {
    {"Size", &R, dR, &init, &prms},
    {"Aggr. prob.", &agg_prob, 0.1, &init, &prms},
    {"Radius Inc.", &r_inc, 1, &init, &prms},
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
