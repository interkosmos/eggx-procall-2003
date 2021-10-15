/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-02-17 22:02:37 cyamauch> */

/*
 �ʱߤ��������ž�����륵��ץ륳����

 ���Ի�����ؤΰ���˭�ͤ�Web�ڡ���:
   http://ylb.jp/Cguide/_eggx_samples.html
 ���餤����������Τ��Խ����ޤ�����
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eggx.h>

#define AREA_WIDTH 400
#define AREA_HEIGHT 400 

/*
 * eclipse
 * ��ɸ (x, y) ���濴�ˡ��� w, �⤵ h ���ʱߤ� angle �ʥ饸����ñ�̡�
 * ������ž����������
 * �ʱߤ�����ĺ������¿��¿�ѷ��Ȥ��� fillpoly �ؿ����Ѥ�������
 */
#define ECLIPSEPOINTS 90 // 90 �ѷ��Ȥ�������
void eclipse(int win, float x, float y, float w, float h, float angle)
{
  float ptx[ECLIPSEPOINTS], pty[ECLIPSEPOINTS]; // ĺ����ɸ����
  float th; // �ʱߤγ�ĺ�����֤����ݤγ�
  float xx, yy; // ���β�ž�����ʤ��ʱߤγ�ĺ������
  int i;

  /* ��ĺ���κ�ɸ���֤����� */
  for(i=0; i<ECLIPSEPOINTS; i++) {
    th = 2*M_PI * ((float)i/ECLIPSEPOINTS);
    xx=cos(th)*w;
    yy=sin(th)*h;
    ptx[i] = x + xx * cos(angle) - yy * sin(angle);
    pty[i] = y + xx * sin(angle) + yy * cos(angle);
  }
  /* ���� */
  fillpoly(win, ptx, pty, ECLIPSEPOINTS, 0);
}
#undef ECLIPSEPOINTS

/*
 * rotateX , rotateY
 * ��ɸ (posX, posY) ��(orgX, orgY) ���濴�� angle �٤�����ž������
 * ��ɸ���֤� X, Y ��ʬ������
 */
float rotateX(float orgX, float orgY, float posX, float posY, float angle)
{
  float x, y; // (orgX, orgY) �����Ȥ������� (posX, posY) ��ɸ���� 
  float rotX; // (x, y) �� angle �ٲ�ž��������ɸ���֡ʤ� X ��ʬ��
  x = posX - orgX;
  y = posY - orgY;
  rotX = x * cos(angle) - y * sin(angle);
  return rotX + orgX; // ����ͤˤ� orgX ���ͤ�ä��Ƥ���
}

float rotateY(float orgX, float orgY, float posX, float posY, float angle)
{
  float x, y; // (orgX, orgY) �����Ȥ������� (posX, posY) ��ɸ���� 
  float rotY; // (x, y) �� angle �ٲ�ž��������ɸ���֡ʤ� Y ��ʬ��
  x = posX - orgX;
  y = posY - orgY;
  rotY = x * sin(angle) + y * cos(angle);
  return rotY + orgY; // ����ͤˤ� orgX ���ͤ�ä��Ƥ���
}

/*
 * main
 */
