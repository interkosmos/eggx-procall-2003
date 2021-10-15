#include <stdio.h>
#include <stdlib.h>

typedef struct {
  double f[6];
} vec6;

void rk4fixv6(vec6 f(), double t, double *r, double h)
{
  int i, n;
  double ts, rs[6], k[4][6];
  double c[4] = { 1, 0.5, 0.5, 1 };
  vec6 diff;

  for (n = 0; n < 6; n++) {
    rs[n] = r[n];
  }

  ts = t;
  for (i = 0; i < 4; i++) {
    if (i > 0) {
      for (n = 0; n < 6; n++) {
	rs[n] = r[n] + k[i - 1][n] * c[i];
      }
      ts = t + c[i] * h;
    }
    diff = f(ts, rs);
    for (n = 0; n < 6; n++) {
      k[i][n] = h * diff.f[n];
    }
  }

  for (n = 0; n < 6; n++) {
    for (i = 0; i < 4; i++) {
      r[n] += k[i][n] / c[i] / 6;
    }
  }
}
