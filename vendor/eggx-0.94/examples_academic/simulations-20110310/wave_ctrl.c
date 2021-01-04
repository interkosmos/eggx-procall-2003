#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "colormode.h"

#define CMAX  128
#define L     256
#define PIX   2
#define XORIG 8
#define YORIG 16

typedef struct _param_set {
  int *win;
  double *freq;
  double *kw;
  double *dth;
  double *atn;
  double *t;
  double *cmap;
  int r[CMAX];
  int g[CMAX];
  int b[CMAX];
} param_set;

/* バッファに pset する*/
void pset_on_buffer(unsigned char *buf, int width, int height,
		    int x, int y, int red, int green, int blue)
{
  if (0 <= x && x < width && 0 <= y && y < height) {
    int yy = height - 1 - y;	/* ←左下を原点にする場合 */
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

void draw(int win, double freq, double kw, double dth, double atn,
	  double t, double cmap, int *r, int *g, int *b,
	  unsigned char *buffer)
{
  gclr(win);
  int i, j, ii, jj, n;
  double dr1, dr2;
  for (i = 0; i < L; i++) {
    for (j = 0; j < L; j++) {
      dr1 = hypot(i - XORIG, j - L / 2 + YORIG);
      dr2 = hypot(i - XORIG, j - L / 2 - YORIG);
      n = CMAX * (sin(kw * dr1 - 2 * M_PI * freq * t + dth / 180 * M_PI)
		  * exp(-atn * dr1)
		  + sin(kw * dr2 - 2 * M_PI * freq * t)
		  * exp(-atn * dr2) + 2.0) / 4.0;
      for (ii = 0; ii < PIX; ii++) {
	for (jj = 0; jj < PIX; jj++) {
	  pset_on_buffer(buffer, PIX * L, PIX * L, PIX * i + ii,
			 PIX * j + jj, r[n], g[n], b[n]);
	}
      }
    }
  }
  gputimage(win, 0, 0, buffer, PIX * L, PIX * L, 0);
  fillrect(win, PIX * XORIG, PIX * (L / 2.0 + YORIG), PIX, PIX);
  fillrect(win, PIX * XORIG, PIX * (L / 2.0 - YORIG), PIX, PIX);
  copylayer(win, 1, 0);
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
  int win;
  double freq = 1.0, kw = 0.5, dth = 0.0, atn = 0.0;
  double t = 0, dt = 0.05, cmap = IDL1_RGN_WHT_EXPONENTIAL;
  double run = 1, quit = 0;
  unsigned char *buffer = NULL;

  param_set prms = {
    &win, &freq, &kw, &dth, &atn, &t, &cmap
  };

  e_ctrl ctrls[] = {
    {"Freq.", &freq, 0.1, NULL, NULL},
    {"Wavenumber", &kw, 0.01, NULL, NULL},
    {"Phase diff.", &dth, 1, NULL, NULL},
    {"Attenuation", &atn, 0.001, NULL, NULL},
    {"Colormode", &cmap, 1, &change_cmap, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL}
  };

  int cwin;
  buffer = malloc(PIX * L * PIX * L * 4);
  if (buffer == NULL) {
    fprintf(stderr, "ERROR: malloc() failed.\n");
    return -1;
  }

  cwin = init_ctrls(ctrls, 7);
  win = gopen(PIX * L, PIX * L);
  newcolor(win, "red");
  layer(win, 0, 1);
  change_cmap(&prms);
  gsetnonblock(ENABLE);
  while (!quit) {
    int type, button, iscwin;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 7, wx, wy, iscwin, type, button);
    if (run) {
      t += dt;
      draw(win, freq, kw, dth, atn, t, cmap, prms.r, prms.g, prms.b,
	   buffer);
    }
    msleep(40);
  }
  gcloseall();

  if (buffer != NULL)
    free(buffer);
  return 0;
}
