#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eggx.h"
#include "e_3d.h"
#include "e_ctrls.h"


typedef struct {
  int *win;
  double *speed;
  double *mode;
  double *gear1_color;
  double *gear2_color;
  double *gear3_color;
  double *polar;
  double *azimuth;
  double *run;
  double *reset;
  double *quit;
} param_set;


static void
gear(int win, double x, double y, double z, double rotate,
     double inner_radius, double outer_radius, double width, int teeth,
     double tooth_depth)
{
  int i;
  double r0, r1, r2;
  double angle, da;
  double angle_rot;
  double ca0, ca1, ca2, ca3, ca4;
  double sa0, sa1, sa2, sa3, sa4;
  double *frx, *fry, *frz;

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2.0;
  r2 = outer_radius + tooth_depth / 2.0;

  da = 2.0 * M_PI / teeth / 4.0;

  frx = malloc(10 * (teeth + 1) * sizeof(double));
  fry = malloc(10 * (teeth + 1) * sizeof(double));
  frz = malloc(10 * (teeth + 1) * sizeof(double));

  /* draw gear's face */
  for (i = 0; i <= 8 * teeth; i += 8) {
    angle = i * 2.0 * M_PI / (8 * teeth);
    angle_rot = angle + rotate;
    ca0 = cos(angle_rot);
    sa0 = sin(angle_rot);
    ca1 = cos(angle_rot + da);
    sa1 = sin(angle_rot + da);
    ca2 = cos(angle_rot + 2 * da);
    sa2 = sin(angle_rot + 2 * da);
    ca3 = cos(angle_rot + 3 * da);
    sa3 = sin(angle_rot + 3 * da);

    frx[i] = x + r1 * ca0;
    fry[i] = y + r1 * sa0;
    frz[i] = z - width * 0.5;
    frx[i + 1] = x + r1 * ca0;
    fry[i + 1] = y + r1 * sa0;
    frz[i + 1] = z + width * 0.5;
    frx[i + 2] = x + r2 * ca1;
    fry[i + 2] = y + r2 * sa1;
    frz[i + 2] = z + width * 0.5;
    frx[i + 3] = x + r2 * ca1;
    fry[i + 3] = y + r2 * sa1;
    frz[i + 3] = z - width * 0.5;
    frx[i + 4] = x + r2 * ca2;
    fry[i + 4] = y + r2 * sa2;
    frz[i + 4] = z - width * 0.5;
    frx[i + 5] = x + r2 * ca2;
    fry[i + 5] = y + r2 * sa2;
    frz[i + 5] = z + width * 0.5;
    frx[i + 6] = x + r1 * ca3;
    fry[i + 6] = y + r1 * sa3;
    frz[i + 6] = z + width * 0.5;
    frx[i + 7] = x + r1 * ca3;
    fry[i + 7] = y + r1 * sa3;
    frz[i + 7] = z - width * 0.5;
  }
  drawlines3d(win, frx, fry, frz, 8 * (teeth) + 1);


  for (i = 0; i <= 8 * teeth; i += 8) {
    angle = i * 2.0 * M_PI / (8 * teeth);
    angle_rot = angle + rotate;
    ca0 = cos(angle_rot);
    sa0 = sin(angle_rot);
    ca1 = cos(angle_rot + da);
    sa1 = sin(angle_rot + da);
    ca2 = cos(angle_rot + 2 * da);
    sa2 = sin(angle_rot + 2 * da);
    ca3 = cos(angle_rot + 3 * da);
    sa3 = sin(angle_rot + 3 * da);
    ca4 = cos(angle_rot + 4 * da);
    sa4 = sin(angle_rot + 4 * da);

    frx[i] = x + r1 * ca0;
    fry[i] = y + r1 * sa0;
    frz[i] = z - width * 0.5;
    frx[i + 1] = x + r2 * ca1;
    fry[i + 1] = y + r2 * sa1;
    frz[i + 1] = z - width * 0.5;
    frx[i + 2] = x + r2 * ca1;
    fry[i + 2] = y + r2 * sa1;
    frz[i + 2] = z + width * 0.5;
    frx[i + 3] = x + r2 * ca2;
    fry[i + 3] = y + r2 * sa2;
    frz[i + 3] = z + width * 0.5;
    frx[i + 4] = x + r2 * ca2;
    fry[i + 4] = y + r2 * sa2;
    frz[i + 4] = z - width * 0.5;
    frx[i + 5] = x + r1 * ca3;
    fry[i + 5] = y + r1 * sa3;
    frz[i + 5] = z - width * 0.5;
    frx[i + 6] = x + r1 * ca3;
    fry[i + 6] = y + r1 * sa3;
    frz[i + 6] = z + width * 0.5;
    frx[i + 7] = x + r1 * ca4;
    fry[i + 7] = y + r1 * sa4;
    frz[i + 7] = z + width * 0.5;
  }
  drawlines3d(win, frx, fry, frz, 8 * (teeth) + 1);


  /* draw inner cylinder */
  for (i = 0; i <= 10 * teeth; i += 5) {
    angle = i * 2.0 * M_PI / (10 * teeth);
    ca0 = cos(angle);
    sa0 = sin(angle);
    ca2 = cos(angle + 2 * da);
    sa2 = sin(angle + 2 * da);

    frx[i] = x + r0 * ca0;
    fry[i] = y + r0 * sa0;
    frz[i] = z - width * 0.5;
    frx[i + 1] = x + r0 * ca0;
    fry[i + 1] = y + r0 * sa0;
    frz[i + 1] = z + width * 0.5;
    frx[i + 2] = x + r0 * ca2;
    fry[i + 2] = y + r0 * sa2;
    frz[i + 2] = z + width * 0.5;
    frx[i + 3] = x + r0 * ca2;
    fry[i + 3] = y + r0 * sa2;
    frz[i + 3] = z - width * 0.5;
    frx[i + 4] = x + r0 * ca0;
    fry[i + 4] = y + r0 * sa0;
    frz[i + 4] = z - width * 0.5;
  }
  drawlines3d(win, frx, fry, frz, 10 * (teeth) + 1);

  free(frx);
  free(fry);
  free(frz);
}


