/* egg plamoclock.c -o plamoclock -Wall */
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <eggx.h>
#include "plamoclock.h"

int main()
{
    time_t time_now ;
    int win ;
    char d0[8],d1[8],d2[8],d3[8], t0[16] ;

    gsetinitialattributes(ENABLE,DOCK_APPLICATION) ;
    win = gopen(56,56) ;
    layer(win,0,1) ;
    while(1){
	time(&time_now) ;
	sscanf(ctime(&time_now),"%s %s %s %s %s\n",d0,d1,d2,t0,d3) ;
	gclr(win) ;
	newrgbcolor(win,0xff,0xff,0xff) ;
	drawstr(win,4,1, 8 ,0,"%s %s %s",d0,d1,d2) ;
	drawstr(win,4,46, 10 ,0,"%s",t0) ;
	gputimage(win,0,10,Ppmimage,PPM_WIDTH,PPM_HEIGHT,0) ;
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
