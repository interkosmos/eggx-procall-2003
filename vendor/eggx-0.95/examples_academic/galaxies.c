#include <math.h>
#include <eggx.h>

/* パラメータ */
#define T_MAX 16.0			/* 計算終了条件 */
#define T_PREC 0.005			/* ステップ幅 */
#define RATIO_F 0.5			/* 質量比 */
#define INITIAL_R 1.0			/* 銀河の初期値 */
#define INITIAL_V 0.1
#define PARAM_B 0.1			/* ポテンシャルのパラメータ */
#define NUM_OF_ALL_PARTICLES 510	/* 粒子数(2つの銀河あわせて) */
#define NUM_OF_RINGS 8			/* ディスクのリングの数 */
#define DISC_W_G0 0.05			/* ディスクのリング間隔 */
#define DISC_W_G1 0.03
#define ANGLE_I_G0 80.0			/* ディスクの傾斜 */
#define ANGLE_L_G0 120.0
#define ANGLE_I_G1 60.0
#define ANGLE_L_G1 30.0
/* ここは安直に変更不可 */
#define NUM_OF_DIMENSION 3
#define NUM_OF_EQUATIONS (NUM_OF_DIMENSION*2)
#define NUM_OF_GALAXIES  2
#define NUM_OF_ALL ((NUM_OF_ALL_PARTICLES+NUM_OF_GALAXIES)*NUM_OF_EQUATIONS)
/* グラフィックス */
#define WIN_SIZE 400		/* ウィンドゥサイズ */
#define X0 WIN_SIZE/2.0		/* 原点 */
#define Y0 WIN_SIZE/2.0
#define SX 100.0/1.0		/* 拡大率 */
#define SY SX
#define PLOT_G(wn,x,y) fillarc((wn),X0+(x)*SX,Y0+(y)*SY,2,2,0,360,1)
#define PLOT_P(wn,x,y) pset((wn),X0+(x)*SX,Y0+(y)*SY)

/* rngkut用構造体 */
struct rngkut_val {
    double f ;			/* 求める関数 */
    double df ;			/* 求める関数の微分 */
    double q ;			/* 作業用 */
} ;
struct rngkut_arg {
    int n ;			/* 求める関数の個数 */
    struct rngkut_val *v ;	/* 求める関数 */
    double h ;			/* ステップ幅 */
    double t ;			/* 微分をする変数 */
} ;
struct params {
    int n_dim ;
    int n_particles ;
    struct rngkut_val **vg ;	/* 銀河中心 */
    struct rngkut_val **vp ;	/* ディスクを表す粒子 */
    double *ratio ;
    double b ;
} ;

void dif_equation( struct rngkut_arg *, void * ) ;
void rngkut( struct rngkut_arg *, void *,
	     void (*)( struct rngkut_arg *, void * ) ) ;

