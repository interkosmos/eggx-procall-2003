#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "colormode.h"

#define L    150
#define CMAX 256
#define PIX  2

typedef struct {
  int *win;
  double *order_n;
  double *order_m;
  double *t;
  double *h;
  double *run;
  double *cmap;
  unsigned char *buffer;
  int *r, *g, *b;
} param_set;

void pset_on_buffer(unsigned char *buf, int width, int height,
		    int x, int y, int red, int green, int blue)
{
  if (0 <= x && x < width && 0 <= y && y < height) {
    int yy = height - 1 - y;
    if (red < 0)
      red = 0;
    else if (255 < red)
      red = 255;
    if (green < 0)
      green = 0;
    else if (255 < green)
      green = 255;
    if (blue < 0)
      blue = 0;
    else if (255 < blue)
      blue = 255;
    buf[4 * (width * yy + x)] = 255;
    buf[4 * (width * yy + x) + 1] = red;
    buf[4 * (width * yy + x) + 2] = green;
    buf[4 * (width * yy + x) + 3] = blue;
  }
  return;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i, j, n, ii, jj;
  double xr, th, jnr0;
  const double jz[8][5] = {
    {2.40483, 5.52008, 8.65373, 11.7915, 14.9309},
    {3.83171, 7.01559, 10.1735, 13.3237, 16.4706},
    {5.13562, 8.41724, 11.6198, 14.7960, 17.9598},
    {6.38016, 9.76012, 13.0152, 16.2235, 19.4094},
    {7.58834, 11.0647, 14.3725, 17.6160, 20.8269},
    {8.77148, 12.3386, 15.7002, 18.9801, 22.2178},
    {9.93611, 13.5893, 17.0038, 20.3208, 23.5861},
    {11.0864, 14.8213, 18.2876, 21.6415, 24.9349},
  };				/* Zeros of  Bessel function */

  if (*p->order_n >= 7)
    *p->order_n = 7;
  if (*p->order_n <= 0)
    *p->order_n = 0;
  if (*p->order_m >= 4)
    *p->order_m = 4;
  jnr0 = jz[(int) *p->order_n][(int) *p->order_m];
  gsetbgcolor(*p->win, ECTRL_BGCOLOR);
  gclr(*p->win);
  for (i = 0; i < L; i++) {
    for (j = 0; j < L; j++) {
      if (hypot(i - L / 2, j - L / 2) <= L / 2) {
	xr = hypot(i - L / 2, j - L / 2) / (L / 2) * jnr0;
	th = atan2(i - L / 2, j - L / 2);
	n = CMAX * (sin(*p->t * M_PI * jnr0) * jn(*p->order_n, xr)
		    * cos(*p->order_n * th) + 1) / 2;
	for (ii = 0; ii < PIX; ii++)
	  for (jj = 0; jj < PIX; jj++)
	    pset_on_buffer(p->buffer, PIX * L, PIX * L, PIX * i + ii,
			   PIX * j * jj, p->r[n], p->g[n], p->b[n]);
      } else {
	for (ii = 0; ii < PIX; ii++)
	  for (jj = 0; jj < PIX; jj++)
	    pset_on_buffer(p->buffer, PIX * L, PIX * L, PIX * i + ii, PIX * j * jj, 0x36, 0x64, 0x8B);	/* steelblue4 */
      }
    }
  }
  gputimage(*p->win, 0, 0, p->buffer, PIX * L, PIX * L, 0);
  copylayer(*p->win, 1, 0);
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
  winname((*p->win), "%s", colormodename[icmap]);
}

int main()
{
  int win, i;
  double t = 0, h = 0.01, cmap = IDL2_WAVES;
  double order_n = 1, order_m = 1, quit = 0, run = 1;
  unsigned char *buffer = NULL;
  int r[CMAX], g[CMAX], b[CMAX];

  buffer = malloc(PIX * L * PIX * L * 4);
  if (buffer == NULL) {
    fprintf(stderr, "ERROR: malloc() failed.\n");
    return -1;
  }

  param_set prms = { &win, &order_n, &order_m, &t, &h, &run, &cmap,
    buffer, r, g, b
  };

  e_ctrl ctrls[] = {
    {"Node_n max=7", &order_n, 1, NULL, NULL}
    ,
    {"Node_m max=4", &order_m, 1, NULL, NULL}
    ,
    {"Step", &h, 0.001, NULL, NULL}
    ,
    {"Colormode", &cmap, 1, &change_cmap, &prms}
    ,
    {"_Run", &run, 0, NULL, NULL}
    ,
    {"_Quit", &quit, 0, NULL, NULL}
  };

  int cwin = init_ctrls(ctrls, 6);

  win = gopen(PIX * L, PIX * L);
  layer(win, 0, 1);
  for (i = 0; i < CMAX; i++)
    makecolor(cmap, 0, CMAX - 1, i, &prms.r[i], &prms.g[i], &prms.b[i]);
  fillarc(win, 0, 0, L / 2, L / 2, 0, 360, 0);
  winname(win, "%s", colormodename[(int) cmap]);
  gsetnonblock(ENABLE);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 6, wx, wy, iscwin, type, button);
    if (run) {
      t += h;
      draw(&prms);
    }
    msleep(40);
  }
  gcloseall();

  return 0;
}
