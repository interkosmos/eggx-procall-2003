#include <stdio.h>
#include <stdlib.h>
#include <eggx.h>

/* #define USE_IMAGEMAGICK 1 */

#define ZOOMWN_WIDTH 256
#define ZOOMWN_HEIGHT 256

typedef struct _pixel_rgb {
    unsigned char a;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel_rgb;

/* 1ピクセルをゲット */
static pixel_rgb get_pixel( int width, int height, const unsigned char *buf,
			    int x, int y )
{
    const unsigned char (*p)[4] = (const unsigned char (*)[4])buf;
    pixel_rgb pix;
    if ( 0 <= x && x < width && 0 <= y && y < height ) {
	long i = (long)y * width + (long)x;
	pix.a = p[i][0];
	pix.r = p[i][1];
	pix.g = p[i][2];
	pix.b = p[i][3];
    }
    else {
	pix.a = 0;
	pix.r = 0;
	pix.g = 0;
	pix.b = 0;
    }
    return pix;
}

/* 拡大されたイメージを取得 */
static void get_zoom_image( int src_width, int src_height,
			    const unsigned char *src_buf,
			    int center_x, int center_y, int zoom_factor,
			    int dest_width, int dest_height,
			    unsigned char *dest_buf )
{
    unsigned char (*dest_p)[4] = (unsigned char (*)[4])dest_buf;
    pixel_rgb pix;
    long i;
    for ( i=0 ; i < dest_height ; i++ ) {
	long j, sx, sy;
	sy = center_y - dest_height/2/zoom_factor + i/zoom_factor;
	for ( j=0 ; j < dest_width ; j++ ) {
	    long idx = i * dest_width + j;
	    sx = center_x - dest_width/2/zoom_factor + j/zoom_factor;
	    pix = get_pixel(src_width, src_height, src_buf, sx, sy);
	    dest_p[idx][0] = pix.a;
	    dest_p[idx][1] = pix.r;
	    dest_p[idx][2] = pix.g;
	    dest_p[idx][3] = pix.b;
	}
    }
}

int main( int argc, char *argv[] )
{
    int ret = 1;
    int win_main, win_zoom, zoom_factor = 2;
    double p_x = -1, p_y = -1;
    unsigned char *zoom_buf = NULL;
    unsigned char *img_buf = NULL;
    int img_width, img_height, img_msk;
    int crosshairs = 1;

    if ( argc < 2 ) {
	fprintf(stderr,"[USAGE]\n");
	fprintf(stderr,"%s filename.png\n",argv[0]);
	return 1;
    }

    /* zoom ウィンドゥ用の画像バッファ */
    zoom_buf = (unsigned char *)malloc(ZOOMWN_WIDTH * ZOOMWN_HEIGHT * 4);
    if ( zoom_buf == NULL ) goto quit;

    //img_buf = readimage(NULL,argv[1],&img_width,&img_height,&img_msk);
    //img_buf = readimage("xbmtopbm",argv[1],&img_width,&img_height,&img_msk);
#ifdef USE_IMAGEMAGICK
    img_buf = readimage("convert",argv[1],&img_width,&img_height,&img_msk);
#else
    img_buf = readimage("pngtopnm",argv[1],&img_width,&img_height,&img_msk);
#endif
    if ( img_buf == NULL ) {
	fprintf(stderr,"ERROR: cannot read file\n");
	goto quit;
    }
    printf("mask value = %d\n",img_msk);

    /* 属性設定(左上を原点にする) */
    gsetinitialattributes(DISABLE, BOTTOM_LEFT_ORIGIN);
    /* ウィンドゥのオープン */
    win_zoom = gopen(ZOOMWN_WIDTH*2, ZOOMWN_HEIGHT);
    win_main = gopen(img_width, img_height);
    layer(win_zoom,0,1);

    /* メインウィンドゥへの画像の転送 */
    gclr(win_main);
    gputimage(win_main, 0,0, img_buf, img_width, img_height, img_msk);

    /* カーソル描画のために，XOR での描画functionに設定 */
    newgcfunction(win_main, GXxor);
    newcolor(win_main, "red");

#if 0	/* 保存のテスト */
    //writeimage(img_buf,img_width,img_height,img_msk,"pnmtopng",256,"hoge.png");
    //writeimage(img_buf,img_width,img_height,img_msk,NULL,256,"hoge.pam");
    gsaveimage( win_main,0,0,0,1999,1999,"pnmtopng",256,"capt0.png");
    {
	int im_w, im_h;
	unsigned char *im = ggetimage(win_main,0,0,0,1999,1999,&im_w,&im_h);
	writeimage(im_w,im_h,im,0,"pnmtopng",256,"capt.png");
	if ( im != NULL ) free(im);
    }
#endif

    /* メインループ */
    while ( 1 ) {
	int win_ev, type, b;
	int needs_redraw = 0;
	double x, y;
	win_ev = ggetevent(&type,&b,&x,&y) ;
	if ( type == EnterNotify ) {
	    fprintf(stderr,"event type = EnterNotify wid=%d\n",win_ev);
	}
	else if ( type == LeaveNotify ) {
	    fprintf(stderr,"event type = LeaveNotify wid=%d\n",win_ev);
	}
	else {
	    fprintf(stderr,"event type = %d wid=%d\n",type,win_ev);
	}
	if ( win_ev == win_main ) {
	    if ( type == MotionNotify || 
		 type == EnterNotify || type == LeaveNotify ) {
		if ( type == LeaveNotify ) {
		    x = -32000;  y = -32000;
		}
		if ( crosshairs ) {
		    /* crosshairs カーソルを消す */
		    drawline(win_main, 0,p_y, img_width,p_y);
		    drawline(win_main, p_x,0, p_x,img_height);
		    /* crosshairs カーソルを表示する */
		    drawline(win_main, 0,y, img_width,y);
		    drawline(win_main, x,0, x,img_height);
		}
		needs_redraw = 1;
		p_x = x;
		p_y = y;
	    }
	    else if ( type == ButtonPress ) {
		if ( b == 1 ) zoom_factor++;
		else if ( b == 3 ) zoom_factor--;
		needs_redraw = 1;
	    }
	}
	if ( type == KeyPress ) {
	    if ( b == 'q' ) goto quit;
	    else if ( b == '>' || b == '+' ) zoom_factor++;
	    else if ( b == '<' || b == '-' ) zoom_factor--;
	    else if ( b == ' ' ) {
		if ( crosshairs ) {
		    /* crosshairs カーソルを消す */
		    drawline(win_main, 0,p_y, img_width,p_y);
		    drawline(win_main, p_x,0, p_x,img_height);
		    crosshairs = 0;
		}
		else {
		    /* crosshairs カーソルを表示する */
		    drawline(win_main, 0,p_y, img_width,p_y);
		    drawline(win_main, p_x,0, p_x,img_height);
		    crosshairs = 1;
		}
	    }
	    needs_redraw = 1;
	}
	if ( needs_redraw ) {
	    if ( zoom_factor < 2 ) zoom_factor = 2;
	    else if ( 32 < zoom_factor ) zoom_factor = 32;
	    get_zoom_image(img_width, img_height,
			   img_buf, p_x, p_y, zoom_factor,
			   ZOOMWN_WIDTH, ZOOMWN_HEIGHT, zoom_buf);
	    gclr(win_zoom);
	    /* 普通のコピー */
	    newgcfunction(win_zoom, GXcopy);
	    gputimage(win_zoom, 0, 0, 
		      zoom_buf, ZOOMWN_WIDTH, ZOOMWN_HEIGHT, img_msk);
	    /* 反転コピー */
	    newgcfunction(win_zoom, GXcopyInverted);
	    gputarea(win_zoom, ZOOMWN_WIDTH, 0, 
		     win_zoom,1, 0,0,ZOOMWN_WIDTH-1, ZOOMWN_HEIGHT-1);
	    /* ズームウィンドゥで，文字列を反転描画する */
	    newgcfunction(win_zoom, GXinvert);
	    drawstr(win_zoom,2,14,14,0,
		    "%dx zoom pos=(%g,%g)\n",zoom_factor,p_x,p_y);
	    copylayer(win_zoom,1,0);
	}
    }

    ret = 0;
 quit:
    gcloseall();
    if ( img_buf != NULL ) free(img_buf);
    if ( zoom_buf != NULL ) free(zoom_buf);
    return ret;
}
