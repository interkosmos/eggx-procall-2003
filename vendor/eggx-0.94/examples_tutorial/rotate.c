/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-02-17 22:02:37 cyamauch> */

/*
 楕円や星型を回転させるサンプルコード

 京都産業大学の安田豊様のWebページ:
   http://ylb.jp/Cguide/_eggx_samples.html
 からいただいたものを編集しました．
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eggx.h>

#define AREA_WIDTH 400
#define AREA_HEIGHT 400 

/*
 * eclipse
 * 座標 (x, y) を中心に、幅 w, 高さ h の楕円を angle （ラジアン単位）
 * だけ回転させて描く
 * 楕円は非常に頂点数が多い多角形として fillpoly 関数を用いて描く
 */
#define ECLIPSEPOINTS 90 // 90 角形として描く
void eclipse(int win, float x, float y, float w, float h, float angle)
{
  float ptx[ECLIPSEPOINTS], pty[ECLIPSEPOINTS]; // 頂点座標位置
  float th; // 楕円の各頂点位置を求める際の角
  float xx, yy; // 元の回転させない楕円の各頂点位置
  int i;

  /* 各頂点の座標位置を得る */
  for(i=0; i<ECLIPSEPOINTS; i++) {
    th = 2*M_PI * ((float)i/ECLIPSEPOINTS);
    xx=cos(th)*w;
    yy=sin(th)*h;
    ptx[i] = x + xx * cos(angle) - yy * sin(angle);
    pty[i] = y + xx * sin(angle) + yy * cos(angle);
  }
  /* 描画 */
  fillpoly(win, ptx, pty, ECLIPSEPOINTS, 0);
}
#undef ECLIPSEPOINTS

/*
 * rotateX , rotateY
 * 座標 (posX, posY) を、(orgX, orgY) を中心に angle 度だけ回転させた
 * 座標位置の X, Y 成分を得る
 */
float rotateX(float orgX, float orgY, float posX, float posY, float angle)
{
  float x, y; // (orgX, orgY) を原点とした場合の (posX, posY) 座標位置 
  float rotX; // (x, y) を angle 度回転させた座標位置（の X 成分）
  x = posX - orgX;
  y = posY - orgY;
  rotX = x * cos(angle) - y * sin(angle);
  return rotX + orgX; // 戻り値には orgX の値を加えておく
}

