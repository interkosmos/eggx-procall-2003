/* egg nihongo.c -o nihongo -Wall */
/* 日本語表示のテスト */
#include <eggx.h>

int main()
{
    int x = 4, y = 4;
    int win;
    win = gopen(270,139);

    drawstr(win,x,y, FONTSET,0, "デフォルト日本語フォント");
    y += 16;

    drawstr(win,x,y, 14,0, "14-dot ASCII font");
    y += 16;

    newfontset(win, "-*-fixed-medium-r-normal--14-*");
    drawstr(win,x,y, FONTSET,0, "14ドット日本語フォント");
    y += 16;

    drawstr(win,x,y, 16,0, "16-dot ASCII font");
    y += 18;

    newfontset(win, "-*-fixed-medium-r-normal--16-*");
    drawstr(win,x,y, FONTSET,0, "16ドット日本語フォント");
    y += 18;

    drawstr(win,x,y, 24,0, "24-dot ASCII font");
    y += 26;

    newfontset(win, "-*-fixed-medium-r-normal--24-*");
    drawstr(win,x,y, FONTSET,0, "24ドット日本語フォント");
    y += 26;
    
    ggetch();
    gclose(win);

    return 0;
}