int main()
{
    struct rngkut_val val[NUM_OF_ALL] ;
    struct rngkut_arg arg = { NUM_OF_ALL, val } ;
    struct rngkut_val *va_g[NUM_OF_GALAXIES] ;
    struct rngkut_val *va_p[NUM_OF_ALL_PARTICLES] ;
    double ratiof[NUM_OF_GALAXIES] ;
    int num_of_particles[NUM_OF_GALAXIES] ;
    double angle_i[] = {ANGLE_I_G0,ANGLE_I_G1} ;
    double angle_l[] = {ANGLE_L_G0,ANGLE_L_G1} ;
    double disc_w[]  = {DISC_W_G0,DISC_W_G1} ;
    struct params prms = {NUM_OF_DIMENSION,NUM_OF_ALL_PARTICLES,va_g,va_p,ratiof};
    int ii,i,j,k,win,sum_of_particles ;
    for( i=0 ; i<NUM_OF_GALAXIES ; i++ )
	va_g[i] = val + i*NUM_OF_EQUATIONS ;
    for( j=0 ; j<NUM_OF_ALL_PARTICLES ; j++,i++ )
	va_p[j] = val + i*NUM_OF_EQUATIONS ;
    arg.t = 0.0 ;
    arg.h = T_PREC ;
    prms.ratio[0]=1.0/(1.0+RATIO_F) ;
    prms.ratio[1]=RATIO_F/(1.0+RATIO_F) ;
    prms.b=PARAM_B ;
    /* ==== 初期条件 ==== */
    /* 銀河の中心 */
    for( i=0 ; i<NUM_OF_EQUATIONS ; i++ ) va_g[0][i].f=0 ;
    va_g[0][0].f=INITIAL_R ;
    va_g[0][4].f=INITIAL_V ;
    for( i=0 ; i<NUM_OF_EQUATIONS ; i++ ) 
	va_g[1][i].f=-va_g[0][i].f/RATIO_F ;
    /* 銀河のディスクを表す粒子 */
    sum_of_particles=0 ;
    for( ii=0 ; ii<NUM_OF_GALAXIES ; ii++ ){
	double sin_i,cos_i,sin_l,cos_l ;
	sin_i = sin(angle_i[ii]*M_PI/180) ;
	cos_i = cos(angle_i[ii]*M_PI/180) ;
	sin_l = sin(angle_l[ii]*M_PI/180) ;
	cos_l = cos(angle_l[ii]*M_PI/180) ;
	for( i=0 ; i<NUM_OF_RINGS ; i++ ){
	    int nr ;
	    double r,v ;
	    nr = ((1+i)*2.0/(NUM_OF_RINGS*(NUM_OF_RINGS+1)))*(NUM_OF_ALL_PARTICLES/2) ;
	    r=disc_w[ii] * (i+1) ;
	    v=r/pow(r*r+prms.b*prms.b,3.0/4.0)*sqrt(prms.ratio[ii]) ;
	    for( j=0 ; j<nr ; j++ ){
		double sin_t,cos_t,xdd,ydd,zdd ;
		double xd,yd,zd,x1,y1,z1 ;
		double vxdd,vydd,vzdd ;
		double vxd,vyd,vzd ;
		double vx1,vy1,vz1 ;
		k= sum_of_particles + j ;
		if( k < NUM_OF_ALL_PARTICLES ){
		    sin_t=sin(j*2.0*M_PI/nr) ;
		    cos_t=cos(j*2.0*M_PI/nr) ;
		    xdd=r*cos_t ;
		    ydd=r*sin_t ;
		    zdd=0.0 ;
		    xd=xdd ;
		    yd=ydd*cos_i+zdd*sin_i ;
		    zd=-ydd*sin_i+zdd*cos_i ;
		    x1=xd*cos_l-yd*sin_l ;
		    y1=xd*sin_l+yd*cos_l ;
		    z1=zd ;
		    vxdd=-v*sin_t ;
		    vydd=v*cos_t ;
		    vzdd=0.0 ;
		    vxd=vxdd ;
		    vyd=vydd*cos_i+vzdd*sin_i ;
		    vzd=-vydd*sin_i+vzdd*cos_i ;
		    vx1=vxd*cos_l-vyd*sin_l ;
		    vy1=vxd*sin_l+vyd*cos_l ;
		    vz1=vzd ;
		    va_p[k][0].f=va_g[ii][0].f+x1 ;
		    va_p[k][1].f=va_g[ii][1].f+y1 ;
		    va_p[k][2].f=va_g[ii][2].f+z1 ;
		    va_p[k][3].f=va_g[ii][3].f+vx1 ;
		    va_p[k][4].f=va_g[ii][4].f+vy1 ;
		    va_p[k][5].f=va_g[ii][5].f+vz1 ;
		}
	    }
	    sum_of_particles += nr ;
	}
	num_of_particles[ii] = sum_of_particles ;
	if( 0 < ii ){
	    for( j=0 ; j<ii ; j++ ) 
		num_of_particles[ii] -= num_of_particles[j] ;
	}
    }

    for( i=0 ; i<NUM_OF_ALL ; i++ ) val[i].df = 0 ;
    for( i=0 ; i<NUM_OF_ALL ; i++ ) val[i].q = 0 ;
    win=gopen(WIN_SIZE,WIN_SIZE) ;
    layer(win,0,1) ;
    i=0 ;
    /* ==== 計算開始 ==== */
    do{
	if( (i % 2) == 0 ){
	    gclr(win) ;
	    newcolor(win,"blue") ;
	    PLOT_G(win,va_g[0][0].f,va_g[0][1].f) ;
	    newcolor(win,"violet") ;
	    PLOT_G(win,va_g[1][0].f,va_g[1][1].f) ;
	    newcolor(win,"cyan") ;
	    for( j=0 ; j<sum_of_particles ; j++ ){
		if( j==num_of_particles[0] ) 
		    newcolor(win,"magenta") ;
		PLOT_P(win,va_p[j][0].f,va_p[j][1].f) ;
	    }
	    copylayer(win,1,0) ;
	}
	rngkut(&arg,(void *)(&prms),&dif_equation) ;
	i++ ;
    } while( arg.t < T_MAX ) ;
    ggetch() ;
    gclose(win) ;
    return(0) ;
}

