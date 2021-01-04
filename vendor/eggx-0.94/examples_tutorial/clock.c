/* egg clock.c -o clock -Wall */
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <eggx.h>

#define FONTSIZE 20

int main()
{
    int win ;
    time_t time_now ;
    char d0[8],d1[8],d2[8],d3[8], t0[16] ;

    /* open a graphic window */
    win = gopen(FONTSIZE*24/2+4,FONTSIZE+4) ;
    /* set name of window */
    winname(win,"Simple Clock") ;
    /* configure layer */
    layer(win,0,1) ;
    /* display time... */
    while(1){
        time(&time_now) ;
        sscanf(ctime(&time_now),"%s %s %s %s %s",d0,d1,d2,t0,d3) ;
	gclr(win) ;
	drawstr(win,2,2,FONTSIZE,0,"%s %s %s %s %s",d0,d1,d2,t0,d3) ;
	copylayer(win,1,0) ;
	msleep(200) ;
    }
    /* close a graphic window */
    gclose(win) ;

    return(0) ;
}
