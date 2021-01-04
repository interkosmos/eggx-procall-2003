/* egg 2ch.c -o 2ch -Wall */
/* 2chアスキーアートのサンプル */
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
	    "　　 ∧＿∧　　／‾‾‾‾‾\n"
	    "　　（　´∀｀）＜　オマエモナー\n"
	    "　　（　　　　） 　＼＿＿＿＿＿\n"
	    "　　｜ ｜　|\n"
	    "　　（_＿）＿）") ;
    
    x=210 ;
    y=120-32 ;
    drawstr(win,x,y, FONTSET,0,
	    "　　　　　　　　　　　　　　　　　　∧∧\n"
	    "　　　　　　　　　　　　∧∧　　　(,,゜Д゜)　　　　∧∧\n"
	    "　　　　　　∧∧　　　(,,゜Д゜)　 ⊂　　つ　　　 (　　,,)\n"
	    "〜′‾‾(,,゜Д゜)　　 / つつ　 〜　　|　　　　/　　|\n"
	    "　　UU‾U U　　 〜（＿＿）　　 し｀Ｊ　　〜（＿＿）") ;
    
    ggetch() ;
    gclose(win) ;
    return(0) ;
}
