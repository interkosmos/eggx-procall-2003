/* egg mousetst.c -o mousetst -Wall */
#include <stdio.h>
#include <eggx.h>

int main()
{
    int win,b ;
    float x=0,y=0 ;
    /* gsetinitialattributes(DISABLE, BOTTOMLEFTORIGIN) ; */
    win = gopen(400,300) ;
    gsetbgcolor(win,"blue4") ;
    gclr(win) ;
    
    puts("Click the window or type your keyboard :-)") ;

    pset(win,0,0) ;
    pset(win,399,0) ;
    pset(win,0,299) ;
    pset(win,399,299) ;

    while( 1 ){
	int type ;
	if ( ggetxpress(&type,&b,&x,&y) == win ) {
	    if( type == ButtonPress ){
		newpen(win,b) ;
		moveto(win,0,y) ;
		lineto(win,399,y) ;
		moveto(win,x,0) ;
		lineto(win,x,299) ;
		drawstr(win,x,y,14,0,"%g %g",x,y) ;
		printf("button=%d x=%g y=%g\n",b,x,y) ;
	    }
	    else if( type == KeyPress ){
		if( b=='q' ) break ;
		drawstr(win,x,y-14,14,0,"%c",b) ;
		printf("key code = %d\n",b) ;
		x+=7 ;
	    }
	}
    }

    gclose(win) ;
    return(0) ;
}
