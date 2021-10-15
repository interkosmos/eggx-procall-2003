#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "colormode.h"

typedef struct Vec3d {
  double x, y, z;
} vec3d;

double iprod(vec3d a, vec3d b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3d oprod(vec3d a, vec3d b)
{
  vec3d c;

  c.x = a.y * b.z - a.z * b.y;
  c.y = a.z * b.x - a.x * b.z;
  c.z = a.x * b.y - a.y * b.x;

  return c;
}

double norm(vec3d a)
{
  return sqrt(iprod(a, a));
}

double intensity(double u, double v, vec3d s)
{
  vec3d Lu, Lv, LL;

  Lu.x = cos(u) * cos(v);
  Lu.y = cos(u) * sin(v);
  Lu.z = -sin(u);
  Lv.x = -sin(u) * sin(v);
  Lv.y = sin(u) * cos(v);
  Lv.z = 0;
  LL = oprod(Lu, Lv);

  return pow(iprod(LL, s) / norm(LL) / norm(s), 1);
}

#define L    150
#define R    60
#define CMAX 128
#define PIX  2



typedef struct {
  int *win;
  double *cmap;
  vec3d *light;
  int r[CMAX], g[CMAX], b[CMAX];
} param_set;

void put_sphere(int win, double rad, double x, double y,
		vec3d light, int *r, int *g, int *b)
{
  int i, j, n;
  double rh, u, v;

  gclr(win);
  for (i = -rad; i <= rad; i++) {
    for (j = -rad; j <= rad; j++) {
      if ((rh = hypot(i, j)) <= rad) {
	u = acos(j / rad);
	v = acos(i / (sin(u) * rad));
	n = (int) CMAX *(intensity(u, v, light) * 0.45 + 0.45);
	newrgbcolor(win, r[n], g[n], b[n]);
	fillrect(win, PIX * (x + rad * sin(u) * cos(v)),
		 PIX * (y + rad * cos(u)), PIX, PIX);
      }
    }
  }
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
  put_sphere(*p->win, R, L / 2, L / 2, *p->light, p->r, p->g, p->b);
}

int main()
{
  int win, i;
  double quit = 0, cmap = DS9_GREEN;
  vec3d light = { 10, 20, 10 };

  param_set prms = { &win, &cmap, &light };

  e_ctrl ctrls[] = {
    {"Colormode", &cmap, 1, &change_cmap, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 2);

  srand48(time(NULL));
  win = gopen(PIX * L, PIX * L);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  gclr(win);
  layer(win, 0, 1);
  for (i = 0; i < CMAX; i++)
    makecolor((int) cmap, 0, CMAX - 1, i, &prms.r[i], &prms.g[i],
	      &prms.b[i]);

  put_sphere(win, R, L / 2, L / 2, light, prms.r, prms.g, prms.b);
  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetevent(&type, &button, &wx, &wy);
    if (iscwin == win && type == MotionNotify) {
      light.x = (wx - PIX * L / 2);
      light.z = (wy - PIX * L / 2);
      put_sphere(win, R, L / 2, L / 2, light, prms.r, prms.g, prms.b);
    } else {
      display_ctrls(cwin, ctrls, 2, wx, wy, iscwin, type, button);
    }
  }
  gcloseall();

  return 0;
}
