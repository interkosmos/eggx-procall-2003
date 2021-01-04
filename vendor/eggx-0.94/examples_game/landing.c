#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <eggx.h>
#include "ppms.h"

#define WD 300
#define HT 500

int main()
{
  double x, y, v, a = -10, t = 0, dt = 0.05;
  double xe, ye;
  float firex[3], firey[3];
  int i, win, raise = 0;
  char strv[80], strtime[80];

  for ( i=0 ; i < sizeof(lander)/4 ; i++ ) {	/* create mask */
      if ( 8 < lander[i*4+1] && 8 < lander[i*4+2] && 8 < lander[i*4+3] )
	  lander[i*4] = 255;
  }

  srand48(times(NULL));
  win = gopen(WD,HT);
  newcolor(win, "Orange");
  gsetbgcolor(win, "black");

  y = HT - 35;
  x = 0.5*WD;
  v = 0;
  xe = drand48()*250 + 10;
  ye = drand48()*300 + 100;

  layer(win,0,2);	/* layer2: background */
  putimg24(win, xe, ye, 32, 32, earth);
  putimg24(win, 0, 0, 300, 34, moon);
  layer(win,0,1);

  gsetnonblock(ENABLE);
  for (;;) {  
    if (ggetch() == 0x020) { /* space key */
      a = 10; raise = 1;
    } else {
      a = -10; raise = 0;
    }
    t += dt;
    v += a * dt;
    y += v * dt;
    sprintf(strv,"velocity = %.1f",v);
    sprintf(strtime,"time = %.1f",t);
    firex[0] = x; firex[1] = x - 4; firex[2] = x + 4;
    firey[0] = y - 10; firey[1] = y + 5; firey[2] = y + 5;
    copylayer(win, 2, 1);
    putimg24m(win, x-18, y, 35, 35, lander);
    if (raise == 1) fillpoly(win, firex, firey, 3, 1);
    drawstr(win, WD/2+35, 55, 14, 0, strtime);
    drawstr(win, WD/2+35, 35, 14, 0, strv);
    copylayer(win, 1, 0);
    msleep(40);
    if (y < 15) break;
  }
  if ( v >= -15 ) {
    drawstr(win, WD/2 - 90, HT/2, 24, 0, "Congraturation!");
    copylayer(win, 1, 0);
    gsetnonblock(DISABLE);
    for (;;){ 
      if (ggetch() == 0x071) break; /* q key */
    }
  } else {
    int q;
    newcolor(win, "red");
    newgcfunction(win, GXxor);
    copylayer(win, 2, 1);
    for ( i=20 ; 0 < i ; ) {
      i--;
      q = drand48()*i*2-i;
      if ( 0 < q ) fillrect(win,0,0,WD,HT);
      gscroll(win, q, i, 0);
      copylayer(win, 1, 0);
      msleep(20);
      gscroll(win, -q, -i, 0);
      copylayer(win, 1, 0);
      msleep(20);
      if ( 0 < q ) fillrect(win,0,0,WD,HT);
    }
  }
  gclose(win);

  return 0;
}
