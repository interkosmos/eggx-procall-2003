/*
エネルギーを交換する多粒子系では，速度分布がMaxWell-Boltzmann分布になるという
統計力学の基本原理を体感するシミュレーション．
一様分布から出発したN個の分子の一部で弾性衝突行い，速度分布を表示するだけ．
 */

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "e_3d.h"
#include "stone.h"

#define N     20000

#define Dmax  50
#define L     300
#define A     5      /* 一辺の半分の長さ */

typedef struct vec3d {
    double x;
    double y;
    double z;
} Vec3d;

Vec3d pos[N], v[N];     /* これをparam_setに含めると煩雑過ぎるので */
int   vdist[Dmax];      /* 大域変数とする                          */

typedef struct _param_set{
  int    *win;
  int    *win2;
  double *dt;
  double *Ndisp;
  double *Collision;
  double *th;
  double *phi;
  double *run;
  double *reset;
} param_set;

void getstatis()
{
  int i, j;
  double vi;

  for (i=0; i< Dmax; i++) vdist[i]=0;
  for (i=0; i< N; i++){
    vi = sqrt(pow(v[i].x,2)+pow(v[i].y,2)+pow(v[i].z,2));
    j = (int)(floor(Dmax*vi*20));
    if (j >= Dmax) continue;
    vdist[j]++;
  }
}

void step(void *_prms)
{
  param_set *p = (param_set *)_prms;
  int i, j, k;
  double vr, vmx, vmy, vmz, cost, sint, phi, vrax, vray, vraz;

  for (i=0; i< N ; i++) {
    pos[i].x += v[i].x*(*p->dt);
    pos[i].y += v[i].y*(*p->dt);
    pos[i].z += v[i].z*(*p->dt);
    if (pos[i].x > +A && v[i].x > 0) v[i].x=-fabs(v[i].x);
    if (pos[i].x < -A && v[i].x < 0) v[i].x=+fabs(v[i].x);
    if (pos[i].y > +A && v[i].y > 0) v[i].y=-fabs(v[i].y);
    if (pos[i].y < -A && v[i].y < 0) v[i].y=+fabs(v[i].y);
    if (pos[i].z > +A && v[i].z > 0) v[i].z=-fabs(v[i].z);
    if (pos[i].z < -A && v[i].z < 0) v[i].z=+fabs(v[i].z);
  }

  // 剛体球衝突対の選択とエネルギー交換
  for (i = 0; i < *p->Collision; i++){
    k = j =(int)(N*drand48());
    while ( k == j ){
      k = (int)(N*drand48());
    };
    vmx = (v[j].x + v[k].x)/2.0;
    vmy = (v[j].y + v[k].y)/2.0;
    vmz = (v[j].z + v[k].z)/2.0;
    vr = sqrt( pow((v[j].x - v[k].x),2)
              + pow((v[j].y - v[k].y),2)
	       + pow((v[j].z - v[k].z),2) );
    phi = 2*M_PI*drand48();
    cost = 2.0*drand48()-1.0;
    sint = sqrt(1-cost*cost);
    vrax = vr*sint*cos(phi);
    vray = vr*sint*sin(phi);
    vraz = vr*cost;
    v[j].x = vmx + vrax/2.0;
    v[j].y = vmz + vray/2.0;
    v[j].z = vmz + vraz/2.0;
    v[k].x = vmx - vrax/2.0;
    v[k].y = vmz - vray/2.0;
    v[k].z = vmz - vraz/2.0;
  }
 
  // 分布の取得
  getstatis();
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
  param_set *p = (param_set *)_prms;
  int i,j;
  double sc = L/(3*A), psc = L/Dmax, vdistmax;
  double th, phi, *z;
  zdist *particle;

  particle = malloc(*p->Ndisp*sizeof(zdist));
  z = malloc(*p->Ndisp*sizeof(double));

  for (i = 0; i < *p->Ndisp; i++) {
    z[i] = _3dto2dz(pos[i].x, pos[i].y, pos[i].z);
    particle[i].n = i;
    particle[i].z = z[i];
  }
  /* 粒子が多いと奥行きの間違いには気づかないので不要かも */
  qsort(particle, *p->Ndisp, sizeof(zdist), compz);

  th = torad(*p->th);
  phi = torad(*p->phi);
  g3dsetangle(th, phi);
  gclr(*p->win);
  for (j = 0; j < *p->Ndisp; j++) {
    i = particle[j].n;
    putimg24m3d(*p->win, 
                sc*(pos[i].x)-4* (-sin(phi) - cos(th) * cos(phi)),
                sc*(pos[i].y)-4* (cos(phi) - cos(th)), 
                sc*(pos[i].z)-4* sin(th), 
                8, 8, Xpm_image_stone4);
  }
  newcolor(*p->win, "lightsteelblue");
  drawline3d(*p->win, -sc*A, -sc*A, -sc*A, -sc*A, -sc*A, sc*A);
  drawline3d(*p->win, -sc*A, -sc*A, -sc*A, -sc*A, sc*A, -sc*A);
  drawline3d(*p->win, -sc*A, -sc*A, -sc*A, sc*A, -sc*A, -sc*A);
  drawline3d(*p->win, sc*A, sc*A, sc*A, sc*A, sc*A, -sc*A);
  drawline3d(*p->win, sc*A, sc*A, sc*A, sc*A, -sc*A, sc*A);
  drawline3d(*p->win, sc*A, sc*A, sc*A, -sc*A, sc*A, sc*A);
  
  drawline3d(*p->win, sc*A, -sc*A, sc*A, sc*A, -sc*A, -sc*A);
  drawline3d(*p->win, sc*A, -sc*A, sc*A, -sc*A, -sc*A, sc*A);
  drawline3d(*p->win, -sc*A, sc*A, sc*A, -sc*A, sc*A, -sc*A);
  drawline3d(*p->win, -sc*A, sc*A, sc*A, -sc*A, -sc*A, sc*A);
  drawline3d(*p->win, sc*A, sc*A, -sc*A, sc*A, -sc*A, -sc*A);
  drawline3d(*p->win, sc*A, sc*A, -sc*A, -sc*A, sc*A, -sc*A);

  copylayer(*p->win, 1, 0);

  gclr(*p->win2);
  newcolor(*p->win2, "steelblue4");
  vdistmax = vdist[0];
  for (i = 0; i < Dmax; i++) { 
    if (vdistmax < vdist[i]) vdistmax = vdist[i]; 
  }
  for (i = 0; i < Dmax; i++) {
    fillrect(*p->win2, psc*i, 0, psc, vdist[i]/vdistmax*L/2);
  }
  newcolor(*p->win2, "steelblue");
  for (i = 0; i < Dmax; i++) {
    drawrect(*p->win2, psc*i, 0, psc, vdist[i]/vdistmax*L/2);
  }
  copylayer(*p->win2, 1, 0);
  free(z);
  free(particle);
}

