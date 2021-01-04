/* egg 2ch.c -o 2ch -Wall */
/* 2ch¥¢¥¹¥­¡¼¥¢¡¼¥È¤Î¥µ¥ó¥×¥ë */
#include <eggx.h>

int main()
{
    int win ;
    int x,y ;
    win = gopen(560,120) ;

    newfontset(win,
		"-mona-*-medium-r-normal-*-16-*-*-*-*-*-*-*") ;
    x=0 ;
    y=120-32 ;

    drawstr(win,x,y, FONTSET,0,
	    "¡¡¡¡ ¢Ê¡²¢Ê¡¡¡¡¡¿¡±¡±¡±¡±¡±\n"
	    "¡¡¡¡¡Ê¡¡¡­¢Ï¡®¡Ë¡ã¡¡¥ª¥Þ¥¨¥â¥Ê¡¼\n"
	    "¡¡¡¡¡Ê¡¡¡¡¡¡¡¡¡Ë ¡¡¡À¡²¡²¡²¡²¡²\n"
	    "¡¡¡¡¡Ã ¡Ã¡¡|\n"
	    "¡¡¡¡¡Ê_¡²¡Ë¡²¡Ë") ;
    
    x=210 ;
    y=120-32 ;
    drawstr(win,x,y, FONTSET,0,
	    "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¢Ê¢Ê\n"
	    "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¢Ê¢Ê¡¡¡¡¡¡(,,Žß§¥Žß)¡¡¡¡¡¡¡¡¢Ê¢Ê\n"
	    "¡¡¡¡¡¡¡¡¡¡¡¡¢Ê¢Ê¡¡¡¡¡¡(,,Žß§¥Žß)¡¡ ¢¾¡¡¡¡¤Ä¡¡¡¡¡¡ (¡¡¡¡,,)\n"
	    "¡Á¡ì¡±¡±(,,Žß§¥Žß)¡¡¡¡ / ¤Ä¤Ä¡¡ ¡Á¡¡¡¡|¡¡¡¡¡¡¡¡/¡¡¡¡|\n"
	    "¡¡¡¡UU¡±U U¡¡¡¡ ¡Á¡Ê¡²¡²¡Ë¡¡¡¡ ¤·¡®£Ê¡¡¡¡¡Á¡Ê¡²¡²¡Ë") ;
    
    ggetch() ;
    gclose(win) ;
    return(0) ;
}
