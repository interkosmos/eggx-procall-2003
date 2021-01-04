/* egg mousetst2.c -o mousetst2 -Wall */
#include <stdio.h>
#include <eggx.h>
#include "rbomb.h"

int main()
{
    int win,win2,b ;
    int pentype = 4 ;
    float p_x=0,p_y=0,x=0,y=0 ;
    //gsetinitialgeometry("400x300") ;
    //gsetinitialgeometry("400x300+20-20") ;
    //gsetinitialgeometry("+40-20") ;
    win2 = gopen(100,100) ;
    win = gopen(400,300) ;
    /* window(win,399,299,00,00) ; */
    /* window(win,0,299,399,00) ; */
    gsetbgcolor(win,"blue4") ;
    gclr(win) ;
    drawstr(win2,16,56,14,0,"Click here") ;
    drawstr(win2,16,40,14,0," to clear.") ;
    
    puts("Click the window or type your keyboard :-)") ;

    while( 1 ){
	int win_ev;	/* イベントのあったウィンドゥ番号 */
	int type;	/* イベントのタイプ */
	win_ev = ggetevent(&type,&b,&x,&y) ;
	/* 小さい方のウィンドゥだった場合 */
	if ( win_ev == win2 ) {
	    if( type == ButtonPress ){
		if ( b == 1 ) gsetbgcolor(win,"gray4") ;
		else if ( b == 2 ) gsetbgcolor(win,"blue4") ;
		else if ( b == 3 ) gsetbgcolor(win,"red4") ;
 		gclr(win) ;
	    }
	    else if( type == KeyPress ){
		if( b=='q' ) break ;
	    }
	}
	/* 大きい方のウィンドゥだった場合 */
	else if ( win_ev == win ) {
	    if( type == MotionNotify ){
		if ( pentype == 4 ) {
		    gputimage(win, x-10, y-10,
		      Xpm_image_rbomb, XPM_WIDTH_RBOMB, XPM_HEIGHT_RBOMB, 255);
		} else if ( pentype == 1 ) {
		    newpen(win,7) ;
		    pset(win,x,y) ;
		} else if ( pentype == 2 ) {
		    gscroll(win,x-p_x,y-p_y,0);
		    p_x = x;
		    p_y = y;
		}
		printf("button=%d x=%g y=%g\n",b,x,y) ;
	    }
	    else if( type == ButtonPress ){
		pentype = b;
		printf("button=%d x=%g y=%g\n",b,x,y) ;
	    }
	    else if( type == KeyPress ){
		if( b=='q' ) break ;
		else if( b=='>' ) gresize(win,600,400);
		else if( b=='<' ) gresize(win,400,300);
		printf("key code = %d\n",b) ;
	    }
	}
    }

    gclose(win2) ;
    gclose(win) ;
    return(0) ;
}
