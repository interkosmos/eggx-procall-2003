#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <eggx.h>
#include "e_ctrls.h"

#define NX   150
#define NY   150
#define PIX  2
#define STEP 100

int site[NX + 2][NY + 2];
int new[NX + 2][NY + 2];

typedef struct {
  int *win;
  double *prob;
  double *maxstep;
  int *count;
  double *run;
  double *maxglider;
  double *quit;
  double *dummy;
} param_set;

void setborder()
{
  int i, j;

  for (j = 1; j < NY + 1; ++j) {
    site[0][j] = site[NX][j];
    site[NX + 1][j] = site[1][j];
  }

  for (i = 0; i < NX + 2; ++i) {
    site[i][0] = site[i][NY];
    site[i][NY + 1] = site[i][1];
  }
}

void step()
{
  int i, j, sum, state;

  for (i = 1; i < NX + 1; ++i) {
    for (j = 1; j < NY + 1; ++j) {
      state = site[i][j];
      sum = 0;
      sum = site[i - 1][j - 1] + site[i - 1][j] + site[i - 1][j + 1]
	  + site[i][j - 1] + site[i][j + 1]
	  + site[i + 1][j - 1] + site[i + 1][j] + site[i + 1][j + 1];

      if (state == 0 && sum == 3) {
	new[i][j] = 1;
      } else if (state == 1 && (sum == 2 || sum == 3)) {
	new[i][j] = 1;
      } else {
	new[i][j] = 0;
      }
    }
  }

  for (i = 1; i < NX + 1; i++) {
    for (j = 1; j < NY + 1; j++) {
      site[i][j] = new[i][j];
    }
  }
  setborder();
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  static char *color[] = { ECTRL_BGCOLOR, ECTRL_FGCOLOR };
  int s, i, j;

  gclr(*p->win);
  for (s = 0; s < 2; s++) {
    newcolor(*p->win, color[s]);
    for (i = 0; i < NX; i++)
      for (j = 0; j < NY; j++)
	if (site[i][j] == s)
	  fillrect(*p->win, 2 * i, 2 * j, PIX, PIX);
  }
  copylayer(*p->win, 1, 0);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i, j;

  for (i = 1; i < NX + 1; i++) {
    for (j = 1; j < NY + 1; j++)
      if (drand48() < *p->prob)
	site[i][j] = 1;
      else
	site[i][j] = 0;
  }
  setborder();
  *p->run = 0;
  *p->count = 0;
  draw(p);
}

void glider(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i, j, k, nx, ny, gt;
  int gl[4][3][3] = {
    {{0, 1, 0}, {1, 0, 0}, {1, 1, 1}},
    {{0, 1, 0}, {0, 0, 1}, {1, 1, 1}},
    {{1, 1, 1}, {1, 0, 0}, {0, 1, 0}},
    {{1, 1, 1}, {0, 0, 1}, {0, 1, 0}}
  };

  for (i = 0; i < NX + 2; i++)
    for (j = 0; j < NY + 2; j++)
      site[i][j] = 0;

  for (k = 0; k < *p->maxglider; k++) {
    nx = 3 * (floor(drand48() * (NX / 3)));
    ny = 3 * (floor(drand48() * (NY / 3)));
    gt = (int) (drand48() * 4);
    for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
	site[nx + i][ny + j] = gl[gt][i][j];
  }
  *p->run = 0;
  *p->count = 0;
  *p->dummy = 0;
  draw(p);
}

void train(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int train[5][18] = {
    {0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}
  };

  int i, j;
  for (i = 0; i < NX + 2; i++)
    for (j = 0; j < NY + 2; j++)
      site[i][j] = 0;

  for (i = 0; i < 5; i++)
    for (j = 0; j < 18; j++)
      site[NX - 10 + i][NY / 2 + j] = train[i][j];

  *p->run = 0;
  *p->count = 0;
  *p->dummy = 0;
  *p->maxstep = 250;
  draw(p);
}


int main()
{
  int win, count = 0;
  double probability = 0.5, run = 0, quit = 0;
  double dummy = 0, maxstep = 500, maxglider = 30;

  param_set prms = { &win, &probability, &maxstep, &count,
    &run, &maxglider, &quit, &dummy
  };

  e_ctrl ctrls[] = {
    {"Density", &probability, 0.1, &init, &prms},
    {"Steps", &maxstep, 100, &init, &prms},
    {"No. of glider", &maxglider, 1, &glider, &prms},
    {"_Arrange glider", &dummy, 0, &glider, &prms},
    {"_Arrange train", &dummy, 0, &train, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 7);

  srand48(time(NULL));
  win = gopen(PIX * NX, PIX * NY);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  layer(win, 0, 1);
  gsetnonblock(ENABLE);
  init(&prms);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 7, wx, wy, iscwin, type, button);
    if (run) {
      if (count > maxstep) {
	run = 0;
      }
      step(&prms);
      draw(&prms);
      count++;
      msleep(40);
    }
  }
  ggetch();
  return 0;
}
