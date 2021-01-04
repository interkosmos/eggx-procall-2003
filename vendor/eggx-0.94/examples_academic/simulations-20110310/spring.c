/* EGGX�ǿ�ʿ����angle�������Фͤ����� */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"

#define IMAX 100

void spring(int win,		/* ������ */
	    double x0,		/* ����ü */
	    double y0, double L,	/* �Ф�Ĺ�� */
	    double NECK,	/* ���Ĺ�� */
	    double R,		/* �Фͤ�Ⱦ�� */
	    double TN,		/* ���� */
	    double angle	/* ���� */
    ) {
  int i;
  float xs[IMAX + 3], ys[IMAX + 3], xx, yy, rad;
  rad = (float) (M_PI / 180 * angle);

  xs[0] = (float) x0;
  ys[0] = (float) y0;
  for (i = 1; i < IMAX + 2; i++) {
    xx = (float) (i - 1) / IMAX * (L - 2 * NECK) + NECK;
    yy = (float) (sin(TN * 2 * M_PI / IMAX * (i - 1)) * R);
    xs[i] = (float) (x0 + yy * sin(rad) + xx * cos(rad));
    ys[i] = (float) (y0 - yy * cos(rad) + xx * sin(rad));
  }
  xs[IMAX + 2] = (float) (x0 + L * cos(rad));
  ys[IMAX + 2] = (float) (y0 + L * sin(rad));
  moveto(win, xs[0], ys[0]);
  drawlines(win, xs, ys, IMAX + 3);
}
