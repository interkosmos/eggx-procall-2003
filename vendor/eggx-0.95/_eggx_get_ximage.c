#ifndef __EGGX_GET_XIMAGE_C
#define __EGGX_GET_XIMAGE_C 1

#define  CBN 64			/* XColor のキャッシュの個数 */

typedef struct _eggx_color_rbuf {
    int cln;
    int clw;
    int clr;
    int clf;
    int red_depth;
    int green_depth;
    int blue_depth;
    int red_sft;
    int green_sft;
    int blue_sft;
    unsigned long msk;
    unsigned long msk_red;
    unsigned long msk_green;
    unsigned long msk_blue;
    XColor cl[CBN];
} eggx_color_rbuf;

static void init_color_rbuf( eggx_color_rbuf *cache,
			     Display *dis, Colormap cmap,
			     int red_depth, int green_depth, int blue_depth,
			     int red_sft, int green_sft, int blue_sft )
{
    //fprintf(stderr,"debug: init_color_rbuf()\n");
    /* X の depth が 8 の時は，{red,green,blue}_depth がすべて 0 なので，*/
    /* msk も 0 になる */
    cache->msk = 0;
    cache->msk_red = (0x0ffff >> (16-red_depth)) << red_sft;
    cache->msk |= cache->msk_red;
    cache->msk_green = (0x0ffff >> (16-green_depth)) << green_sft;
    cache->msk |= cache->msk_green;
    cache->msk_blue = (0x0ffff >> (16-blue_depth)) << blue_sft;
    cache->msk |= cache->msk_blue;

    //fprintf(stderr,"debug: init_color_rbuf() msk=%lx\n",cache->msk);
    cache->red_depth = red_depth;
    cache->green_depth = green_depth;
    cache->blue_depth = blue_depth;
    cache->red_sft = red_sft;
    cache->green_sft = green_sft;
    cache->blue_sft = blue_sft;

    if ( cache->msk == 0 /* for 8bpp */ ) {
	cache->cln = 0;
	cache->clw = CBN - 1;	/* write */
	cache->clf = cache->clw;	/* offset */
	cache->cl[cache->clw].pixel = 0;
	XQueryColor( dis, cmap, &(cache->cl[cache->clw]) );
	cache->clr = 0;		/* read */
	cache->cln ++;
    }

    return;
} 

static int get_idx_from_color_rbuf( eggx_color_rbuf *cache,
				    Display *dis, Colormap cmap,
				    unsigned long px )
{
    //fprintf(stderr,"debug: get_idx_from_color_rbuf() : pix=%lx\n",px);
    if ( cache->msk == 0 /* for 8bpp */ ) {
	int j, k = 0;
	for ( j=0 ; j < cache->cln ; j++ ) {
	    k = cache->clf + ((cache->clr + j) % cache->cln);
	    if ( cache->cl[k].pixel == px ) {
		cache->clr = k - cache->clf;
		break ;
	    }
	}
	if ( j == cache->cln ) {
	    cache->cln ++;
	    if ( cache->cln > CBN ) cache->cln --;	/* 個数 */
	    cache->clf --;
	    if ( cache->clf < 0 ) cache->clf = 0;	/* offset */
	    cache->clw --;
	    if ( cache->clw < 0 ) cache->clw += CBN;
	    cache->clr = cache->clw - cache->clf;
	    cache->cl[cache->clw].pixel = px;
	    XQueryColor( dis, cmap, &(cache->cl[cache->clw]) );
	    k = cache->clw;
	}
	return k;
    }
    else {
	cache->cl[0].red = (px & cache->msk_red) >> cache->red_sft;
	cache->cl[0].red <<= (16 - cache->red_depth);
	cache->cl[0].green = (px & cache->msk_green) >> cache->green_sft;
	cache->cl[0].green <<= (16 - cache->green_depth);
	cache->cl[0].blue = (px & cache->msk_blue) >> cache->blue_sft;
	cache->cl[0].blue <<= (16 - cache->blue_depth);
	return 0;
    }
}

static XImage *get_ximage( Display *dis, Colormap cmap,
			int red_depth, int green_depth, int blue_depth,
			int red_sft, int green_sft, int blue_sft,
			Pixmap pix_id, int sx0, int sy0, int width, int height,
			eggx_color_rbuf *cache )
{
    XImage *image;
    image = XGetImage( dis, pix_id, sx0, sy0, width, height,
		       0xffffffff, ZPixmap );
    XFlush( dis );

    /* カラーバッファを初期化 */
    init_color_rbuf(cache, dis, cmap, red_depth, green_depth, blue_depth,
		    red_sft, green_sft, blue_sft );

    return image;
}

static void ximage_to_ppmline( Display *dis, Colormap cmap,
			       eggx_color_rbuf *cache,
			       const XImage *image, int width, int height,
			       int y_idx, int depth, unsigned char *line_buf )
{
    int wd, ii, j, k;
    unsigned long px;
    const unsigned char *badr ;

    wd = (image->bits_per_pixel) / 8;
    /* If your Xserver returns wrong image->bit_per_pixel, */
    /* use this determination method of wd. */
    /*
    if( 16 < image->depth ) wd=4 ;
    else wd=(7+image->depth)/8 ;
    */

    badr = (const unsigned char *)(image->data);
    badr += y_idx * image->bytes_per_line;

    for ( ii=0 ; ii < width ; ii++, badr+=wd ) {
	if ( image->byte_order == MSBFirst ) {	/* big */
	    for ( j=0, px=0 ; j < wd ; j++ ) {
		px <<= 8;
		px |= ( badr[j] & 0xff );
	    }
	}
	else {					/* little */
	    for ( j=wd-1, px=0 ; 0 <= j ; j-- ) {
		px <<= 8;
		px |= ( badr[j] & 0xff );
	    }
	}
	/* pixel id からバッファの ID を得る */
	k = get_idx_from_color_rbuf(cache, dis, cmap, px);
	/* */
	//fprintf(stderr, "debug: %d %d %d\n",
	//cache->cl[k].red, cache->cl[k].green, cache->cl[k].blue);
	line_buf[ii * 3 + 0] = depth * (cache->cl[k].red >> 8) / 256;
	line_buf[ii * 3 + 1] = depth * (cache->cl[k].green >> 8) / 256;
	line_buf[ii * 3 + 2] = depth * (cache->cl[k].blue >> 8) / 256;
    }
    return;
}

#endif	/* __EGGX_GET_XIMAGE_C */
