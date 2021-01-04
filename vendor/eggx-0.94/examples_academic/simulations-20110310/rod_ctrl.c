#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rk4fix.h"
#include "eggx.h"
#include "e_ctrls.h"

#define G  9.8
#define L  5
#define RH 10000
#define WD 240
#define HT 240
#define MG 20
#define SC 20

typedef struct {
  int *win;
  double *angle;
  double *t;
  double *run;
  double *reset;
  double r[6];
} param_set;


static vec6 rod(double t, double *r)
{ 
  vec6 ret;
  double s = sin(r[0]);

  ret.f[0] = r[3];
  ret.f[3] = (G/L*s - sin(2*r[0])*r[3]*r[3]/2)/(s*s + 1.0/3) ;
  ret.f[1] = ret.f[2] = ret.f[4] = ret.f[5] = 0;

  return ret;
}

void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  double x, y ; 
  double rad ;
  char tstr[20];

  rad = p->r[0];

  y = L*cos(rad)*SC; 
  x = L*sin(rad)*SC;
  gclr(*p->win);
  sprintf(tstr, "t = %.4f", *p->t);
  newrgbcolor(*p->win, 204,153, 0);
  fillrect(*p->win, -WD/2, -MG, WD, 18);
  newpen(*p->win, 1);
  moveto(*p->win, -x, 0);
  lineto(*p->win, x, 2*y);
  drawstr(*p->win, -WD/2+MG, HT-2*MG, 14, 0, tstr);
  copylayer(*p->win, 1, 0);
}

void post(void *_prms)
{
  param_set *p = (param_set *) _prms;
  char omega[20];
  double al = (*p->angle)*M_PI/180;

  newpen(*p->win, 1);
  sprintf(omega, "w = %.4f, %.4f\n", p->r[3], sqrt(3*G*cos(al)/(2*L)));
  drawstr(*p->win, -WD/2+MG, HT-3*MG, 14, 0, omega);
  copylayer(*p->win, 1, 0);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  *p->run = 0;
  *p->t = 0;
  *p->reset = 0;
  p->r[0] = (*p->angle)*M_PI/180;
  p->r[3] = 0;
  p->r[1] = p->r[2] = p->r[4] = p->r[5] = 0;
  draw(p);
}


int main(int argc, char **argv)
{
  int win;
  double angle = 10, run = 0, reset = 0, quit = 0;
  double t = 0, h = 0.0001;
  param_set prms = {&win, &angle, &t, &run, &reset};

  e_ctrl ctrls[] = {
    {"Initial Angle", &angle, 1, &init, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &init, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 4);

  gsetinitialbgcolor(ECTRL_BGCOLOR);
  win = gopen(WD, HT);
  newlinewidth(win, 6);
  window(win, -WD/2, -MG, WD/2, HT-MG);
  layer(win, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);

  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 4, wx, wy, iscwin, type, button);
    if (run) {
      rk4fixv6(rod, t, prms.r, h);
      t += h;
      if ((int)(t*RH) % (RH/100) == 0 ) {
        draw(&prms); msleep(10);
      }
      if (prms.r[0] >= M_PI/2) {
         run = 0;
         post(&prms);
      }
    }
  }
  gcloseall();

  return 0;
}

  


    