int main()
{
  int win;
  float theta;

  win = gopen(AREA_WIDTH*2, AREA_WIDTH*2);  
  winname(win, "rotation");

  /* �쥤�������򤹤� */
  layer(win, 0,1);
  /* �Υ�֥�å��⡼�ɤˤ��� */
  gsetnonblock(ENABLE);

  theta = 0.0;

  while (1) {

    gclr(win);

    /*
     * ���ꥢ1: �ɤ��٤��줿�ʱߤ�����
     */
    coordinate(win, 0,0, 0,0, 1.0,1.0);
    // ��ɸ��������
    newpen(win, 9); 
    newlinestyle(win, LineOnOffDash);
    drawline(win, 0.0, AREA_HEIGHT/2, AREA_WIDTH, AREA_HEIGHT/2); 
    drawline(win, AREA_WIDTH/2, 0.0, AREA_WIDTH/2, AREA_HEIGHT); 
    newlinestyle(win, LineSolid);

    newpen(win, 2);  // �ֿ�������
    eclipse(win, 200.0, 100.0, 100.0, 50.0, 0.0);     // ��ž�ʤ�
    newpen(win, 3);  // �п�������
    eclipse(win, 200.0, 200.0, 100.0, 50.0, theta);   // ��ž����

    /*
     * ���ꥢ2: �ʱߤ�����
     */
    coordinate(win, AREA_WIDTH,0, 0,0, 1.0,1.0);

#define POLYNUM 90 // 90 �ѷ��Ȥ�������
    {
      float posx[POLYNUM], posy[POLYNUM]; // ĺ�����ֺ�ɸ����
      float newx[POLYNUM], newy[POLYNUM]; // ��ž��������κ�ɸ��������
      float th;
      int i;
      // ��ɸ��������
      newpen(win, 1); 
      newlinestyle(win, LineOnOffDash);
      drawline(win, 0.0, AREA_HEIGHT/2, AREA_WIDTH, AREA_HEIGHT/2); 
      drawline(win, AREA_WIDTH/2, 0.0, AREA_WIDTH/2, AREA_HEIGHT); 
      newlinestyle(win, LineSolid);

      // �����ΰ��֤�׻�����ʿ�����ʱߤ�����

      // �������
      newpen(win, 2);  // �ֿ�������
      for(i=0; i<POLYNUM; i++) {
	th = 2*M_PI * ((float)i/POLYNUM);
	posx[i] = 250.0 + cos(th)*100.0;
	posy[i] = 150.0 + sin(th)*50.0;
      }
      drawpoly(win, posx, posy, POLYNUM);

      // ��ž��ΰ��֤�����
      newpen(win, 3);  // �п�������
      for(i=0; i<POLYNUM; i++) {
	/* (200,200) ���濴�ˡ�(x,y) ���ž�������������֤�׻�*/
	newx[i] = rotateX(200.0, 200.0, posx[i], posy[i], theta); // ��ž����
	newy[i] = rotateY(200.0, 200.0, posx[i], posy[i], theta);
      }
      drawpoly(win, newx, newy, POLYNUM);
    }
#undef POLYNUM

    /*
     * ���ꥢ3: Ĺ����������
     */
    coordinate(win, 0,AREA_HEIGHT, 0,0, 1.0,1.0);

    {
      float posx[4], posy[4]; // �ͳѷ���ĺ�����ֺ�ɸ����
      float newx[4], newy[4]; // ��ž��������κ�ɸ��������
      int i;
      // ��ɸ��������
      newpen(win, 1); 
      newlinestyle(win, LineOnOffDash);
      drawline(win, 0.0, AREA_HEIGHT/2, AREA_WIDTH, AREA_HEIGHT/2); 
      drawline(win, AREA_WIDTH/2, 0.0, AREA_WIDTH/2, AREA_HEIGHT); 
      newlinestyle(win, LineSolid);

      // Ŭ����Ĺ�����γ�ĺ����ɸ���֤��������˳�Ǽ
      posx[0]=150.0;    posy[0]=100.0;
      posx[1]=320.0;    posy[1]=100.0;
      posx[2]=320.0;    posy[2]=140.0;
      posx[3]=150.0;    posy[3]=140.0;  

      newpen(win, 2);  // �ֿ�������
      fillpoly(win, posx, posy, 4, 0); // Ĺ����������

      /* (200,200) ���濴�ˡ���ĺ���ˤĤ��Ʋ�ž���������֤�׻����� */
      /* �̤�����˳�Ǽ                                             */
      for(i=0; i<4; i++) {
	newx[i] = rotateX(200.0, 200.0, posx[i], posy[i], theta); // ��ž����
	newy[i] = rotateY(200.0, 200.0, posx[i], posy[i], theta);
      }
      newpen(win, 3);  // �п�������
      fillpoly(win, newx, newy, 4, 0); // ��ž���Ĺ����������
    }

    /*
     * ���ꥢ4: ����������
     */
    coordinate(win, AREA_WIDTH,AREA_HEIGHT, 0,0, 1.0,1.0);

#define POLYNUM 12 // ¿�ѷ��γѤο�
#define LENGTH 150.0
    {
      float posx[POLYNUM], posy[POLYNUM]; // ����ĺ�����ֺ�ɸ����
      float newx[POLYNUM], newy[POLYNUM]; // ��ž��������κ�ɸ��������
      float th, l; // ������ѿ�
      float x,y;
      int i;
      x=200.0;
      y=200.0;

      /* ��ĺ����ɸ���֤��������˳�Ǽ */
      for(i=0;i<POLYNUM;i++) {
	th=2.0*M_PI/(float)POLYNUM*(float)i;
	if(i%2==0) { // ĺ���ֹ椬�����λ��ϳ���
	  l=LENGTH;
	} else { // ����λ������
	  l=LENGTH * 0.4; 
	}
	posx[i]=cos(th)*l + x;
	posy[i]=sin(th)*l + y;
      }

      newpen(win, 1);  // �򿧤�����

      for(i=0;i<POLYNUM;i++) { // ĺ�����Ȥ� th ������ž��������ɸ���֤����
	newx[i]=rotateX(x, y, posx[i], posy[i], theta);
	newy[i]=rotateY(x, y, posx[i], posy[i], theta);
      }
      fillpoly(win, newx, newy, POLYNUM, 0); // ¿�ѷ�������
    }
#undef LENGTH
#undef POLYNUM

    theta += M_PI/90.0; // ����������ž��ʤ��
    if ( M_PI * 2 < theta ) theta -= M_PI * 2;

    copylayer(win,1,0);
    msleep(20);

    if ( 0 < ggetch() ) break; // ���������򲡤��Ƚ�λ

  }	/* while (1) */

  gclose(win); 

  return 0;
}