void draw(void *_prms)
{
  param_set *p = (param_set *) _prms;
  static double theta=0, phi=0;
  double  sp = *p->speed;

  int g1c = ((int)*p->gear1_color) % 16 + 1;
  int g2c = ((int)*p->gear2_color) % 16 + 1;
  int g3c = ((int)*p->gear3_color) % 16 + 1;
  
  gclr(*p->win);
  g3dsetangle(0.75*M_PI*cos(*p->polar), *p->azimuth); 
  if ((int)*p->mode) {
      *p->polar = fmod(*p->polar + 0.0073, 2*M_PI);
      *p->azimuth = fmod(*p->azimuth + 0.0043, 2*M_PI);
  }
  newpen(*p->win, g1c);
  gear(*p->win, -3.0, -2.0, 0.0, 2 * phi, 1.0, 4.0, 2.0, 20, 0.7);
  newpen(*p->win, g2c);
  gear(*p->win, 3.1, -2.0, 0.0, -4.0 * phi - 9.0, 0.5, 2.0, 2.0, 10, 0.7);
  newpen(*p->win, g3c);
  gear(*p->win, -3.0, 4.2, 0.0, -4.0 * phi - 25.0, 1.0, 2.0, 1.0, 10, 0.7);
  copylayer(*p->win, 1, 0);
  phi = fmod(phi + sp * M_PI / 360, 2 * M_PI);
  theta = fmod(theta + sp * M_PI / 323, 2 * M_PI);
}

void init(void *_prms)
{
  param_set *p = (param_set *) _prms;

  *p->quit = 0;
  *p->gear1_color = (int)*p->gear1_color % 16;
  *p->gear2_color = (int)*p->gear2_color % 16;
  *p->gear3_color = (int)*p->gear3_color % 16;
  *p->mode = (int)*p->mode ? 1 : 0;
  draw(_prms);
}

void _reset(void *_prms)
{
  param_set *p = (param_set *) _prms;

  *p->polar = M_PI/2;
  *p->azimuth = 0;
  *p->mode = 0;
  *p->reset = 0;

  draw(_prms);
}


int main()
{
  int win;
  double run = 1, quit = 0, speed = 1, mode = 0, reset = 0;
  double polar = M_PI/2, azimuth = 0;
  double g1c = 1, g2c = 2, g3c = 3;
  
  param_set prms ={&win, &speed, &mode, &g1c, &g2c, &g3c, &polar, 
                   &azimuth, &run, &reset, &quit};
  e_ctrl ctrls[] = {
    {"Speed", &speed, 0.1, &init, &prms},
    {"_Waving", &mode, 0, &init, &prms},
    {"Gear1 Color", &g1c, 1, &init, &prms},
    {"Gear2 Color", &g2c, 1, &init, &prms},
    {"Gear3 Color", &g3c, 1, &init, &prms},
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &_reset, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 8);

  gsetinitialbgcolor(ECTRL_BGCOLOR);
  win = gopen(300, 300);
  window(win, -8, -8, 8, 8);
  newlinewidth(win, 0);
  layer(win, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);


  while (!quit) {
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 8, wx, wy, iscwin, type, button);
    if(run) draw(&prms); 
    msleep(10);
  } 
  gcloseall();
  return 0;
}
