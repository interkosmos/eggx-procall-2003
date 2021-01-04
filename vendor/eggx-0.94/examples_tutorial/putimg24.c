/* egg putimg24.c -o putimg24 -Wall */
#include <stdio.h>
#include <eggx.h>

#define PPMFILE "plamo_banner.ppm"

int main()
{
    FILE *fp ;
    int i,j,width,height,depth ;
    int xdepth,rootwidth,rootheight ;
    /* Xサーバのdepthを調べる */
    if( ggetdisplayinfo(&xdepth,&rootwidth,&rootheight) ) return(-1) ;
    if( xdepth < 16 ){
	fprintf(stderr,"16bpp以上のXサーバで実行してください．\n") ;
	return(-1) ;
    }

    fp = fopen(PPMFILE,"rb") ;
    if( fp == NULL ) return(-1) ;
    if( fgetc(fp)!='P' ) return(-1) ;
    if( fgetc(fp)!='6' ) return(-1) ;
    while( fgetc(fp) != '\n' ) ;
    i=fscanf(fp,"%d %d\n",&width,&height) ;
    if( i!=2 ) return(-1) ;
    i=fscanf(fp,"%d\n",&depth) ;
    if( i!=1 ) return(-1) ;
    if( depth!=255 ) return(-1) ;

    {
	int wn ;
	unsigned char img[width*height*4] ;
	/* ウィンドゥを OVERRIDE_REDIRECT 属性に */
	gsetinitialattributes(ENABLE, OVERRIDE_REDIRECT) ;
	/* ウィンドゥの出現位置を決定 */
	gsetinitialgeometry("%+d%+d",
			    rootwidth/2-width/2,
			    rootheight/2-height/2) ;
	wn=gopen(width,height) ;
	winname(wn,"画像転送のテスト") ;
	/* 画像を用意する */
	for( i=0,j=0 ; i<width*height ; i++ ){
	    img[j++]=0 ;
	    img[j++]=(unsigned char)fgetc(fp) ;
	    img[j++]=(unsigned char)fgetc(fp) ;
	    img[j++]=(unsigned char)fgetc(fp) ;
	}
	/* 画像を転送 */
	gputimage(wn,0,0,img,width,height,0) ;
	/* キー入力があれば終了 */
	ggetch() ;
    }
    return(0) ;
}
