#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"

#define _3dto2dx(a, b, c) (-sin(_phi)*(a)+cos(_phi)*(b))
#define _3dto2dy(a, b, c) (-cos(_th)*cos(_phi)*(a) - cos(_th)*sin(_phi)*(b)+sin(_th)*(c))
#define _3dto2dz(a, b, c) (sin(_th)*cos(_phi)*(a)+sin(_th)*sin(_phi)*(b)+cos(_th)*(c))

#define torad(a) ((a)*M_PI/180)
double _phi = torad(15);
double _th = torad(60);
double _uoff = 0;
double _voff = 0;


void g3dsetangle(double th, double phi)
{
  _th  = th; _phi = phi;
}

void g3dsetoffset(double u, double v)
{
  _uoff = u; _voff = v;
}

void drawlines3d(int wn, double *x, double *y, double *z, int n)
{
  double *xp, *yp;
  int i;

  xp = malloc(n *(sizeof(double)));
  yp = malloc(n *(sizeof(double)));
  for(i = 0; i < n; i++) {
    xp[i] = _3dto2dx(x[i], y[i], z[i])+_uoff;
    yp[i] = _3dto2dy(x[i], y[i], z[i])+_voff;
  }
  drawlines(wn, xp, yp, n);
  free(xp);
  free(yp);
}

void drawline3d(int wn, double x0, double y0, double z0,
	            double x1, double y1, double z1 )
{
  drawline(wn, _3dto2dx(x0, y0, z0)+_uoff, _3dto2dy(x0, y0, z0)+_voff, 
	       _3dto2dx(x1, y1, z1)+_uoff, _3dto2dy(x1, y1, z1)+_voff );
}

void moveto3d(int wn, double x1, double y1, double z1 )
{
  moveto(wn, _3dto2dx(x1, y1, z1)+_uoff, _3dto2dy(x1, y1, z1)+_voff );
}

void lineto3d(int wn, double x1, double y1, double z1 )
{
  lineto(wn, _3dto2dx(x1, y1, z1)+_uoff, _3dto2dy(x1, y1, z1)+_voff );
}

void drawpts3d(int wn, double *x, double *y, double *z, int n)
{
  double *xp, *yp;
  int i;

  xp = malloc(n *(sizeof(double)));
  yp = malloc(n *(sizeof(double)));
  for(i = 0; i < n; i++) {
    xp[i] = _3dto2dx(x[i], y[i], z[i])+_uoff;
    yp[i] = _3dto2dy(x[i], y[i], z[i])+_voff;
  }
  drawpts(wn, xp, yp, n);
  free(xp);
  free(yp);
}

void pset3d(int wn, double x, double y, double z )
{
  pset(wn, _3dto2dx(x, y, z)+_uoff, _3dto2dy(x, y, z)+_voff );
}

void drawarrow3d(int wn, double x0, double y0, double z0,
                 double x1, double y1, double z1, 
		 double s, double w, int k)
{
  drawarrow(wn, _3dto2dx(x0, y0, z0)+_uoff, _3dto2dy(x0, y0, z0)+_voff,
	        _3dto2dx(x1, y1, z1)+_uoff, _3dto2dy(x1, y1, z1)+_voff, 
                                                               s, w, k);
}

void drawstr3d(int wn, double x, double y, double z, int size,
               double theta, const char *argsformat, ...)
{
  drawstr(wn, _3dto2dx(x, y, z)+_uoff, _3dto2dy(x, y, z)+_voff, 
          size, theta, argsformat);
}

void putimg24m3d(int wn, double x, double y, double z, int w, int h, 
              unsigned char * buf)
{
  putimg24m(wn, _3dto2dx(x,y,z)+_uoff, _3dto2dy(x,y,z)+_voff, w, h, buf);
}

void fillpoly3d(int wn, double *x, double *y, double *z, int n, int mode)
{
  double *xp, *yp;
  int i;

  xp = malloc(n *(sizeof(double)));
  yp = malloc(n *(sizeof(double)));
  for(i = 0; i < n; i++) {
    xp[i] = _3dto2dx(x[i], y[i], z[i])+_uoff;
    yp[i] = _3dto2dy(x[i], y[i], z[i])+_voff;
  }
  fillpoly(wn, xp, yp, n, mode);

  free(xp);
  free(yp);
}

void drawpoly3d(int wn, double *x, double *y, double *z, int n)
{
  double *xp, *yp;
  int i;

  xp = malloc(n *(sizeof(double)));
  yp = malloc(n *(sizeof(double)));
  for(i = 0; i < n; i++) {
    xp[i] = _3dto2dx(x[i], y[i], z[i])+_uoff;
    yp[i] = _3dto2dy(x[i], y[i], z[i])+_voff;
  }
  drawpoly(wn, xp, yp, n);
  free(xp);
  free(yp);
}

typedef  struct {
  int idx;
  double w;
} _zdepth;


static int _compz(const void *_p1, const void *_p2)
{
  _zdepth *p1 = (_zdepth *) _p1;
  _zdepth *p2 = (_zdepth *) _p2;

  if (p1->w < p2->w) {
    return -1;
  } else if (p1->w == p2->w) {
    return 0;
  } else
    return 1;
}

void g3dmesh(int wn, double *d, int m, int n, double sx, double sy, double sz,
	     int lr, int lg, int lb, int line, int cmode, double zmin, double zmax)
{
  int i, j, k, sk, ij, n3, bi[4], r, g, b;
  double facet[3][4];
  _zdepth *zdepth;

  zdepth = malloc((n-1)*(m-1)*sizeof(_zdepth));
  n3 = 3*n;

  for (i = 0; i < m - 1; i++) {
    for (j = 0; j < n - 1; j++) {
      ij = 3*(n*i+j);
      zdepth[(n-1)*i+j].idx = (n-1)*i + j;
      zdepth[(n-1)*i+j].w = 
                _3dto2dz(d[ij],  d[ij+1],  d[ij+2])  +
                _3dto2dz(d[ij+3],  d[ij+4],  d[ij+5]) +
                _3dto2dz(d[ij+n3], d[ij+n3+1], d[ij+n3+2]) +
	        _3dto2dz(d[ij+n3+3], d[ij+n3+4], d[ij+n3+5]) ;
    }
  }
  qsort(zdepth, (n-1)*(m-1), sizeof(_zdepth), _compz);

  bi[0] = 0; bi[1] = 3; bi[2] = 3 + n3; bi[3] = n3;
  gclr(wn);

  for (k = 0; k < (n-1)*(m-1); k++) {
    sk = zdepth[k].idx;
    j = sk % (n - 1);
    i = (sk - j)/(n - 1);
    ij = 3*(n*i+j);
    for (i = 0; i < 4; i++) {
       facet[0][i] = sx*d[ij+bi[i]];
       facet[1][i] = sy*d[ij+bi[i]+1];
       facet[2][i] = sz*d[ij+bi[i]+2];
    }
    makecolor(cmode, zmin, zmax, (d[ij+2]+d[ij+5]+d[ij+2+n3]+d[ij+n3+5])/4.0, 
              &r, &g, &b);
    newrgbcolor(wn, r, g, b);
    fillpoly3d(wn, facet[0], facet[1], facet[2], 4, 0);
    if (line) {
      newrgbcolor(wn, lr, lg, lb);
      drawpoly3d(wn, facet[0], facet[1], facet[2], 4);
    }
  }
  free(zdepth);
}

