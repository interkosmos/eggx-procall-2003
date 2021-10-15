/* egg roche.c -o roche -Wall */
#include <stdio.h>
#include <math.h>
#include <eggx.h>

#define MRATIO_F 0.23	/* 質量比f */

#define XMIN -1.5	/* x座標の範囲 */
#define XMAX 1.5
#define YMIN -1.5	/* y座標の範囲 */
#define YMAX 1.5
#define ZMIN -3.5	/* カラーの範囲 */
#define ZMAX -1.4
#define WINWIDTH  512	/* ウィンドゥのサイズ */
#define WINHEIGHT 512
#define XSAMPLES 30	/* メッシュのサイズ */
#define YSAMPLES 30
#define VXSCALE 0.02	/* 矢印の大きさ */
#define VYSCALE 0.02
#define VCARMAX 0.25	/* 表示する矢印の最大サイズ */

/*
#define USE_IMAGEMAGICK
*/

#define USE_NETPBM


float get_fx( float, float, float ) ;
float get_fy( float, float, float ) ;
float get_phi( float, float, float ) ;

int main()
{
    int i,j,key,sl=0,wl=1 ;
    int shape_i = 11, shape_j = 4 ;
    float f=MRATIO_F ;
    float x,y,zx,zy,z,ph,ms_w,ms_h ;
    float zran=(ZMAX-ZMIN) ;
    float zcen=(ZMAX+ZMIN)/2.0 ;
    color_prms cl = {
	EGGX_COLOR_BEGIN,			/* カラーパターン */
	CP_CONTRAST | CP_BRIGHTNESS | CP_GAMMA,	/* フラグ */
	1.0,					/* コントラスト */
	0.0,					/* ブライトネス*/
	1.0,					/* γ */
    } ;
    int win ;
    int cl_r,cl_g,cl_b ;

    win=gopen(WINWIDTH,WINHEIGHT) ;	/* ウィンドゥのタイトル */
    /* 座標系を変更する */
    coordinate(win, 0,0, XMIN,YMIN, 
	       WINWIDTH/(XMAX-XMIN), WINHEIGHT/(YMAX-YMIN)) ;
    layer(win,sl,wl) ;

    puts("【キーボードでの操作方法】") ;
    puts("'PageUp','PageDown'  … 質量比変更") ;
    puts("'c','C'              … カラーパターン") ;
    puts("'↑','↓','←','→'  … カラー調整") ;
    puts("'[',']'              … コントラスト") ;
    puts("'{','}'              … ブライトネス") ;
    puts("'<','>'              … γ補正") ;
    puts("'s'                  … 画像を保存") ;
    puts("'q','Esc'            … 終了") ;

    ms_w=(float)(XMAX-XMIN)/XSAMPLES ;	/* メッシュ1個分のサイズ */
    ms_h=(float)(YMAX-YMIN)/YSAMPLES ;

    do{
	/* ウィンドゥのタイトル */
	winname(win,"ロッシュワールド('s'キーで画像save) f=%g zcen=%g zran=%g",
		f,zcen,zran) ;
	for( i=0 ; i<XSAMPLES ; i++ ){	/* ポテンシャルを色で表現 */
	    x=XMIN+ms_w*i ;
	    for( j=0 ; j<YSAMPLES ; j++ ){
		y=YMIN+ms_h*j ;
		ph=get_phi(x+ms_w/2.0,y+ms_w/2.0,f) ;
		generatecolor(&cl,zcen-zran/2,zcen+zran/2,ph,
			      &cl_r,&cl_g,&cl_b) ;
		newrgbcolor(win,cl_r,cl_g,cl_b) ;
		fillrect(win,x,y,ms_w*1.5,ms_h*1.5) ;
	    }
	}
	newpen(win,1) ;
	for( i=0 ; i<XSAMPLES ; i++ ){	/* テスト粒子に働く力を矢印で表現 */
	    x=XMIN+ms_w*(i+0.5) ;
	    for( j=0 ; j<YSAMPLES ; j++ ){
		y=YMIN+ms_h*(j+0.5) ;
		zx=get_fx(x,y,f) ;
		zy=get_fy(x,y,f) ;
		zx *= VXSCALE ;
		zy *= VYSCALE ;
		z=sqrt(zx*zx+zy*zy) ;
		if( z <= VCARMAX ){
		    drawarrow(win,x-zx,y-zy,x+zx,y+zy,0.3,0.2,
			      shape_i*10+shape_j) ;
		}
	    }
	}
	sl ^= 1 ;				/* XORをとってレイヤを切替 */
	wl ^= 1 ;
	layer( win,sl,wl ) ;
	key=ggetch() ;				/* キー入力があるまで待つ */
	if( key == 0x002 ) f += 0.1 ;		/* PageUp */
	else if( key == 0x006 ) f -= 0.1 ;	/* PageDown */
	else if( key == 0x01e ) zcen += 0.1 ;	/* ↑ */
	else if( key == 0x01f ) zcen -= 0.1 ;	/* ↓ */
	else if( key == 0x01c ) zran += 0.1 ;	/* → */
	else if( key == 0x01d ) zran -= 0.1 ;	/* ← */
	else if( key == 'i' ) {
	    shape_i += 1 ;
	    if ( 12 < shape_i ) shape_i = 10 ;
	}
	else if( key == 'j' ) {
	    shape_j += 1 ;
	    if ( 7 < shape_j ) shape_j = 1 ;
	}
	else if( key == 'c' ){			/* 'c','C'キーでカラーパターン変更 */
	    cl.colormode++ ;
	    if( EGGX_COLOR_BEGIN+EGGX_COLOR_NUM <= cl.colormode )
		cl.colormode=EGGX_COLOR_BEGIN ;
	}
	else if( key == 'C' ){
	    cl.colormode-- ;
	    if( cl.colormode < EGGX_COLOR_BEGIN )
		cl.colormode=EGGX_COLOR_BEGIN+EGGX_COLOR_NUM-1 ;
	}
	else if( key == '[' ){			/* '[',']'でコントラスト変更 */
	    cl.contrast += 0.05 ;
	    if( 1 < cl.contrast ) cl.contrast = 1 ;
	}
	else if( key == ']' ){
	    cl.contrast -= 0.05 ;
	    if( cl.contrast < 0 ) cl.contrast = 0 ;
	}
	else if( key == '{' ){			/* '{','}'でブライトネス変更 */
	    cl.brightness += 0.05 ;
	    if( 1 < cl.brightness ) cl.brightness = 1 ;
	}
	else if( key == '}' ){
	    cl.brightness -= 0.05 ;
	    if( cl.brightness < 0 ) cl.brightness = 0 ;
	}
	else if( key == '<' ){			/* '<','>'でγ変更 */
	    cl.gamma += 0.025 ;
	}
	else if( key == '>' ){
	    cl.gamma -= 0.025 ;
	    if( cl.gamma <= 0 ) cl.gamma = 0.025 ;
	}
	else if( key == 's' ){			/* 's'キーで保存 */
#ifdef USE_NETPBM
	    saveimg( win,sl,XMIN,YMIN,XMAX,YMAX,
		     "pnmtops -noturn -dpi 72 -equalpixels -psfilter -flate -ascii85",256,
		     "roche_f=%g.eps",f) ;
	    printf("画像を保存: filename='roche_f=%g.eps'\n",f) ;
#else
#ifdef USE_IMAGEMAGICK
	    saveimg( win,sl,XMIN,YMIN,XMAX,YMAX,"convert",256,
		     "roche_f=%g.png",f) ;
	    printf("画像を保存: filename='roche_f=%g.png'\n",f) ;
#else
	    saveimg( win,sl,XMIN,YMIN,XMAX,YMAX,"",256,
		     "roche_f=%g.ppm",f) ;
	    printf("画像を保存: filename='roche_f=%g.ppm'\n",f) ;
#endif
#endif
	}
	if( f < 0 ) f=0 ;
	if( zran < 0 ) zran=0.1 ;
    } while( key != 0x01b && key != 'q' ) ;	/* ESCキーか 'q'キーで終了 */

    gcloseall() ;
    return(0) ;
}

/* テスト粒子に働く力 */
float get_fx( float x, float y, float f )
{
    float fx0,fx1 ;
    fx0 = -(x+f/(1+f))/pow(pow(x+f/(1+f),2.0)+y*y,3.0/2.0) ;
    fx1 = -(x-1/(1+f))*f/pow(pow(x-1/(1+f),2.0)+y*y,3.0/2.0) ;
    return( fx0+fx1+(1+f)*x ) ;
}     
float get_fy( float x, float y, float f )
{
    float fy0,fy1 ;
    fy0= -(y)/pow(pow(x+f/(1+f),2)+y*y,3.0/2.0) ;
    fy1= -(y)*f/pow(pow(x-1/(1+f),2)+y*y,3.0/2.0) ;
    return( fy0+fy1+(1+f)*y ) ;
}

/* ポテンシャル */
float get_phi( float x,float y,float f )
{
    return( -1/sqrt(pow(x+f/(1+f),2)+y*y) 
	    -f/sqrt(pow(x-1/(1+f),2)+y*y)
	    -(1+f)*(x*x+y*y)/2 ) ;
}
