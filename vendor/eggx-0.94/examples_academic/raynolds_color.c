/*
 ����򥷥ߥ�졼�Ȥ���
 */

#include  <math.h>
#include  <stdlib.h>
#include  <stdio.h>
#include  <time.h>
#include  <eggx.h>

#define WW 600 // ���̤���
#define HH 600 // ���̤ι⤵
#define TH 40  // �ɤΰ��֡�ķ���֤����
#define RR 9   // �ߤ�ľ��
#define SS 8.0   // ɸ��Ū���ԡ���
#define DD 30.0  // �����Ⱦ��

#define MM 300 // ��ɸ���ξ�¿�
double x[MM], y[MM];    // ��ɸ������¸���뤿�������
double xd[MM], yd[MM];  // ®�٤���¸���뤿�������
double xk[MM], yk[MM];  // �����®�٤���¸���뤿�������
int    c[MM];           // ��
int mm;                // ��ɸ�������Ĥ��ä���������Ƥ���
int interval;          // �ߥ���ñ�̤��Ԥ�����

int win;

// �����֤������δؿ�
int sig(double a, double b)
{
  if(a>=b) {
    return 1;
  } else {
    return -1;
  }
}

// 0 �� ratio �ޤǤ��������
double rnd(double ratio)
{
  return ( rand()%10000 ) / 10000.0 * ratio;
}

// ���߰��֤�����
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


// ���ΰ�ư���֤�׻�����
int moveit(int i) 
{
  int j, num;
  double xx, yy, dist, limit, kick, ratio;
  
  // ����®�٤�����®�٤򸵤ˤϤ����
  xk[i] = xd[i];
  yk[i] = yd[i];
  
  // �������Τ�ʿ��Ū��ư�٥��ȥ�˱褦��ư�٥��ȥ�򻻽�
  limit=DD; // �����å�����ΤϷ������ΤĤޤ� DD 
  ratio=0.6+rnd(0.6);    // �ɽ�Ψ�ʥ֥�Ĥ���
  num=0; xx=0.0; yy=0.0;
  for(j=0;j<mm;j++) {
    if(j==i) continue; // ��ʬ���ȤȤ���ߺ���̵����
    dist=hypot(x[i]-x[j], y[i]-y[j]);
	if(dist<limit) { // ���Ͱʲ��ε�Υ�ˤ���ʷ�����оݤǤ����
      xx+=xd[j]; // ��ư�٥��ȥ���ѻ�
      yy+=yd[j];
	  num++;
	}
  }
  if(num!=0) { 
    xx = xx / num; // ʿ�Ѱ�ư�٥��ȥ������ʤ��������Ĥ���
    yy = yy / num;
    xx = xd[i] + ( xx - xd[i] ); // ���ߤΥ٥��ȥ������������ʬ������
    yy = yd[i] + ( yy - yd[i] );
    xx *= (SS / hypot(xx, yy)) * ratio; // ��������®�٤��ݤġ���ߤ��롪��
    yy *= (SS / hypot(xx, yy)) * ratio; // â�� ratio �Ǳƶ��٤򸺤餹
    xk[i]=xd[i]*0.7+xx*0.3; // ����®�٤ˤɤ����ٱƶ���Ϳ���뤫��
    yk[i]=yd[i]*0.7+yy*0.3; // Ŭ���˷���
  }
  
  // ����ʾ��Ť��ʤ�����ΰ�ư�٥��ȥ�򻻽�
  limit=DD*0.6; // ȿȯ��Ϥ����³��͡����͡ˤ� DD ��Ⱦʬ
  ratio=0.6+rnd(1.4);    // ȿȯΨ
  num=0; xx=0.0; yy=0.0;
  for(j=0;j<mm;j++) {
    if(j==i) continue; // ��ʬ���ȤȤ���ߺ���̵����
    dist=hypot(x[i]-x[j], y[i]-y[j]);
	if(dist<limit) { // ���Ͱʲ��ε�Υ�ˤ���ʶ᤹�����
	  kick=1.0-(dist/limit); // �ɤ����ȿȯ(kickback)���뤫
      xx+=(x[i]-x[j])*kick; // ��ư�٥��ȥ���ѻ�
      yy+=(y[i]-y[j])*kick;
	  num++;
	}
  }
  if(num!=0) {
    xk[i] += xx * ratio / num; // �ƶ��٤� ratio �ǲ�����
    yk[i] += yy * ratio / num;
  }

  // �������Τ�ʿ��Ū�ʰ��֡��濴���֡ˤ˸�������ư�٥��ȥ�򻻽�
  limit=DD; // �����å�����ΤϷ������ΤĤޤ� DD 
  ratio=0.01+rnd(0.05);    // �ɽ�Ψ
  num=0; xx=0.0; yy=0.0;
  for(j=0;j<mm;j++) {
    if(j==i) continue; // ��ʬ���ȤȤ���ߺ���̵����
    dist=hypot(x[i]-x[j], y[i]-y[j]);
	if(dist<limit) { // ���Ͱʲ��ε�Υ�ˤ���ʷ�����оݤǤ����
      xx+=x[j]; // ���֤��ѻ�
      yy+=y[j];
	  num++;
	}
  }
  if(num!=0) { // ʿ�Ѱ��֤����ơ������ظ������٥��ȥ������
    xx = ( xx / num ) - x[i];
    yy = ( yy / num ) - y[i];
    if( SS < hypot(xx, yy) ) { // ��Υ���������Ĺ���� SS ���ڤ� 
      xx *= (SS / hypot(xx, yy));
      yy *= (SS / hypot(xx, yy));
	}
    xk[i] += xx * ratio; // �ƶ��٤� ratio �ǲ�����
    yk[i] += yy * ratio;
  }
  
  // �����ط��ʤ��֥�������
  xk[i] += SS * (-0.2 + rnd(0.4));
  yk[i] += SS * (-0.2 + rnd(0.4));


  return 0;
}

int main(int argc, char *argv[])
{
  int i;                // ���Ū�˻Ȥ��ѿ��ʥ롼�ץ����󥿡�
  
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
  
  // �������
  win=gopen(WW,HH);
  winname(win, "Raynolds");
  layer(win, 0, 1); // ɽ���� 0 �֡������ 1 �֥쥤�䡼��

  srand(time(NULL));
  // ����ˤ��ä����Ǽ����� m ����ɸ���θĿ���
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
    // ®�١ʡ��ư��Υ�˷׻��򤹤�
    for(i=0; i<mm; i++) {
      moveit(i);
//      printf("%3d = %10.8f %f,%f\n", i, l, xd[i], yd[i]);
    }

    // �����Ǽ�᤿�֤������ư��������
    gclr(win);
	newhsvcolor(win, 0, 0, 128);
	drawrect(win, TH, TH, WW-2*TH, HH-2*TH);
    for(i=0; i<mm; i++) {
      x[i]+=xk[i]; y[i]+=yk[i]; // ��ư
      drawit(i); // ����
	  xd[i]=xk[i]; yd[i]=yk[i]; // ��ư��Υ����¸
	  if(x[i]<TH)      xd[i]=fabs(xd[i]);
	  if(x[i]>(WW-TH)) xd[i]=fabs(xd[i]) * -1.0;
	  if(y[i]<TH)      yd[i]=fabs(yd[i]);
	  if(y[i]>(HH-TH)) yd[i]=fabs(yd[i]) * -1.0;
    }
    copylayer(win, 1, 0); // �쥤�䡼 1 �֤����Ƥ� 0 �˥��ԡ�
    msleep(interval);
  }
      
// never stop

}
