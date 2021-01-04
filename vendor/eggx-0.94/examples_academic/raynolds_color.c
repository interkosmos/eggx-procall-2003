/*
 群れをシミュレートする
 */

#include  <math.h>
#include  <stdlib.h>
#include  <stdio.h>
#include  <time.h>
#include  <eggx.h>

#define WW 600 // 画面の幅
#define HH 600 // 画面の高さ
#define TH 40  // 壁の位置（跳ね返る場所）
#define RR 9   // 円の直径
#define SS 8.0   // 標準的スピード
#define DD 30.0  // 群れの半径

#define MM 300 // 座標点の上限数
double x[MM], y[MM];    // 座標点を保存するための配列
double xd[MM], yd[MM];  // 速度を保存するための配列
double xk[MM], yk[MM];  // 次回の速度を保存するための配列
int    c[MM];           // 色
int mm;                // 座標点が幾つあったかを数えておく
int interval;          // ミリ秒単位の待ち時間

int win;

// 符合を返すだけの関数
int sig(double a, double b)
{
  if(a>=b) {
    return 1;
  } else {
    return -1;
  }
}

// 0 〜 ratio までの乱数を作る
double rnd(double ratio)
{
  return ( rand()%10000 ) / 10000.0 * ratio;
}

// 現在位置に描く
int drawit(int i) 
{
  double ratio;
  newhsvcolor(win, 0, 0, 200);
  ratio=(RR*3.0)/hypot(xk[i],yk[i]);
  moveto(win,x[i]-RR/2.0,            y[i]-RR/2.0            );
  lineto(win,x[i]-RR/2.0-xk[i]*ratio,y[i]-RR/2.0-yk[i]*ratio);

  newhsvcolor(win, c[i], 255, 255);
  fillarc(win,x[i]-RR/2.0,y[i]-RR/2.0,RR,RR,0.0,360.0,1) ;

  newhsvcolor(win, 0, 0, 128);
  circle(win,x[i]-RR/2.0,y[i]-RR/2.0,RR,RR);

  return 0;
}


// 次の移動位置を計算する
int moveit(int i) 
{
  int j, num;
  double xx, yy, dist, limit, kick, ratio;
  
  // 次回速度を前回速度を元にはじめる
  xk[i] = xd[i];
  yk[i] = yd[i];
  
  // 群れ全体の平均的移動ベクトルに沿う移動ベクトルを算出
  limit=DD; // チェックするのは群れ全体つまり DD 
  ratio=0.6+rnd(0.6);    // 追従率（ブレつき）
  num=0; xx=0.0; yy=0.0;
  for(j=0;j<mm;j++) {
    if(j==i) continue; // 自分自身とは相互作用無し！
    dist=hypot(x[i]-x[j], y[i]-y[j]);
	if(dist<limit) { // 閾値以下の距離にいる（群れの対象である）
      xx+=xd[j]; // 移動ベクトルを積算
      yy+=yd[j];
	  num++;
	}
  }
  if(num!=0) { 
    xx = xx / num; // 平均移動ベクトルを得る（ただし誤差つき）
    yy = yy / num;
    xx = xd[i] + ( xx - xd[i] ); // 現在のベクトルを補正する成分を得る
    yy = yd[i] + ( yy - yd[i] );
    xx *= (SS / hypot(xx, yy)) * ratio; // ある程度速度を保つ（停止する！）
    yy *= (SS / hypot(xx, yy)) * ratio; // 但し ratio で影響度を減らす
    xk[i]=xd[i]*0.7+xx*0.3; // 元の速度にどの程度影響を与えるかを
    yk[i]=yd[i]*0.7+yy*0.3; // 適当に決める
  }
  
  // これ以上近づかないための移動ベクトルを算出
  limit=DD*0.6; // 反発をはじめる限界値（閾値）は DD の半分
  ratio=0.6+rnd(1.4);    // 反発率
  num=0; xx=0.0; yy=0.0;
  for(j=0;j<mm;j++) {
    if(j==i) continue; // 自分自身とは相互作用無し！
    dist=hypot(x[i]-x[j], y[i]-y[j]);
	if(dist<limit) { // 閾値以下の距離にいる（近すぎる）
	  kick=1.0-(dist/limit); // どれだけ反発(kickback)するか
      xx+=(x[i]-x[j])*kick; // 移動ベクトルを積算
      yy+=(y[i]-y[j])*kick;
	  num++;
	}
  }
  if(num!=0) {
    xk[i] += xx * ratio / num; // 影響度を ratio で下げる
    yk[i] += yy * ratio / num;
  }

  // 群れ全体の平均的な位置（中心位置）に向かう移動ベクトルを算出
  limit=DD; // チェックするのは群れ全体つまり DD 
  ratio=0.01+rnd(0.05);    // 追従率
  num=0; xx=0.0; yy=0.0;
  for(j=0;j<mm;j++) {
    if(j==i) continue; // 自分自身とは相互作用無し！
    dist=hypot(x[i]-x[j], y[i]-y[j]);
	if(dist<limit) { // 閾値以下の距離にいる（群れの対象である）
      xx+=x[j]; // 位置を積算
      yy+=y[j];
	  num++;
	}
  }
  if(num!=0) { // 平均位置を得て、そこへ向かうベクトルを得る
    xx = ( xx / num ) - x[i];
    yy = ( yy / num ) - y[i];
    if( SS < hypot(xx, yy) ) { // 距離がある場合は長さを SS に切る 
      xx *= (SS / hypot(xx, yy));
      yy *= (SS / hypot(xx, yy));
	}
    xk[i] += xx * ratio; // 影響度を ratio で下げる
    yk[i] += yy * ratio;
  }
  
  // 全く関係なくブレを入れる
  xk[i] += SS * (-0.2 + rnd(0.4));
  yk[i] += SS * (-0.2 + rnd(0.4));


  return 0;
}

