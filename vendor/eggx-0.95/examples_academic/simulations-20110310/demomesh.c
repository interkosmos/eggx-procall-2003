#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_3d.h"

#define M 72
#define N 24
#define L 300

int main()
{
  int win, i, j;
  double data[M][N][3], u, v;

  for(i = 0; i < M; i++) {
    u = 3.1*M_PI/(M-1) * i;
    for(j = 0; j < N; j++) {
      v = 2*M_PI/(N-1) *j - M_PI;
       data[i][j][0] = u*(1+0.5*cos(v))*cos(u);
       data[i][j][1] = 0.5*u*sin(v);
       data[i][j][2] = -u*(1+0.5*cos(v))*sin(u);
    }
  }
  
  win = gopen(L, L);
  window(win, -L/2, -L/2, L/2, L/2); 
  layer(win, 0, 1);
  gsetbgcolor(win, "steelblue4");
  newcolor(win, "red");
  for (i = 0; i < 32; i++){
    g3dsetangle(7*M_PI/18, i*M_PI/18);
    g3dmesh(win, (double *)data, M, N, 10, 10, 10, 0x99, 0x66, 0x00, 1, 
            DS9_HEAT, -15, 7);
    copylayer(win, 1, 0);
    msleep(10);
  }
  msleep(1000);
  for (i = 0; i < 60; i++){
    g3dsetangle(7*M_PI/18 + i*M_PI/30, 31*M_PI/18);
    g3dmesh(win, (double *)data, M, N, 10, 10, 10, i*2+60, i*2, 255-4*i, 1, 
            (i+11) % 59, -15, 7);
    copylayer(win, 1, 0);
    msleep(50);
  }
  ggetch();
  exit(EXIT_SUCCESS);
}