void init(void *_prms)
{
  param_set *p = (param_set *)_prms;
  int i;
  double vv;

  for (i = 0; i < N; i++){
    vv = 4e-2 * 2*(drand48()-0.5);
    v[i].x = vv * 2*(drand48()-0.5);
    v[i].y = sqrt(vv*vv - v[i].x*v[i].x)*(1-2*rint(drand48()));
    v[i].z = 0;
  }
  for (i=0; i < N; i++){
    pos[i].x = (2*A)*(drand48()-0.5);
    pos[i].y = (2*A)*(drand48()-0.5);
    pos[i].z = (2*A)*(drand48()-0.5);
  }
  getstatis();
  *p->run = 0;
  *p->reset = 0;
  draw(p);
}  

int main(int argc, char *argv[])
{
   int win, win2;
   double run = 1, reset = 0, quit = 0, th = 70, phi = 20;;
   double Ndisp = 500.0, Collision = 80, dt = 3;   

   param_set prms = {
     &win, &win2, &dt, &Ndisp, &Collision, &th, &phi, &run, &reset
   };

  e_ctrl ctrls[] = {
    {"dt", &dt, 0.1, NULL, NULL},
    {"Number disp.", &Ndisp, 100, NULL, NULL},
    {"Collisions ", &Collision, 10, NULL, NULL},
    {"Theta ", &th, 10, NULL, NULL},
    {"Phi ", &phi, -10, NULL, NULL},
    {"_Run", &run, 0, NULL, NULL},
    {"_Reset", &reset, 0, &init, &prms},
    {"_Quit", &quit, 0, NULL, NULL}
  };
  int cwin;
  cwin = init_ctrls(ctrls, 8);

  srand48(time(NULL));
  win = gopen(L, L);
  window(win, -L/2, -L/2, L/2-1, L/2-1);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  layer(win, 0, 1);
  win2 = gopen(L, L/2);
  window(win2, 0, 0, L-1, 1.1*L/2);
  gsetbgcolor(win2, ECTRL_FGCOLOR);
  layer(win2, 0, 1);
  init(&prms);
  gsetnonblock(ENABLE);
  run = 1;

  while (!quit){
    int iscwin, type, button;
    double wx, wy;
    iscwin = ggetxpress(&type, &button, &wx, &wy);
    display_ctrls(cwin, ctrls, 8, wx, wy, iscwin, type, button);
    if (run) {
      step(&prms);
      draw(&prms);
    } 
    msleep(10);
  }
  gcloseall();
  return 0;
}
