#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eggx.h"
#include "e_3d.h"
#include "e_ctrls.h"
#include "stone.h"

#define L 300

int main()
{
  int win, i, j, k, m, key = 0;
  int arp[5] = { 0, 3, 1, 4, 2 }, ars[7] = {
  0, 5, 3, 1, 6, 4, 2};
  double pentagon[3][5], septagon[3][7], parab[3][144], y;

  win = gopen(L, L);
  window(win, -L / 2, -L / 2, L / 2, L / 2);
  gsetbgcolor(win, ECTRL_BGCOLOR);
  newcolor(win, ECTRL_FGCOLOR);
  winname(win, "e_3d demo");
  layer(win, 0, 1);
  gsetnonblock(ENABLE);
  i = j = m = 0;

  for (i = 0; i < 144; i++) {
    y = L/(2*144.0)*i;
    parab[0][i] = 0;
    parab[1][i] = -y;
    parab[2][i] = 0.02*y*(L/2 - y);
  }

  while (key != 'q') {
    g3dsetangle(torad(40 * (1.5 + sin(j * 2 * M_PI / 30))),
		i * 2 * M_PI / 144);
    gclr(win);

    for (k = 0; k < 5; k++) {
      pentagon[0][k] =
	  L / 6 * cos(arp[k] * 2 * M_PI / 5 + m * 2 * M_PI / 50) + L / 5;
      pentagon[1][k] =
	  L / 6 * sin(arp[k] * 2 * M_PI / 5 + m * 2 * M_PI / 50) + L / 5;
      pentagon[2][k] = 0;
    }
    for (k = 0; k < 7; k++) {
      septagon[0][k] =
	  L / 6 * cos(ars[k] * 2 * M_PI / 7 + 2 * m * 2 * M_PI / 50) -
	  L / 5;
      septagon[1][k] = 0;
      septagon[2][k] =
	  L / 6 * sin(ars[k] * 2 * M_PI / 7 + 2 * m * 2 * M_PI / 50) +
	  L / 5;
    }
    newcolor(win, "lightsteelblue");
    newlinestyle(win, LineOnOffDash);
    for(k = 0; k < 6; k++) {
      drawline3d(win, -L/10*k, 0, 0,  -L/10*k, -L/2, 0);
      drawline3d(win, 0, -L/10*k, 0,  -L/2, -L/10*k, 0);
    }
    newlinestyle(win, LineSolid);
    newcolor(win, "red4");
    fillpoly3d(win, pentagon[0], pentagon[1], pentagon[2], 5, 0);
    newcolor(win, "green4");
    fillpoly3d(win, septagon[0], septagon[1], septagon[2], 7, 0);
    newcolor(win, ECTRL_FGCOLOR);
    drawarrow3d(win, 0, 0, 0, 0, 0, L / 3, 10, 6, 12);
    drawarrow3d(win, 0, 0, 0, 0, L / 3, 0, 10, 6, 12);
    drawarrow3d(win, 0, 0, 0, L / 3, 0, 0, 10, 6, 12);
    newcolor(win, "gold3");
    drawstr3d(win, 0, 0, L/3+10, FONTSET, 0, "z¼´");
    drawlines3d(win, parab[0], parab[1], parab[2], 144);
    y = L/(2*36.0)*(i%36);
    putimg24m3d(win, 0 - 6 *(-sin(_phi) - cos(_th)*cos(_phi)), 
                     -y - 6*(cos(_phi) - cos(_th)*sin(_phi)), 
                0.02*y*(L/2 - y) -6*sin(_th), 12, 12, Xpm_image_stone6);
    copylayer(win, 1, 0);
    i++;
    i %= 144;
    j++;
    j %= 30;
    m++;
    m %= 50;
    msleep(40);
    key = ggetch();
  }
  gcloseall();
  return 0;
}