#define GET_EQU(ratio,v0,v1,denom) ( -(ratio)*(v0-v1)/(denom) )
#define GET_DEM(r,b) (pow((r)*(r)+(b)*(b),3.0/2.0))
void dif_equation( struct rngkut_arg *a, void *ptr )
{
    int i,j,k,kk ;
    struct params *p = (struct params *)ptr ;
    double denom=0,radius=0 ;
    double rad[2],den[2] ;
    for( k=0 ; k<2 ; k++ ){
	for( i=0 ; i<p->n_dim ; i++ ){
	    p->vg[k][i].df = p->vg[k][p->n_dim+i].f ;
	    p->vg[k][p->n_dim+i].df = 0 ;
	}
	if( k ) kk=0 ;
	else{
	    kk=1 ;
	    radius = 0 ;
	    for( i=0 ; i<p->n_dim ; i++ )
		radius += pow(p->vg[k][i].f - p->vg[kk][i].f,2) ;
	    radius=sqrt(radius) ;
	    denom=GET_DEM(radius,p->b) ;
	}
	for( i=0 ; i<p->n_dim ; i++ )
	    p->vg[k][p->n_dim+i].df += 
		GET_EQU((1-p->ratio[k]),p->vg[k][i].f,p->vg[kk][i].f,denom) ;
    }
    for( i=0 ; i<p->n_particles ; i++ ){
	for( k=0 ; k<2 ; k++ ){
	    rad[k] = 0 ;
	    for( j=0 ; j<p->n_dim ; j++ )
		rad[k] += pow(p->vp[i][j].f - p->vg[k][j].f,2) ;
	    rad[k]=sqrt(rad[k]) ;
	    den[k]=GET_DEM(rad[k],p->b) ;
	}
	for( j=0 ; j<p->n_dim ; j++ ){
	    p->vp[i][j].df = p->vp[i][p->n_dim+j].f ;
	    p->vp[i][p->n_dim+j].df = 0 ;
	    for( k=0 ; k<2 ; k++ ){
		p->vp[i][p->n_dim+j].df += 
		    GET_EQU(p->ratio[k],p->vp[i][j].f,p->vg[k][j].f,den[k]) ;
	    }
	}
    }
}
#undef GET_EQU
#undef GET_DEM
/* ルンゲクッタ・ギル法 rngkut */
void rngkut( struct rngkut_arg *arg, void *ptr,
             void (*difequ)( struct rngkut_arg *, void * ) )
{
    const double a[6] = { 0, 0.5, 0.5, 1, 1, 0.5 } ;
    const double b[4] = { 1, 0.292893218813452,
                          1.707106781186547, 0.333333333333333 } ;
    const double c[4] = { 0, 0.707106781186548,
                          -0.707106781186548, 0 } ;
    double pt,kt,r ;
    int i,j ;
    pt=arg->t ;
    for( j=0 ; j<4 ; j++ ){
        arg->t=pt+a[j]*arg->h ;
        difequ( arg, ptr ) ;
        for( i=0 ; i<arg->n ; i++ ){
            kt = a[j+2]*arg->h*arg->v[i].df-arg->v[i].q ;
            r = b[j]*kt ;
            arg->v[i].f += r ;
            arg->v[i].q = r*3-kt+c[j]*arg->h*arg->v[i].df ;
        }
    }
    return ;
}
