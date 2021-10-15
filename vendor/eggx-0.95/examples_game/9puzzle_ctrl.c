#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "eggx.h"
#include "e_ctrls.h"
#include "9puzzle.h"

#define HIST 10
#define PIX  50
#define N    3

enum mode {SHUFFLE, MOVE};
int    win, iswin, cwin, iscwin, button, type = ButtonPress;
unsigned  char  
       *pix[N][N] = {
          {Xpm_image_cat_0, Xpm_image_cat_1, Xpm_image_cat_2},
          {Xpm_image_cat_3, Xpm_image_cat_4, Xpm_image_cat_5},
          {Xpm_image_cat_6, Xpm_image_cat_7, Xpm_image_cat_8}
       };
void   *ptr[N][N], *ptr0[N][N];
int    pos[N][N]={{0,0,0}}, history[HIST], hist = 0, i_0, j_0;
int    rev[HIST];
double  wx, wy;
double quit = 0, shf = 0, hints = 0, rty = 0;

void draw() 
{
  int i,j;  

  gclr(win);
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      putimg24(win, j*PIX, (2-i)*PIX, PIX, PIX, (unsigned char *)ptr[i][j]);
      if (pos[i][j] == 1) fillrect(win, j*PIX, (2-i)*PIX, PIX, PIX);
     }
  }
  copylayer(win,1,0);
}

int mov(float x, float y, int mode)
{
  int i, j, si, sj;

  j = (int)floor(x/PIX);
  i = 2 - (int)floor(y/PIX);
  si = i - 1; sj = j;
  if (si < 0){;} else if (pos[si][sj] == 1) goto RET;
  si = i + 1, sj = j;
  if (si > 2){;} else if (pos[si][sj] == 1) goto RET;
  si = i; sj = j - 1;
  if (sj < 0){;} else if (pos[si][sj] == 1) goto RET;
  si = i; sj = j + 1;
  if (sj > 2){;} else if (pos[si][sj] == 1) goto RET;
  
  return 0;

RET:
  pos[i][j] = 1;   
  pos[si][sj] = 0;   
  ptr[si][sj] = ptr[i][j];
  if ( mode == SHUFFLE) {
    ptr0[si][sj] = ptr0[i][j];
    rev[hist] = N*si + sj;
    history[hist] = N*i + j;
    hist++;
  }
  return 1;
}

void init()
{
  int i, j;

  for (i = 0; i < N; i++) 
    for (j = 0; j< N; j++) {
      ptr[i][j] = ptr0[i][j] = (void *)pix[i][j];
      pos[i][j] = 0;
    };
  pos[i_0][j_0] = 1;
}

void retry()
{
  int i, j, n;
  
  for (i = 0; i < N; i++) 
    for (j = 0; j < N; j++) 
      ptr[i][j] = (void *)pix[i][j];
  pos[i_0][j_0] = 1;
  for (n = 0; n < HIST; n++){
    j = history[n] % N; 
    i = (int)(history[n]-j)/N;
    mov(j*PIX, (2-i)*PIX, MOVE);
  }
  draw();
  rty = 0;
}

void shuffle()
{
  init();
  hist = 0; /* hist ¤Ï mov()Æâ¤Çincrement */
  while (hist < HIST) {
    wx = (float)(drand48()*N*PIX);
    wy = (float)(drand48()*N*PIX);
    mov(wx,wy,SHUFFLE);
  }
  draw();
  shf = 0;
}

void solve()
{
  int i, j, n;

  retry();
  msleep(1000);

  for (n = HIST-1; n >= 0; n--){
    j = rev[n] % N; 
    i = (rev[n]-j)/N;
    mov(j*PIX, (2-i)*PIX, MOVE);
    draw();
    msleep(100);   
  }
  hints = 0;
}

void bye(){ exit(0); }

int main(int argc, char **argv)
{
  e_ctrl ctrls[] = {
    {"_shuffle", &shf, 0, &shuffle, NULL},
    {"_retry", &rty, 0, &retry, NULL},
    {"_hints", &hints, 0, &solve, NULL},
    {"_quit", &quit, 0, &bye, NULL},
  };
  cwin = init_ctrls(ctrls, 4);

  win = gopen(N*PIX, N*PIX);
  newcolor(win, "black");
  layer(win, 0, 1);
  srand48(time(NULL));
  i_0 =(int)(drand48()*N);
  j_0 =(int)(drand48()*N);
  init();
  while (!quit) {
    draw();
    display_ctrls(cwin, ctrls, 4, wx, wy, iswin, type, button);
    iswin = ggetxpress(&type, &button, &wx, &wy);
    if (win == iswin && type == ButtonPress) {mov(wx, wy, MOVE);}
    msleep(100);
  }

  return 0;
}