float rotateY(float orgX, float orgY, float posX, float posY, float angle)
{
  float x, y; // (orgX, orgY) を原点とした場合の (posX, posY) 座標位置 
  float rotY; // (x, y) を angle 度回転させた座標位置（の Y 成分）
  x = posX - orgX;
  y = posY - orgY;
  rotY = x * sin(angle) + y * cos(angle);
  return rotY + orgY; // 戻り値には orgX の値を加えておく
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

  /* レイヤの設定をする */
  layer(win, 0,1);
  /* ノンブロックモードにする */
  gsetnonblock(ENABLE);

  theta = 0.0;

  while (1) {

    gclr(win);

    /*
     * エリア1: 塗り潰された楕円を描く
     */
    coordinate(win, 0,0, 0,0, 1.0,1.0);
    // 座標軸を描く
    newpen(win, 9); 
    newlinestyle(win, LineOnOffDash);
    drawline(win, 0.0, AREA_HEIGHT/2, AREA_WIDTH, AREA_HEIGHT/2); 
    drawline(win, AREA_WIDTH/2, 0.0, AREA_WIDTH/2, AREA_HEIGHT); 
    newlinestyle(win, LineSolid);

    newpen(win, 2);  // 赤色に設定
    eclipse(win, 200.0, 100.0, 100.0, 50.0, 0.0);     // 回転なし
    newpen(win, 3);  // 緑色に設定
    eclipse(win, 200.0, 200.0, 100.0, 50.0, theta);   // 回転あり

    /*
     * エリア2: 楕円を描く
     */
    coordinate(win, AREA_WIDTH,0, 0,0, 1.0,1.0);

#define POLYNUM 90 // 90 角形として描く
    {
      float posx[POLYNUM], posy[POLYNUM]; // 頂点位置座標配列
      float newx[POLYNUM], newy[POLYNUM]; // 回転させた後の座標位置配列
      float th;
      int i;
      // 座標軸を描く
      newpen(win, 1); 
      newlinestyle(win, LineOnOffDash);
      drawline(win, 0.0, AREA_HEIGHT/2, AREA_WIDTH, AREA_HEIGHT/2); 
      drawline(win, AREA_WIDTH/2, 0.0, AREA_WIDTH/2, AREA_HEIGHT); 
      newlinestyle(win, LineSolid);

      // 各点の位置を計算して平たい楕円を描く

      // 正常位置
      newpen(win, 2);  // 赤色に設定
      for(i=0; i<POLYNUM; i++) {
	th = 2*M_PI * ((float)i/POLYNUM);
	posx[i] = 250.0 + cos(th)*100.0;
	posy[i] = 150.0 + sin(th)*50.0;
      }
      drawpoly(win, posx, posy, POLYNUM);

      // 回転後の位置に打点
      newpen(win, 3);  // 緑色に設定
      for(i=0; i<POLYNUM; i++) {
	/* (200,200) を中心に、(x,y) を回転させた打点位置を計算*/
	newx[i] = rotateX(200.0, 200.0, posx[i], posy[i], theta); // 回転あり
	newy[i] = rotateY(200.0, 200.0, posx[i], posy[i], theta);
      }
      drawpoly(win, newx, newy, POLYNUM);
    }
#undef POLYNUM

    /*
     * エリア3: 長方形を描く
     */
    coordinate(win, 0,AREA_HEIGHT, 0,0, 1.0,1.0);

    {
      float posx[4], posy[4]; // 四角形の頂点位置座標配列
      float newx[4], newy[4]; // 回転させた後の座標位置配列
      int i;
      // 座標軸を描く
      newpen(win, 1); 
      newlinestyle(win, LineOnOffDash);
      drawline(win, 0.0, AREA_HEIGHT/2, AREA_WIDTH, AREA_HEIGHT/2); 
      drawline(win, AREA_WIDTH/2, 0.0, AREA_WIDTH/2, AREA_HEIGHT); 
      newlinestyle(win, LineSolid);

      // 適当な長方形の各頂点座標位置を決めて配列に格納
      posx[0]=150.0;    posy[0]=100.0;
      posx[1]=320.0;    posy[1]=100.0;
      posx[2]=320.0;    posy[2]=140.0;
      posx[3]=150.0;    posy[3]=140.0;  

      newpen(win, 2);  // 赤色に設定
      fillpoly(win, posx, posy, 4, 0); // 長方形を描く

      /* (200,200) を中心に、全頂点について回転させた位置を計算して */
      /* 別の配列に格納                                             */
      for(i=0; i<4; i++) {
	newx[i] = rotateX(200.0, 200.0, posx[i], posy[i], theta); // 回転あり
	newy[i] = rotateY(200.0, 200.0, posx[i], posy[i], theta);
      }
      newpen(win, 3);  // 緑色に設定
      fillpoly(win, newx, newy, 4, 0); // 回転後の長方形を描く
    }

    /*
     * エリア4: 星型を描く
     */
    coordinate(win, AREA_WIDTH,AREA_HEIGHT, 0,0, 1.0,1.0);

#define POLYNUM 12 // 多角形の角の数
#define LENGTH 150.0
    {
      float posx[POLYNUM], posy[POLYNUM]; // 元の頂点位置座標配列
      float newx[POLYNUM], newy[POLYNUM]; // 回転させた後の座標位置配列
      float th, l; // 作業用変数
      float x,y;
      int i;
      x=200.0;
      y=200.0;

      /* 各頂点座標位置を決めて配列に格納 */
      for(i=0;i<POLYNUM;i++) {
	th=2.0*M_PI/(float)POLYNUM*(float)i;
	if(i%2==0) { // 頂点番号が偶数の時は外郭
	  l=LENGTH;
	} else { // 奇数の時は内郭
	  l=LENGTH * 0.4; 
	}
	posx[i]=cos(th)*l + x;
	posy[i]=sin(th)*l + y;
      }

      newpen(win, 1);  // 白色に設定

      for(i=0;i<POLYNUM;i++) { // 頂点ごとに th だけ回転させた座標位置を求める
	newx[i]=rotateX(x, y, posx[i], posy[i], theta);
	newy[i]=rotateY(x, y, posx[i], posy[i], theta);
      }
      fillpoly(win, newx, newy, POLYNUM, 0); // 多角形を描く
    }
#undef LENGTH
#undef POLYNUM

    theta += M_PI/90.0; // 少しだけ回転を進める
    if ( M_PI * 2 < theta ) theta -= M_PI * 2;

    copylayer(win,1,0);
    msleep(20);

    if ( 0 < ggetch() ) break; // 何かキーを押すと終了

  }	/* while (1) */

  gclose(win); 

  return 0;
}
