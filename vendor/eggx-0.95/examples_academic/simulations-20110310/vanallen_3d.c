#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "e_3d.h"
#include "gold.h"
#include "rk4fix.h"

#define L   300
#define SC  70

#define QMC 9260		/* q/(mc) */
#define H0  0.309		/* [Gauss/R^3] R:ÃÏµå¤ÎÈ¾·Â */
#define N   50000

vec6 motion(double t, double *r)
{
  vec6 ret;
  double Hx, Hy, Hz, R5;

  R5 = pow(r[0]*r[0] + r[2]*r[2] + r[4]*r[4], 2.5);
  Hx = H0 * 3 * r[0] * r[4] / R5;
  Hy = H0 * 3 * r[2] * r[4] / R5;
  Hz = H0 * (2 * r[4] * r[4] - r[0] * r[0] - r[2] * r[2])/R5;

  ret.f[0] = r[1];
  ret.f[2] = r[3];
  ret.f[4] = r[5];
  ret.f[1] = QMC * (r[3] * Hz - Hy * r[5]);
  ret.f[3] = QMC * (r[5] * Hx - Hz * r[1]);
  ret.f[5] = QMC * (r[1] * Hy - Hx * r[3]);

  return ret;
}

typedef struct {
  int *win;
  int *win2;
  double *V0;
  double *th0;
  double *phi0;
  double *r0;
  double *th;
  double *phi;
  double *t;
  double *run;
  double *reset;
  double *keep;
  int *icount;
  double *r;
} param_set;


void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  double tr2y[400]={0}, tr2z[400]={0};
  static double trx[N], try[N], trz[N];
  static int count = 0;
  
  if (*p->keep == 0) {
    count = 0;
    *p->keep = 1;
  }
  trx[count] = SC * p->r[0];
  try[count] = SC * p->r[2];
  trz[count] = SC * p->r[4];

  layer(*p->win, 0, 1);
  gclr(*p->win);
  copylayer(*p->win, 2, 1);
  double th = torad(*p->th);
  double phi = torad(*p->phi);
  newcolor(*p->win, "gold3");
  drawlines3d(*p->win, trx, try, trz, count-1);
  newcolor(*p->win, "lightsteelblue");
  drawstr(*p->win, L / 2 - 100, L / 2 - 20, 14, 0, "t = %.5f", *p->t);
  putimg24m3d(*p->win,
	      SC * (p->r[0]) - 4 * (-sin(phi) - cos(th) * cos(phi)),
	      SC * (p->r[2]) - 4 * (cos(phi) - cos(th) * sin(phi)),
	      SC * (p->r[4]) - 4 * (sin(th)), 8, 8, Xpm_image_gold4);
  copylayer(*p->win, 1, 0);
  if (count >= N - 1)
    count = N - 1;
  count++;

  int i, s = 0;
  double sum = 0, mtr2y = 0;
  for (i = 0; i < 400; i++) 
   if ( count - 400 + i > 0 ) {
     s++;
     sum += try[count-400+i];
   }
  mtr2y = sum / s;

  for (i = 399; i >= 0 ; i--) {
    if (count - 400 + i > 0) {
      tr2y[i] = 10*(try[count-400+i] - mtr2y);
      tr2z[i] = 10*(trz[count-400+i] - trz[count-1]);
    } else {
      tr2y[i] = tr2y[i+1];
      tr2z[i] = tr2z[i+1];
    }
  }

  gclr(*p->win2);
  if (count < 8000) {
    drawlines(*p->win2, tr2y, tr2z, 400);
    putimg24m(*p->win2, tr2y[399]-8, -8, 16, 16, Xpm_image_gold8); 
    copylayer(*p->win2, 1, 0);
  }
}

