#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "e_3d.h"
#include "orange.h"
#include "silver.h"
#include "cyan.h"

#define N  3

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
#define G  1.183e-4		/* 万有引力定数　天文単位系 */
#define M  332950		/* 2つの太陽の質量 */
#define m0 1			/* 地球の質量 0番 */
#define SN 15000
#define SC 10

typedef struct {
  int *win;
  double *vy0;
  double *x0;
  double *th;
  double *phi;
  double *sc;
  double *t;
  double *keep;
  double *r;
} param_set;


vecN motion(double t, double *r)
{
  int i, j;
  vecN ret;
  double rc01, rc02, rc12;

  for (i = 0; i < N; i++)
    for (j = 0; j < 3; j++)
      ret.f[6 * i + j] = r[6 * i + j + 3];	/* r = dr/dt */

  rc01 =
      pow(pow((r[0] - r[6]), 2) + pow((r[1] - r[7]), 2) +
	  pow((r[2] - r[8]), 2), 1.5);
  rc02 =
      pow(pow((r[0] - r[12]), 2) + pow((r[1] - r[13]), 2) +
	  pow((r[2] - r[14]), 2), 1.5);
  rc12 =
      pow(pow((r[6] - r[12]), 2) + pow((r[7] - r[13]), 2) +
	  pow((r[8] - r[14]), 2), 1.5);

  for (j = 3; j < 6; j++) {
    ret.f[j] =
	G * M * ((r[j + 3] - r[j - 3]) / rc01 +
		 (r[j + 9] - r[j - 3]) / rc02);
  }
  for (j = 9; j < 12; j++) {
    ret.f[j] =
	G * (m0 * (r[j - 9] - r[j - 3]) / rc01 +
	     M * (r[j + 3] - r[j - 3]) / rc12);
  }
  for (j = 15; j < 18; j++) {
    ret.f[j] =
	G * (m0 * (r[j - 15] - r[j - 3]) / rc02 +
	     M * (r[j - 9] - r[j - 3]) / rc12);
  }

  return ret;
}

typedef struct {
  int n;
  double z;
} zdist;

int compz(const void *_p1, const void *_p2)
{
  zdist *p1 = (zdist *) _p1;
  zdist *p2 = (zdist *) _p2;

  if (p1->z < p2->z) {
    return -1;
  } else if (p1->z == p2->z) {
    return 0;
  } else
    return 1;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  double z0[3], z[3], K, U;
  unsigned char *image[3] = {
    Xpm_image_cyan4, Xpm_image_orange6, Xpm_image_silver6
  };
  int i, j, size[3] = { 4, 6, 6 };
  static double dtrx[3][SN], dtry[3][SN], dtrz[3][SN];
  static double trx[3][SN], try[3][SN], trz[3][SN];
  static int count = 0, interlace = 0;
  zdist planet[3];

  if (*p->keep == 0) {
    for (; count >= 0; count--) {
      trx[0][count] = try[0][count] = trz[0][count] = 0;
      trx[1][count] = try[1][count] = trz[1][count] = 0;
      trx[2][count] = try[2][count] = trz[2][count] = 0;
    }
    *p->keep = 1;
  }
  for (i = 0; i < 3; i++) {
    trx[i][count] = p->r[6 * i];
    try[i][count] = p->r[6 * i + 1];
    trz[i][count] = p->r[6 * i + 2];
  }

  for (i = 0; i < 3; i++)
    for (j = 0; j < count; j++) {
      dtrx[i][j] = (*p->sc) * trx[i][j];
      dtry[i][j] = (*p->sc) * try[i][j];
      dtrz[i][j] = (*p->sc) * trz[i][j];
    }


  K = (m0 * (pow(p->r[3], 2) + pow(p->r[4], 2) + pow(p->r[5], 2)) +
       M * (pow(p->r[9], 2) + pow(p->r[10], 2) + pow(p->r[11], 2)) +
       M * (pow(p->r[15], 2) + pow(p->r[16], 2) + pow(p->r[17], 2))) / 2;

  U = G * m0 * M / sqrt(pow((p->r[0] - p->r[6]), 2) +
			pow((p->r[1] - p->r[7]), 2)
			+ pow((p->r[2] - p->r[8]), 2))
      + G * m0 * M / sqrt(pow((p->r[0] - p->r[12]), 2) +
			  pow((p->r[1] - p->r[13]), 2)
			  + pow((p->r[2] - p->r[14]), 2))
      + G * M * M / sqrt(pow((p->r[6] - p->r[12]), 2) +
			 pow((p->r[7] - p->r[13]), 2)
			 + pow((p->r[8] - p->r[14]), 2));

  for (i = 0; i < 3; i++) {
    z[i] = z0[i] = _3dto2dz(p->r[6 * i], p->r[6 * i + 1], p->r[6 * i + 2]);
    planet[i].n = i;
    planet[i].z = z[i];
  }
  qsort(planet, 3, sizeof(zdist), compz);
  /* 星をw軸（紙面に垂直な軸）方向の奥から並べる */

  layer(*p->win, 0, 1);
  gclr(*p->win);
  newcolor(*p->win, "cyan4");
  drawlines3d(*p->win, dtrx[0], dtry[0], dtrz[0], count - 1);
  newcolor(*p->win, "lightsteelblue");
  drawlines3d(*p->win, dtrx[1], dtry[1], dtrz[1], count - 1);
  newcolor(*p->win, "wheat3");
  drawlines3d(*p->win, dtrx[2], dtry[2], dtrz[2], count - 1);
  double phi = torad(*p->phi);
  double th = torad(*p->th);
  for (j = 0; j < 3; j++) {
    i = planet[j].n;
    putimg24m3d(*p->win,
		(*p->sc) * p->r[i * 6] - size[i] * (-sin(phi) -
						    cos(th) * cos(phi)),
		(*p->sc) * p->r[i * 6 + 1] - size[i] * (cos(phi) -
							cos(th) *
							sin(phi)),
		(*p->sc) * p->r[i * 6 + 2] - size[i] * sin(th),
		2 * size[i], 2 * size[i], image[i]);
  }
  drawstr(*p->win, L / 2 - 280, L / 2 - 20, 14,
	  0, "e = %12.9e, t = %.4f [AU]", K - U, *p->t);
  copylayer(*p->win, 1, 0);
  if (count >= SN - 1)
    count = SN - 1;
  if (interlace++ % 3 == 0)
    count++;
}

