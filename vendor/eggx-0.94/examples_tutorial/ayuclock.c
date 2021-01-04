#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <eggx.h>
#include "ayu.h"

int main( int argc, char *argv[] )
{
    time_t time_now ;
    int win,i ;
    char d0[8],d1[8],d2[8],d3[8], t0[16] ;
    char *geometry=NULL ;

    for( i=1 ; i<argc ; i++ ){
	if( argv[i+1] != NULL ){
	    if( strcmp(argv[i],"-position")==0 ){
		geometry=argv[i+1] ;
	    }
	}
    }
    /* gsetinitialgeometry("%+d%+d",-30,40) ; */
    gsetinitialgeometry(geometry) ;
    gsetinitialwinname("AyuClock","AyuClock","ayuclock","AyuClock") ;
    gsetinitialattributes(ENABLE, DOCK_APPLICATION) ;
    gsetinitialbgcolor("white") ;
    win = gopen(56,56) ;
    layer(win,0,1) ;
    newcolor(win,"black") ;
    while(1){
	time(&time_now) ;
	sscanf(ctime(&time_now),"%s %s %s %s %s\n",d0,d1,d2,t0,d3) ;
	gclr(win) ;
	gputimage(win,1,1,Ppmimage,PPM_WIDTH,PPM_HEIGHT,0) ;
	newcolor(win,"white") ;
	drawstr(win,3,1, 8 ,0,"%s %s %s",d0,d1,d2) ;
	drawstr(win,5,1, 8 ,0,"%s %s %s",d0,d1,d2) ;
	drawstr(win,4,0, 8 ,0,"%s %s %s",d0,d1,d2) ;
	drawstr(win,4,2, 8 ,0,"%s %s %s",d0,d1,d2) ;
	newcolor(win,"black") ;
	drawstr(win,4,1, 8 ,0,"%s %s %s",d0,d1,d2) ;
	drawstr(win,4,46, 10 ,0,"%s",t0) ;
	newrgbcolor(win,0x40,0x40,0x40) ;
	moveto(win,0,0) ;
	lineto(win,0,55) ;
	lineto(win,55,55) ;
	newrgbcolor(win,0xe7,0xe7,0xe7) ;
	lineto(win,55,0) ;
	lineto(win,0,0) ;
	copylayer(win,1,0) ;
	msleep(200) ;
    }
    gcloseall() ;
    return(0) ;
}
