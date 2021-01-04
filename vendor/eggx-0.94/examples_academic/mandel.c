/* egg mandel.c -o mandel -O2 -Wall */
/* マンデルブロ集合の表示 */
#include <math.h>
#include <eggx.h>
/* ウィンドゥのサイズ */
#define WIN_WIDTH  400
#define WIN_HEIGHT 400
/* カラーモード */
#define MYCOLOR    DS9_B
/* マンデルブロ集合を計算するためのパラメータ */
#define XSCALE     1.0
#define YSCALE     1.0
#define REALBEGIN  0.075	/* -2.0 */
#define REALEND    0.175	/* 0.5 */
#define IMAGIBEGIN 0.59		/* -1.25 */
#define IMAGIEND   0.69		/* 1.25 */
#define LIMIT      1000		/* 100 */
#define THRES      4.0
/* マンデブロ集合の色計算 */
int get_mandel( double c_real , double c_imaginary )
{
  int i ;
  double xsq, ysq, tmp, re = 0, im = 0 ;
  for( i=0 ; i < LIMIT ; i++ ){
    if( THRES < ((xsq=re*re)+(ysq=im*im)) ) break ;
    tmp=xsq-ysq+c_real ;
    im=2*re*im+c_imaginary ;
    re=tmp ;
  }
  if( i == LIMIT ) return( 0 ) ;
  else return( i ) ;
}
/* メイン */
int main()
{
  int n,i,j, win, color_r,color_g,color_b ;
  int font_frame[8]={-1,0,1,1,1,0,-1,-1} ;
  double a,b, xstep, ystep ;
  /* 窓をオープン */
  win=gopen(WIN_WIDTH,WIN_HEIGHT) ;
  winname(win,"mandel.c real=[%g:%g] "
          "imag=[%g:%g] limit=%d",
          REALBEGIN,REALEND,IMAGIBEGIN,IMAGIEND,LIMIT) ;
  /* 手動flushモードにする */
  gsetnonflush(ENABLE) ;
  xstep=(double)XSCALE*(REALEND-REALBEGIN)/WIN_WIDTH ;
  ystep=(double)YSCALE*(IMAGIEND-IMAGIBEGIN)/WIN_HEIGHT ;
  /* 集合を描画 */
  b=IMAGIBEGIN ;
  for( i=0 ; i<WIN_HEIGHT ; i++ ){
    a = REALBEGIN ;
    for( j=0 ; j<WIN_WIDTH ; j++ ){
      n = get_mandel( a, b ) ;
      if( 0 < n ){
        makecolor(MYCOLOR,log(1),log(LIMIT),log(n),
                  &color_r,&color_g,&color_b) ;
        newrgbcolor(win,color_r,color_g,color_b) ;
        pset(win,j,i) ;
      }
      a+=xstep ;
    }
    b+=ystep ;
  }
  /* カラーバーを表示 */
  for( i=16 ; i<116 ; i++ ){
    makecolor(MYCOLOR,16,115,i,&color_r,&color_g,&color_b);
    newrgbcolor(win,color_r,color_g,color_b) ;
    moveto(win,2,i) ;
    lineto(win,18,i) ;
  }
  newcolor(win,"black") ;
  for( i=0 ; i<8 ; i++ ){
    int xo,yo ;
    xo=font_frame[i] ;
    yo=font_frame[(i+2)%8] ;
    drawstr(win,1+xo,2+yo,14,0,"%.1f",log(1)) ;
    drawstr(win,1+xo,118+yo,14,0,"%.1f",log(n)) ;
  }
  newcolor(win,"white") ;
  drawstr(win,1,2,14,0,"%.1f",log(1)) ;
  drawstr(win,1,118,14,0,"%.1f",log(n)) ;
  /* 描画命令をflushする */
  gflush() ;
  /* キー入力で終了 */
  ggetch() ;
  gclose(win) ;
  return(0) ;
}