void redraw(void *_prms)
{
  param_set *p = (param_set *) _prms;

  g3dsetangle(torad(*p->th), torad(*p->phi));
  draw(p);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;
  int i;

  for (i = 0; i < 6 * N; i++)
    p->r[i] = 0;
  p->r[0] = 1;			/* 地球の公転半径の初期位置 x=0 */
  p->r[4] = sqrt(M * m0 * G);	/* 地球の公転速度をy方向の初速度に与える */
  p->r[12] = *p->x0;
  p->r[13] = -20;
  p->r[16] = *p->vy0;
  *p->t = 0;
  *p->keep = 0;

  draw(p);
}

int main()
{
  int win, count = 0;
  double r[6 * N] = { 0 }, t = 0, h = 0.002, x0 = -1.22, vy0 = 7;
  double quit = 0, run = 1, keep = 1;
  double th = 0, phi = 0, zm = 20.0;
  param_set prms = { &win, &vy0, &x0, &th, &phi, &zm, &t, &keep, r };

  e_ctrl ctrls[] = {
    {"Theta", &th, 1, &redraw, &prms},	/* 結果を視点を変えてみたいので */
    {"Phi", &phi, -1, &redraw, &prms},	/* init しない                  */
    {"Zoom", &zm, 1, &redraw, &prms},
    {"X0", &x0, 0.1, &init, &prms},
    {"Vy0", &vy0, 0.1, &init, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Quit", &quit, 0, NULL, NULL},
  };
  int cwin = init_ctrls(ctrls, 7);


  win = gopen(L, L);
  window(win, -L / 2, -L / 2, L / 2, L / 2);
  gsetbgcolor(win, "#181830");
  layer(win, 0, 1);
  gsetnonblock(ENABLE);
  g3dsetoffset(-50, 50);
  g3dsetangle(torad(th), torad(phi));
  init(&prms);

  fflush(stdout);
  while (!quit) {
    int iscwin, type, button, i, iter;
    double wx, wy, r01, r02, r12, rmin;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 7, wx, wy, iscwin, type, button);
    if (run) {
      r01 =
	  pow((r[0] - r[6]), 2) + pow((r[1] - r[7]),
				      2) + pow((r[2] - r[8]), 2);
      r02 =
	  pow((r[0] - r[12]), 2) + pow((r[1] - r[13]),
				       2) + pow((r[2] - r[14]), 2);
      r12 =
	  pow((r[6] - r[12]), 2) + pow((r[7] - r[13]),
				       2) + pow((r[8] - r[14]), 2);

      rmin = r12 / 2.0;		/*　距離に応じてステップ h 調整 */
      if (r02 < rmin)
	rmin = r02;
      if (r01 < rmin)
	rmin = r01;
      iter = (int) pow(1 / rmin, 1.27);
      if (iter == 0)
	iter = 1;
      for (i = 0; i < iter; i++) {
	rk4fixN(motion, t, r, h / iter);
      }
      t += h;
      if (count++ % 3 == 0)
	draw(&prms);
    } 
    msleep(2);
  }
  gcloseall();
  return 0;
}
