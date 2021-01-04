/* egg roche.c -o roche -Wall */
#include <stdio.h>
#include <math.h>
#include <eggx.h>

#define MRATIO_F 0.23	/* ������f */

#define XMIN -1.5	/* x��ɸ���ϰ� */
#define XMAX 1.5
#define YMIN -1.5	/* y��ɸ���ϰ� */
#define YMAX 1.5
#define ZMIN -3.5	/* ���顼���ϰ� */
#define ZMAX -1.4
#define WINWIDTH  512	/* ������ɥ��Υ����� */
#define WINHEIGHT 512
#define XSAMPLES 30	/* ��å���Υ����� */
#define YSAMPLES 30
#define VXSCALE 0.02	/* ������礭�� */
#define VYSCALE 0.02
#define VCARMAX 0.25	/* ɽ����������κ��祵���� */

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
	EGGX_COLOR_BEGIN,			/* ���顼�ѥ����� */
	CP_CONTRAST | CP_BRIGHTNESS | CP_GAMMA,	/* �ե饰 */
	1.0,					/* ����ȥ饹�� */
	0.0,					/* �֥饤�ȥͥ�*/
	1.0,					/* �� */
    } ;
    int win ;
    int cl_r,cl_g,cl_b ;

    win=gopen(WINWIDTH,WINHEIGHT) ;	/* ������ɥ��Υ����ȥ� */
    /* ��ɸ�Ϥ��ѹ����� */
    coordinate(win, 0,0, XMIN,YMIN, 
	       WINWIDTH/(XMAX-XMIN), WINHEIGHT/(YMAX-YMIN)) ;
    layer(win,sl,wl) ;

    puts("�ڥ����ܡ��ɤǤ������ˡ��") ;
    puts("'PageUp','PageDown'  �� �������ѹ�") ;
    puts("'c','C'              �� ���顼�ѥ�����") ;
    puts("'��','��','��','��'  �� ���顼Ĵ��") ;
    puts("'[',']'              �� ����ȥ饹��") ;
    puts("'{','}'              �� �֥饤�ȥͥ�") ;
    puts("'<','>'              �� ������") ;
    puts("'s'                  �� ��������¸") ;
    puts("'q','Esc'            �� ��λ") ;

    ms_w=(float)(XMAX-XMIN)/XSAMPLES ;	/* ��å���1��ʬ�Υ����� */
    ms_h=(float)(YMAX-YMIN)/YSAMPLES ;

    do{
	/* ������ɥ��Υ����ȥ� */
	winname(win,"��å������('s'�����ǲ���save) f=%g zcen=%g zran=%g",
		f,zcen,zran) ;
	for( i=0 ; i<XSAMPLES ; i++ ){	/* �ݥƥ󥷥��򿧤�ɽ�� */
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
	for( i=0 ; i<XSAMPLES ; i++ ){	/* �ƥ���γ�Ҥ�Ư���Ϥ������ɽ�� */
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
	sl ^= 1 ;				/* XOR��Ȥäƥ쥤������� */
	wl ^= 1 ;
	layer( win,sl,wl ) ;
	key=ggetch() ;				/* �������Ϥ�����ޤ��Ԥ� */
	if( key == 0x002 ) f += 0.1 ;		/* PageUp */
	else if( key == 0x006 ) f -= 0.1 ;	/* PageDown */
	else if( key == 0x01e ) zcen += 0.1 ;	/* �� */
	else if( key == 0x01f ) zcen -= 0.1 ;	/* �� */
	else if( key == 0x01c ) zran += 0.1 ;	/* �� */
	else if( key == 0x01d ) zran -= 0.1 ;	/* �� */
	else if( key == 'i' ) {
	    shape_i += 1 ;
	    if ( 12 < shape_i ) shape_i = 10 ;
	}
	else if( key == 'j' ) {
	    shape_j += 1 ;
	    if ( 7 < shape_j ) shape_j = 1 ;
	}
	else if( key == 'c' ){			/* 'c','C'�����ǥ��顼�ѥ������ѹ� */
	    cl.colormode++ ;
	    if( EGGX_COLOR_BEGIN+EGGX_COLOR_NUM <= cl.colormode )
		cl.colormode=EGGX_COLOR_BEGIN ;
	}
	else if( key == 'C' ){
	    cl.colormode-- ;
	    if( cl.colormode < EGGX_COLOR_BEGIN )
		cl.colormode=EGGX_COLOR_BEGIN+EGGX_COLOR_NUM-1 ;
	}
	else if( key == '[' ){			/* '[',']'�ǥ���ȥ饹���ѹ� */
	    cl.contrast += 0.05 ;
	    if( 1 < cl.contrast ) cl.contrast = 1 ;
	}
	else if( key == ']' ){
	    cl.contrast -= 0.05 ;
	    if( cl.contrast < 0 ) cl.contrast = 0 ;
	}
	else if( key == '{' ){			/* '{','}'�ǥ֥饤�ȥͥ��ѹ� */
	    cl.brightness += 0.05 ;
	    if( 1 < cl.brightness ) cl.brightness = 1 ;
	}
	else if( key == '}' ){
	    cl.brightness -= 0.05 ;
	    if( cl.brightness < 0 ) cl.brightness = 0 ;
	}
	else if( key == '<' ){			/* '<','>'�Ǧ��ѹ� */
	    cl.gamma += 0.025 ;
	}
	else if( key == '>' ){
	    cl.gamma -= 0.025 ;
	    if( cl.gamma <= 0 ) cl.gamma = 0.025 ;
	}
	else if( key == 's' ){			/* 's'��������¸ */
#ifdef USE_NETPBM
	    saveimg( win,sl,XMIN,YMIN,XMAX,YMAX,
		     "pnmtops -noturn -dpi 72 -equalpixels -psfilter -flate -ascii85",256,
		     "roche_f=%g.eps",f) ;
	    printf("��������¸: filename='roche_f=%g.eps'\n",f) ;
#else
#ifdef USE_IMAGEMAGICK
	    saveimg( win,sl,XMIN,YMIN,XMAX,YMAX,"convert",256,
		     "roche_f=%g.png",f) ;
	    printf("��������¸: filename='roche_f=%g.png'\n",f) ;
#else
	    saveimg( win,sl,XMIN,YMIN,XMAX,YMAX,"",256,
		     "roche_f=%g.ppm",f) ;
	    printf("��������¸: filename='roche_f=%g.ppm'\n",f) ;
#endif
#endif
	}
	if( f < 0 ) f=0 ;
	if( zran < 0 ) zran=0.1 ;
    } while( key != 0x01b && key != 'q' ) ;	/* ESC������ 'q'�����ǽ�λ */

    gcloseall() ;
    return(0) ;
}

/* �ƥ���γ�Ҥ�Ư���� */
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

/* �ݥƥ󥷥�� */
float get_phi( float x,float y,float f )
{
    return( -1/sqrt(pow(x+f/(1+f),2)+y*y) 
	    -f/sqrt(pow(x-1/(1+f),2)+y*y)
	    -(1+f)*(x*x+y*y)/2 ) ;
}
