/* egg inkey.c -o inkey -Wall */
#include <eggx.h>

#define FNTSZ 24		/* size of font */
#define FOFFSET 2
#define COL 40			/* length of max key inputs */
#define XSIZE FNTSZ/2*COL	/* size of window */
#define YSIZE FNTSZ*2+FOFFSET

int main()
{
    int wn,wn1 ;
    int i ;

    /* open two graphic windows */
    wn1=gopen(10*FNTSZ/2,FNTSZ+FOFFSET) ;
    wn=gopen(XSIZE,YSIZE) ;
    /* set name of windows */
    winname(wn1,"Key Code") ;
    winname(wn,"Test of Keyboard") ;
    /* print a message */
    drawstr(wn,0,FNTSZ+FOFFSET,FNTSZ,0,"[Type your keyboard!]") ; 
    /* keyboard input... */
    for( i=0 ; i<COL ; i++ ){
	int c=ggetch() ;
	if( 0x020 <= c && c <= 0x07f ){
	    char str[2] = " " ;
	    str[0] = (char)c ;
	    drawstr(wn,i*FNTSZ/2,FOFFSET,FNTSZ,0,str) ;
	}
	gclr(wn1) ;
	drawstr(wn1,0,FOFFSET,FNTSZ,0,"%3d(0x0%02x)",c,c) ;
    }
    /* close graphic windows */
    gclose(wn1) ;
    gclose(wn) ;

    return(0) ;
}