void redraw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  double longx[12][72], longy[12][72], longz[12][72];
  double latix[5][72], latiy[5][72], latiz[5][72];
  int i, j;
  double th, phi;

  g3dsetangle(torad(*p->th), torad(*p->phi));
  for (i = 0; i < 12; i++) {
    phi = i * 2 * M_PI / 12.0;
    for (j = 0; j < 72; j++) {
      th = j * M_PI / 72.0;
      longx[i][j] = SC * sin(th) * cos(phi);
      longy[i][j] = SC * sin(th) * sin(phi);
      longz[i][j] = SC * cos(th);
    }
  }
  for (i = 0; i < 5; i++) {
    th = (i + 1) * M_PI / 6.0;
    for (j = 0; j < 72; j++) {
      phi = j * 2 * M_PI / 72.0;
      latix[i][j] = SC * sin(th) * cos(phi);
      latiy[i][j] = SC * sin(th) * sin(phi);
      latiz[i][j] = SC * cos(th);
    }
  }
  layer(*p->win, 0, 2);
  gclr(*p->win);
  newcolor(*p->win, ECTRL_BGCOLOR);
  for (i = 0; i < 12; i++)
    for (j = 0; j<71; j++){ /* µå¤Î±£Àþ½èÍý¤Ï´ÊÃ± */
      if (_3dto2dz(longx[i][j+1], longy[i][j+1], longz[i][j+1]) > 0 ){
         drawline3d(*p->win, longx[i][j], longy[i][j], longz[i][j],
		    longx[i][j+1], longy[i][j+1], longz[i][j+1]);
      }
    }
  for (i = 0; i < 5; i++)
    for (j = 0; j<72; j++){
      if (_3dto2dz(latix[i][j+1], latiy[i][j+1], latiz[i][j+1]) > 0 ){
         drawline3d(*p->win, latix[i][j], latiy[i][j], latiz[i][j],
		    latix[i][(j+1)%72], latiy[i][(j+1)%72], latiz[i][(j+1)%72]);
      }
    }
  drawcirc(*p->win, 0, 0, SC * 1, SC * 1);
  newcolor(*p->win, "steelblue4");
  fillcirc(*p->win, _3dto2dx(0, 0, SC * (-1)), _3dto2dy(0, 0, SC * (-1)),
	   3, 3);
  newcolor(*p->win, "red4");
  fillcirc(*p->win, _3dto2dx(0, 0, SC * (1)), _3dto2dy(0, 0, SC * (1)), 3,
	   3);
  newlinewidth(*p->win,2);
  drawarrow3d(*p->win, SC * (*p->r0), 0, 0,
	      SC * (*p->r0 + sin(torad(*p->th0)) * cos(torad(*p->phi0))),
	      SC * (sin(torad(*p->th0)) * sin(torad(*p->phi0))),
	      SC * (cos(torad(*p->th0))), 10, 6, 12);
  newlinewidth(*p->win,1);
  draw(p);
}


void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  p->r[2] = p->r[4] = 0;
  p->r[0] = *p->r0;
  p->r[1] = (*p->V0) * sin(torad(*p->th0)) * cos(torad(*p->phi0));
  p->r[3] = (*p->V0) * sin(torad(*p->th0)) * sin(torad(*p->phi0));
  p->r[5] = (*p->V0) * cos(torad(*p->th0));

  *p->t = 0;
  *p->run = 0;
  *p->reset = 0;
  *p->keep = 0;
  *p->icount = 0;
  gclr(*p->win);
  g3dsetangle(torad(60), torad(30));
  redraw(p);
}

int main(int argc, char **argv)
{
  int win, win2, icount = 0, divc = 5;
  double th = 70, phi = 30;
  double run = 0, quit = 0, reset = 0, keep = 1;
  double t = 0, h = 0.00001, V0 = 11.6;
  double th0 = 45, r0 = 2, phi0 = 90;
  double r[6];

  param_set prms = { &win, &win2, &V0, &th0, &phi0, &r0, &th, &phi,
    &t, &run, &reset, &keep, &icount, r
  };

  e_ctrl ctrls[] = {
    {"V0", &V0, 1, &init, &prms},
    {"Th0", &th0, 1, &init, &prms},
    {"Phi0", &phi0, 1, &init, &prms},
    {"Init distance", &r0, 0.1, &init, &prms},
    {"Theta", &th, 1, &redraw, &prms},
    {"Phi", &phi, -1, &redraw, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &init, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 9);

  win = gopen(L, L);
  win2 = gopen(L/2, L);
  window(win, -L / 2, -L / 2, L / 2, L / 2);
  window(win2, -L / 4, -L/2, L / 4, L/2);
  layer(win, 0, 1);
  layer(win2, 0, 1);
  newlinewidth(win2,2);
  newcolor(win2, "gold3");
  winname(win2, "Zoom: x10");
  gsetbgcolor(win, "#181830");
  gsetbgcolor(win2, "#181830");
  gsetnonblock(ENABLE);
  init(&prms);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 9, wx, wy, iscwin, type, button);
    if (icount == 0)
      divc = 5;
    if (run) {
      rk4fixv6(motion, t, r, h);
      t += h;
      if (icount % divc == 0) {
	draw(&prms);
	msleep(5);
      }
      icount++;
      if (icount > 40000)
	divc = 100;
    }
  }
  gcloseall();
  return 0;
}
