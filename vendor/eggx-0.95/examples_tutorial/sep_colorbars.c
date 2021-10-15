/* egg sep_colorbars.c -o sep_colorbars -Wall */
#include <eggx.h>

#define WIDTH 512
#define HEIGHT 16
#define SEP_N  16

int main()
{
    int i,j,k,key ;
    int win,r,g,b ;

    win = gopen(WIDTH,HEIGHT*(SEP_N+1)) ;
    layer(win,0,1) ;

    for( i=EGGX_COLOR_BEGIN ; i<EGGX_COLOR_NUM-EGGX_COLOR_BEGIN ; i++ ){
	for( j=0 ; j<WIDTH ; j++ ){
	    makecolor(i,0,WIDTH,j,&r,&g,&b) ;
	    newrgbcolor(win,r,g,b) ;
	    moveto(win,j,0) ;
	    lineto(win,j,HEIGHT-2) ;
	}
	for( k=0 ; k<SEP_N ; k++ ){
	    for( j=0 ; j<WIDTH ; j++ ){
		color_prms p ;
		p.colormode = i ;
		p.flags = CP_SEPLEVEL ;
		p.seplevel = 2*(SEP_N-k) ;
		generatecolor(&p,0,WIDTH,j,&r,&g,&b) ;
		newrgbcolor(win,r,g,b) ;
		moveto(win,j,HEIGHT*(1+k)) ;
		lineto(win,j,HEIGHT*(2+k)-2) ;
	    }
	}
	copylayer(win,1,0) ;
	key = ggetch() ;
	if( key == 'q' ) break ;
    }
    gclose(win) ;

    return(0) ;
}
