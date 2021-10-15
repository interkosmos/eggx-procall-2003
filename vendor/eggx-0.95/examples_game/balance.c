#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "eggx.h"
#include "stone30.h"
#include "hand.h"

// begin{3D 4-th Runge-Kutta} 
typedef struct {
  double f[6];
} vec6;

void rk4fixv6(vec6 f(), 
              double t, double *r, double h)
{
  int i, n;
  double ts, rs[6], k[4][6];
  double c[4] = { 1, 0.5, 0.5, 1 };
  vec6 diff;

  for (n = 0; n < 6; n++) rs[n] = r[n];
  ts = t;
  for (i = 0; i < 4; i++) {
    if (i > 0) {
      for (n = 0; n < 6; n++)rs[n] = r[n] + k[i - 1][n] * c[i];
      ts = t + c[i]*h;
    }
    diff = f(ts, rs);
    for (n = 0; n < 6; n++) k[i][n] = h * diff.f[n];
  }

  for (n = 0; n < 6; n++) 
    for (i = 0; i < 4; i++) 
      r[n] += k[i][n] / c[i] / 6;
}
// end{3D 4th Runge-Kutta}

#define WD 400
#define HT 400
#define SC 250

#define G  9.8
#define L  1.3
#define M  0.5
#define m  2.0
#define SUCCESS 5 /* 好きな目標値設定：結構難しい */

double  f = 0.0;  /* 支点を動かす力 */

vec6 mov(double t, double r[6])
{
  vec6 ret;

  ret.f[0] = r[1];  /* dq/dt */
  ret.f[1] = ( - f*cos(r[0])  + G*(m*M)*sin(r[0]) 
               - m*L*sin(r[0])*cos(r[0])*r[1]*r[1] )
				/ ( L*(M+m*sin(r[0])*sin(r[0])) );
  ret.f[2] = r[3]; /* dx/dt */
  ret.f[3] = ( f - m*sin(r[0])*cos(r[0])*G + m*L*sin(r[0])*r[1]*r[1] )
               		/ (M+m*sin(r[0])*sin(r[0]));
  return ret;
}

int main()
{
  double  r[6] = {0}, px, py, r0 = 0.001, r1, t = 0, dt = 0.02;
  int win, button, type, i, j, sflag;
  double wx, wy, wx1 = 0;
  float rbtx[3] = {190, 2, 2}, bty[3] = {5, 25, -15};
  float lbtx[3] = {-190, -2, -2};
  char *check[] = {"ivory","#336699"};

  win = gopen(WD, HT);
  coordinate(win, WD/2,20, 0,0, 1,1);
  gsetfontset(win,"-*-helvetica-bold-r-*-*-%d-*-*-*-*-*-*-*",34);
  gsetbgcolor(win, "gray10");
  layer(win, 0, 2);
  for (i = 0; i < 7; i++) {
    for (j = 1; j < 40; j++) {
      newcolor(win, check[(i+j)%2]); 
      fillrect(win, (j-20)*50, i*50+30, 50, 50);
    }  
  }
  layer(win, 0, 1);
  gsetnonblock(ENABLE);
  srand48(time(NULL));
  r[0] = 2*r0*drand48() - r0;

  while(1) {
    rk4fixv6(mov, t, r, dt);
    t += dt;
    r1 = SC*r[2]/10; /* 棒の支点の移動速度で背景を動かす */
    layer(win, 0, 2);
    gscroll(win, r1, 0, 0);
    copylayer(win, 2, 1);
    layer(win, 0, 1);
    newcolor(win,"Orange");
    drawstr(win, -WD/2+10, HT-50, FONTSET, 0, "%.4f", t);
    newlinewidth(win,1);
    newcolor(win,"red4");
    fillpoly(win, rbtx, bty, 3, 0);
    fillpoly(win, lbtx, bty, 3, 0);
    /* */
    coordinate(win, WD/2,20, -wx1,0, 1,1);
    newcolor(win, "tan4");
    newlinewidth(win,6); 
    putimg24m(win, -25, -20, 50,55, Xpm_image_hand);
    px = SC*L*sin(r[0]);
    py = SC*L*cos(r[0]); 
    moveto(win, 0, 0);
    lineto(win, px ,py);
    putimg24m(win, px-15, py-15, 30, 30, Xpm_image_stone30); 
    if ( fabs(r[0]) > M_PI/4 ) { sflag = 0; break;}
    if ( t > SUCCESS ){ sflag = 1; break;} 
    copylayer(win, 1, 0);
    msleep(40);
    /* */
    coordinate(win, WD/2,20, 0,0, 1,1);
    while ( 0 <= ggetevent(&type, &button, &wx, &wy) ) {
	if ( type == MotionNotify && button==1 ) wx1 = wx;
    }
    f = (wx1/10.0);
    if (f > 0.1) f -= 0.1;
    else if ( f < -0.1)f +=0.1 ;
 }
 coordinate(win, WD/2,20, 0,0, 1,1);
 gsetnonblock(DISABLE);
 newcolor(win,"Orange");
 if ( sflag == 1 ) {
   drawstr(win, -140, HT/2, FONTSET, 0, "Congraturations!");
 } else {
   drawstr(win, -70, HT/2, FONTSET, 0, "Too bad!");
 }
 copylayer(win, 1, 0);
 ggetch();
 gclose(win);

 return 0;
}
