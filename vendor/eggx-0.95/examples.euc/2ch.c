/* egg 2ch.c -o 2ch -Wall */
/* 2ch�������������ȤΥ���ץ� */
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
	    "���� �ʡ��ʡ���������������\n"
	    "�����ʡ����ϡ��ˡ㡡���ޥ���ʡ�\n"
	    "�����ʡ��������� ��������������\n"
	    "������ �á�|\n"
	    "������_���ˡ���") ;
    
    x=210 ;
    y=120-32 ;
    drawstr(win,x,y, FONTSET,0,
	    "�������������������������������������ʢ�\n"
	    "�������������������������ʢʡ�����(,,�ߧ���)���������ʢ�\n"
	    "�������������ʢʡ�����(,,�ߧ���)�� �������ġ����� (����,,)\n"
	    "���족��(,,�ߧ���)���� / �Ĥġ� ������|��������/����|\n"
	    "����UU��U U���� ���ʡ����ˡ��� �����ʡ������ʡ�����") ;
    
    ggetch() ;
    gclose(win) ;
    return(0) ;
}