int main(int argc, char *argv[])
{
  int i;                // 一時的に使う変数（ループカウンタ）
  
  if(argc != 3) {
    fprintf(stderr, "usage: %s num wait\n",argv[0]);
    return 1;
  }
  mm=atoi(argv[1]);
  if((mm<1)||(mm>MM)) {
    fprintf(stderr, "usage: %s num wait\n",argv[0]);
    fprintf(stderr, "     : given num (%s) is not from 1 to %d\n",
            argv[1], MM);
    return 1;
  }
  interval=atoi(argv[2]);
  if((interval<1)||(interval>1000)) {
    fprintf(stderr, "usage: %s num msec\n",argv[0]);
    fprintf(stderr, "     : given msec (%s) is not from 1 to 1000\n",
            argv[2]);
    return 1;
  }
  
  // 描画準備
  win=gopen(WW,HH);
  winname(win, "Raynolds");
  layer(win, 0, 1); // 表示は 0 番、描画は 1 番レイヤーで

  srand(time(NULL));
  // 配列にいったん格納する（ m が座標点の個数）
  for(i=0;i<mm;i++) {
    double r;
    x[i]=rand()%(WW/3)+WW/3;
    y[i]=rand()%(HH/3)+HH/3;
    r=((rand()%100)/100.0)*(3.14159*2);
    xd[i]=SS*cos(r);
    yd[i]=SS*sin(r);
	c[i]=(double)(i) / (double)(mm) * 360;
  }

  while(1) {
    // 速度（＝移動距離）計算をする
    for(i=0; i<mm; i++) {
      moveit(i);
//      printf("%3d = %10.8f %f,%f\n", i, l, xd[i], yd[i]);
    }

    // 配列に納めたぶんだけ移動し、描く
    gclr(win);
	newhsvcolor(win, 0, 0, 128);
	drawrect(win, TH, TH, WW-2*TH, HH-2*TH);
    for(i=0; i<mm; i++) {
      x[i]+=xk[i]; y[i]+=yk[i]; // 移動
      drawit(i); // 描画
	  xd[i]=xk[i]; yd[i]=yk[i]; // 移動距離を保存
	  if(x[i]<TH)      xd[i]=fabs(xd[i]);
	  if(x[i]>(WW-TH)) xd[i]=fabs(xd[i]) * -1.0;
	  if(y[i]<TH)      yd[i]=fabs(yd[i]);
	  if(y[i]>(HH-TH)) yd[i]=fabs(yd[i]) * -1.0;
    }
    copylayer(win, 1, 0); // レイヤー 1 番の内容を 0 にコピー
    msleep(interval);
  }
      
// never stop

}
