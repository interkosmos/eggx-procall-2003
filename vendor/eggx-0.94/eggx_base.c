/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2020-09-19 12:00:00 cyamauch> */

/*
  EGGX / ProCALL  version 0.94
                   eggx_base.c
  究極の簡単さを目指して作成した，C ，FORTRAN 両用の
  X11グラフィックスライブラリ．
 */

#define _EGGX_BASE_C

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xlocale.h>
#ifdef USE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <locale.h>

#include "_eggx_internal_defs.h"
#include "eggx_base.h"
#include "_xslave_.h"
#include "exec_xslave.h"

#define RTOC_UNITLEN 32
#define MAX_NLAYER 8
#define ROOT_MARGIN 128

#define SLAVEFILE "_xslave_"

#define DEFAULTFONTSET "-*-fixed-medium-r-normal--14-*"
#define FALLBACKFONTSET "fixed,-*--14-*"

/* スクロールバーの色など */
#define WD_BACKGROUND_COLOR "#c0c0c0"
#define WD_SHADOW_COLOR     "#6f6f6f"
#define WD_TROUGH_COLOR     "#a3a3a3"
#define WD_HIGHLIGHT_COLOR  "#e7e7e7"
#define SCROLLBAR_WIDTH     19
/* ShiftMask(Shiftキー)     LockMask(CapsLock)                     */
/* ControlMask(Ctrlキー)    Mod1Mask(Altキー)   Mod2Mask(NumLock)  */
/* Mod5Mask(ScrollLockキー)                                        */
#define SCROLLBAR_DEFAULT_KEYMASK Mod1Mask		/* Altキー */

#define ch_is_space(ch) isspace((int)(ch))

#ifdef NO_ISINF
#define ISINF(x) isnan(x)
#else
#define ISINF(x) isinf(x)
#endif

/* ウィンドゥ管理用の構造体 */
struct pctg {
    int flg ;			/* 有効かどうかのフラグ */
    int attributes ;		/* EGGX での属性フラグ */
    unsigned int sbarkeymask;	/* スクロールバーのキー操作のためのkeyマスク */
    /*
     *  +-- pwin ----------------------------------------------+
     *  | +-- clipwin ---------------------------------+ +---+ |
     *  | | +-- win ---------------------------------+ | | v | |
     *  | | |                                        | | | s | |
     *  | | | この領域はclipwinからはみ出す事もあり  | | | b | |
     *  | | |                                        | | | a | |
     *  | | |                                        | | | r | |
     *  | | |                                        | | | w | |
     *  | | |                                        | | | i | |
     *  | | +----------------------------------------+ | | n | |
     *  | +--------------------------------------------+ +---+ |
     *  | +--------------------------------------------+       |
     *  | | hsbarwin                                   |       |
     *  | +--------------------------------------------+       |
     *  +------------------------------------------------------+
     */
    int wszx ;			/* グラフィックスエリアのサイズ */
    int wszy ;
    Window pwin ;		/* 親 Window ID */
    Window clipwin ;		/* 描画Window を載せるための子窓 */
    Window hsbarwin ;		/* 水平スクロールバーのための小窓 */
    Window vsbarwin ;		/* 垂直スクロールバーのための小窓 */
    Window win ;		/* 描画Window (scrollbar無し時はpwinと同じ) */
    Window iconwin ;		/* Icon Window ID */
    int sly ;			/* 表示するレイヤ */
    int wly ;			/* 書き込むレイヤ */
    Pixmap pix[MAX_NLAYER] ;	/* 各レイヤの Pixmap ID */
    GC pxgc ;			/* レイヤのコピー用GC */
    unsigned long bgcolor ;	/* 背景色 */
    GC bggc ;
    int linewidth;		/* ラインの幅 */
    int linestyle;		/* ラインスタイル */
    int gcfunc;			/* GC function */
    int fsz ;			/* フォントサイズ */
    XFontStruct *fontstruct ;	/* フォント */
    XFontSet fontset ;		/* フォントセット */
    unsigned short fontheight ;
    GC gc ;			/* 描画に使うGC */
    Pixmap tmppix;		/* 一時 Pixmap ID */
    double acx0 ;		/* window座標(0,0) での application座標 */
    double acy0 ;
    double scalex ;		/* スケーリングファクター (xyconvで使用) */
    double scaley ;
    int prevx ;			/* moveto(), lineto() で記憶している座標 */
    int prevy ;
    char *winname ;		/* Windowのtitle winname()関数で指定 */
} ;

/* フォントセットのキャッシュ用 */
struct fontsettg {
    char *fontname;		/* フォント名 */
    XFontSet fontset;		/* フォントセット構造体 */
    char **missing_list;
} ;

/* 
 * プライベートな定数
 */

/* ASCII フォントの設定 */
static const char *Pc_fnt[24] = { "5x7","5x7","5x7","5x7","5x7","5x7","5x7",
				  "5x8","6x9","6x10","6x10","6x12","6x13",
				  "7x14","7x14","8x16","8x16","8x16","8x16",
				  "10x20","10x20","10x20","10x20","12x24" };
static const char *Pc_fallbackfnt_fmt = "-*-fixed-medium-r-normal--%s-*";
static const char *Pc_fallbackfnt = "fixed";

/* マウスの全ボタンマスク */
static const unsigned int Button_12345_Mask = Button1Mask | Button2Mask |
					      Button3Mask | Button4Mask | 
					      Button5Mask;

/*
 * プライベートな変数
 */

/* Xサーバの情報いろいろ */
static Display* Pc_dis = NULL ;			/* Display ID */
static Colormap Pc_cmap = None ;		/* カラーマップ */
static Window Pc_zwin = None ;			/* 子プロセスとの通信用の窓 */
static int Pc_depth = 8 ;			/* depth */
static int Pc_root_width = 0;			/* root window のサイズ */
static int Pc_root_height = 0;
static int Pc_display_width = 0;		/* 最小のディスプレイサイズ */
static int Pc_display_height = 0;		/* (Xinerama 有効時のみ)    */
static XVisualInfo *Pc_vinfo_ptr = NULL ;
static Visual *Pc_visual = NULL ;
static int Red_depth = 0, Green_depth = 0, Blue_depth = 0;
static int Red_sft = 0, Green_sft = 0, Blue_sft = 0 ;

/* 以下，ウィンドゥオープン時の設定を保持 */
static int Pc_bdr_width = 0 ;					/* ボーダー */
static int Pc_attributes = SCROLLBAR_INTERFACE | BOTTOM_LEFT_ORIGIN;	/* 属性 */

static char *Pc_storename = NULL ;
static char *Pc_iconname = NULL ;
static char *Pc_classhint_res_name = NULL ;
static char *Pc_classhint_res_class = NULL ;
static char *Pc_geometry = NULL ;
static char *Pc_bgcolor = NULL ;
static char *Pc_bordercolor = NULL ;

static char _Pname[] = "EGGX" ;
static char *Pname = _Pname ;			/* ウィンドゥの名前 */

static int Pc_nonblock = DISABLE ;		/* ノンブロッキングのフラグ */
static int Pc_nonflush = DISABLE ;		/* 非自動フラッシュのフラグ */

/* EGGX でのウィンドゥ管理のためのobject */
static struct pctg *Pc = NULL ;
static int N_Pc = 0 ;				/* Pc のバッファの個数 */
static integer Wn = -1 ;			/* ProCALL互換routine用winID */

/* 以下，キャッシュ等 */
static Pixmap Pix_mask = None ;			/* putimg24m() 用 */
static int Pix_mask_width = 0 ;
static int Pix_mask_height = 0 ;
static GC Gc_pix_mask = None ;

static unsigned char *Tmp_img24buf = NULL ;	/* putimg24*() 用のバッファ */
static size_t Sz_tmp_img24buf = 0 ;		/* ↑のバイトサイズ */

static XColor *Pc_cmapcolors = NULL ;		/* cmapテーブル: 8bpp 専用 */
static int Pc_ncmapcolors = 0 ;			/* ↑の個数 */
static struct fontsettg *Pc_fscache = NULL ;	/* フォントセットキャッシュ */
static int Pc_nfscache = 0 ;			/* ↑の個数 */

/* マウスの状態の保存用(ggetevent()で使用) */
static int Pc_evrec_pointer_x = -32000;		/* RootWindow での座標 */
static int Pc_evrec_pointer_y = -32000;		/* RootWindow での座標 */
static int Pc_evrec_wid = -1;			/* EGGX のウィンドゥID */
static unsigned int Pc_evrec_button_mask = 0;

/* 以下，プロセスまわり */
static pid_t Pid = -1 ;				/* 自身のプロセスID */
/* Window管理用の子プロセス */
static volatile pid_t Cpid = 0 ;
/* 一時利用のための子プロセス */
static volatile pid_t Cpid_tmp = 0 ;
/* saveimg() 用の子プロセス */
static volatile pid_t Cpid_saveimg = 0 ;
/* saveimg() 用の子プロセスの状態 (0:起動していない，1:起動中) */
static volatile int Chld_imgsave_stat = 0 ;
/* 子プロセスのファイル名 */
static char Exec_slave_tmp[] = "/tmp/" SLAVEFILE "XXXXXX" ;
static int Exec_fd = -1 ;

/* この2つはコードには入っているが，実質，使っていない(そろそろ消そうか…) */
static volatile int Ihflg = 0 ;			/* 割り込み禁止フラグ */
static volatile int Exitflg = 0 ;		/* 割り込みによって終了が知らされたが，すぐにはできない場合にたてる */


/* ≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡ */
/*             汎用的な内部関数             */
/* ≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡ */

static int _eggx_iround( double v )
{
    if ( v < 0 ) return (int)(v-0.5);
    else return (int)(v+0.5);
}

/* エラーチェック付きmalloc,realloc,strdup等 */
static void *_eggx_xmalloc( size_t size )
{
    void *rt ;
    rt=malloc(size) ;
    if( rt==NULL ){
        fprintf(stderr,"EGGX: [ERROR] malloc() failed.\n") ;
        exit(1) ;
    }
    return( rt ) ;
}

static void *_eggx_xrealloc( void *ptr, size_t size )
{
    void *rt ;
    rt=realloc(ptr,size) ;
    if( rt==NULL ){
        fprintf(stderr,"EGGX: [ERROR] realloc() failed.\n") ;
        exit(1) ;
    }
    return( rt ) ;
}

static char *_eggx_xstrdup( const char *s )
{
    char *rt = strdup(s) ;
    if ( rt == NULL ) {
        fprintf(stderr,"EGGX: [ERROR] strdup() failed.\n") ;
        exit(1) ;
    }
    return (rt) ;
}

/* va_copy を使った時点で，vsnprintf が無いOSではコンパイル不可だと思うが… */
#ifdef NO_VSNPRINTF
static int _eggx_dev_null_w_open( void )
{
    int f ;
    f=open("/dev/null",O_WRONLY) ;
    if( f == -1 ){
	fprintf(stderr,"EGGX: [ERROR] Cannot open /dev/null.\n") ;
	exit(1) ;
    }
    return( f ) ;
}

static char *_eggx_vasprintf( const char *format, va_list ap )
{
    char *rt = NULL;
    int f, nn;
    FILE *fp;
    va_list aq;

    if ( format == NULL ) goto quit;

    f = _eggx_dev_null_w_open();
    fp = fdopen(f,"w");
    if ( fp == NULL ) {
	fprintf(stderr,"EGGX: [ERROR] Cannot open /dev/null.\n");
	exit(1);
    }
    va_copy(aq, ap);
    nn = vfprintf(fp,format,ap);
    va_end(ap);
    fclose(fp);

    rt = (char *)_eggx_xmalloc(sizeof(char)*(nn+1));
    vsprintf(rt,format,aq);
    va_end(aq);

 quit:
    return rt;
}
#else
static char *_eggx_vasprintf( const char *format, va_list ap )
{
    const int ini_len = 64;
    char *rt = NULL;
    int nn;
    va_list aq;

    if ( format == NULL ) goto quit;

    va_copy(aq, ap);
    rt = (char *)_eggx_xmalloc(sizeof(char)*ini_len);
    nn = vsnprintf(rt,ini_len,format,ap);
    va_end(ap);

    if ( ini_len <= nn ) {
	rt = (char *)_eggx_xrealloc(rt,sizeof(char)*(nn+1));
	vsnprintf(rt,nn+1,format,aq);
    }
    va_end(aq);

 quit:
    return rt;
}
#endif

static char *_eggx_asprintf( const char *format, ... )
{
    char *rt = NULL;
    va_list ap;

    if ( format == NULL ) goto quit;

    va_start(ap, format);
    rt = _eggx_vasprintf(format,ap);
    va_end(ap);

 quit:
    return rt;
}

/* 割込みを設定する */
static int _eggx_signal( int signum, int sa__flags,
			 void (*sa__handler)(int) )
{
    struct sigaction sa ;
    sigset_t sm ;
    /* シグナルマスクの設定 */
    sigemptyset( &sm ) ;
    sigaddset( &sm, signum ) ;
    /* 割り込み登録 */
    sa.sa_handler = sa__handler ;
    sa.sa_mask = sm ;
    sa.sa_flags = sa__flags ;
    return( sigaction( signum, &sa, NULL ) ) ;
}

#if 0
/* 割込みを設定する */
static int _eggx_sigaction( int signum, int sa__flags,
			    void (*sa__sigaction)(int, siginfo_t *, void *) )
{
    struct sigaction sa ;
    sigset_t sm ;
    /* シグナルマスクの設定 */
    sigemptyset( &sm ) ;
    sigaddset( &sm, signum ) ;
    /* 割り込み登録 */
    sa.sa_sigaction = sa__sigaction ;
    sa.sa_mask = sm ;
    sa.sa_flags = sa__flags | SA_SIGINFO ;
    return( sigaction( signum, &sa, NULL ) ) ;
}
#endif

static char *_eggx_xstrdup_toupper( const char *s )
{
    int i ;
    char *rt ;
    rt = _eggx_xstrdup( s ) ;
    if ( rt!=NULL ){
	for( i=0 ; i<strlen(s) ; i++ ){
	    int ch = ((const unsigned char *)s)[i];
	    ((unsigned char *)rt)[i] = toupper(ch);
	}
    }
    return(rt) ;
}

/* ≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡ */
/*           EGGX 内部用の関数              */
/* ≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡ */

/* ============= テンポラリファイル関連 ============= */

#ifdef __CYGWIN__
static volatile char *volatile *volatile Remove_list = NULL;
static volatile int Remove_cnt = 0;
static int add_to_removelist( const char *filename )
{
    int i;
    volatile char *volatile *volatile old_ptr;
    volatile char *volatile *volatile new_ptr;
    old_ptr = Remove_list;
    new_ptr = (volatile char *volatile *volatile)_eggx_xmalloc(sizeof(char *)*(Remove_cnt+1));
    for ( i=0 ; i < Remove_cnt ; i++ ) new_ptr[i] = old_ptr[i];
    Remove_list = new_ptr;
    Remove_list[Remove_cnt] = _eggx_xstrdup(filename);
    Remove_cnt++;
    if ( old_ptr != NULL ) free((void *)old_ptr);
    return (0);
}

static void remove_removelist( void )
{
    int i;
    for ( i=0 ; i < Remove_cnt ; i++ ) {
	if ( Remove_list[i] != NULL ) {
	    struct stat st;
	    char *fn = (char *)(Remove_list[i]);
	    while ( remove( fn ) ) {
		if ( stat( fn, &st ) ) break;
		eggx_msleep(10);
	    }
	    free((void *)(Remove_list[i]));
	    Remove_list[i] = NULL;
	}
    }
    if ( Remove_list != NULL ) {
	Remove_cnt = 0;
	free((void *)Remove_list);
	Remove_list = NULL;
    }
    return;
}
#endif

static int mkexecfile( void )
{
    struct stat st ;
    mode_t mode ;

#ifdef __CYGWIN__
    if( 0 < Remove_cnt ){
	if( stat( Exec_slave_tmp, &st )==0 ){
	    return(0) ;
	}
    }
#endif

    /* if( Exec_fd != -1 ) return( 0 ) ; */
    while(1){
	strcpy(Exec_slave_tmp,"/tmp/" SLAVEFILE "XXXXXX") ;
#ifdef NO_MKSTEMP	/* mkstempがない場合 */
	if( mktemp( Exec_slave_tmp ) == NULL ) return( -1 ) ;
	Exec_fd = open( Exec_slave_tmp, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR) ;
#else
	Exec_fd = mkstemp( Exec_slave_tmp ) ;
#endif
	if( Exec_fd == -1 ) return( -1 ) ;

	/* 念のため，statで確認 */
	if( stat( Exec_slave_tmp, &st ) ){
	    close( Exec_fd ) ;
	    Exec_fd=-1 ;
	    remove(Exec_slave_tmp) ;
	    eggx_msleep( 10 ) ;
	    continue ;
	}
	if( write( Exec_fd, _xslave_, sizeof(_xslave_) ) != sizeof(_xslave_) ){
	    close( Exec_fd ) ;
	    Exec_fd=-1 ;
	    remove(Exec_slave_tmp) ;
	    eggx_msleep( 10 ) ;
	    continue ;
	}
	if( stat( Exec_slave_tmp, &st ) ){
	    close( Exec_fd ) ;
	    Exec_fd=-1 ;
	    remove(Exec_slave_tmp) ;
	    eggx_msleep( 10 ) ;
	    continue ;
	}
	break ;
    }
    mode=st.st_mode ;
    chmod( Exec_slave_tmp, mode | S_IXUSR ) ;
    fsync(Exec_fd) ;
    /* close(Exec_fd) ; */	/* closeしない．execvする直前にclose */
#ifdef __CYGWIN__
    add_to_removelist(Exec_slave_tmp) ;
#endif
    return( 0 ) ;
}

static void rmexecfile( void )
{
    if( Exec_fd != -1 ){
	close(Exec_fd) ;
	Exec_fd = -1 ;
    }
#ifndef __CYGWIN__
    remove( Exec_slave_tmp ) ;
#endif
}


/* ============= 割込み関連 ============= */

/* 子プロセスが終了した時の処理 */
static void chldexithandler( int dummy )
{
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    if ( pid == Cpid_saveimg ) {
	if ( Chld_imgsave_stat != 0 ) Chld_imgsave_stat = 0;
	Cpid_saveimg = 0;
    }
    else if ( pid == Cpid_tmp ) {
	Cpid_tmp = 0;
    }
    else if ( pid == Cpid ) {
	Cpid = 0;
    }
    return;
}

static void killallchld()
{
    if ( 0 < Cpid_saveimg ) {
	kill(Cpid_saveimg, SIGTERM);
	while ( 0 < Cpid_saveimg ) {
	    eggx_msleep( 10 );
	}
    }
    if ( 0 < Cpid_tmp ) {
	kill(Cpid_tmp, SIGTERM);
	while ( 0 < Cpid_tmp ) {
	    eggx_msleep( 10 );
	}
    }
    if ( 0 < Cpid ) {
	kill(Cpid, SIGTERM);
	while ( 0 < Cpid ) {
	    eggx_msleep( 10 );
	}
    }
    return;
}

static void wait_child( void )
{
    while ( Chld_imgsave_stat ) {
	eggx_msleep( 10 );
	/* fprintf(stderr,"DEBUG: waiting cpid=%d\n",Cpid_saveimg); */
    }
}

static void iint_handler( int dummy )
{
    /*
    if( Ihflg ) Exitflg = 1 ;
    else exit(0) ;
    */
    /* wait_child() ; */	/* imgsave中は待たせる */
    /* set_defaulthandlers() ; */
    exit(130);
}

static void iterm_handler( int dummy )
{
    /*
    if( Ihflg ) Exitflg = 1 ;
    else exit(0) ;
    */
    /* wait_child() ; */	/* imgsave中は待たせる */
    /* set_defaulthandlers() ; */
    exit(143);
}

static int Pc_eggxhandlers_are_set = 0;

/* 割り込みセット */
static void set_eggxhandlers( void )
{
    if ( Pc_eggxhandlers_are_set == 0 ) {
	_eggx_signal( SIGCHLD, SA_NOCLDSTOP|SA_RESTART, &chldexithandler );
	_eggx_signal( SIGINT, 0, &iint_handler );
	_eggx_signal( SIGTERM, 0, &iterm_handler );
	Pc_eggxhandlers_are_set = 1;
    }
    return;
}

/* 割り込みOFF */
static void set_defaulthandlers( void )
{
    if ( Pc_eggxhandlers_are_set != 0 ) {
	_eggx_signal( SIGTERM, 0, SIG_DFL );
	_eggx_signal( SIGINT, 0, SIG_DFL );
	_eggx_signal( SIGCHLD, 0, SIG_DFL );
	Pc_eggxhandlers_are_set = 0;
    }
    return;
}

static void chkexit( void )
{
    if( Exitflg ) iint_handler( 0 ) ;
}

/* プロセスのコマンド名を調べて返す．返り値はヒープ．呼び出し側でfree()する */
static char *get_procname( pid_t pd )
{
    char arg_ps0[] = "/bin/ps";
    char arg_ps1[] = "/usr/bin/ps";
    char arg_opt_p[] = "-p";
    char arg_opt_f[] = "-f";
    char *ret;
    char *ptr,*eptr ;
    int pfds[2] ;
    pid_t pid ;
    char *args[]= { arg_ps0, arg_opt_p, NULL, arg_opt_f, NULL } ;
    char *args0[]= { arg_ps1, arg_opt_p, NULL, arg_opt_f, NULL } ;
    char *buf=NULL ;
    char *name=NULL ;
    int i ;
    FILE *fp ;
    int mlc ;
    int c,cnt=0,line_cnt=0 ;
    int cmd_idx=-1;	/* default */

#ifdef ANOTHERPS
    goto err ;
#endif

    name = _eggx_asprintf("%ld",(long)pd);

    args[2]=name ;
    args0[2]=name ;

    while( 1 ){
	if( pipe(pfds)<0 ) goto err ;
	if( (Cpid_tmp=(pid=fork())) < 0 ) goto err ;
	if( pid == 0 ){
	    dup2( pfds[1],1 ) ;	/* 読み込み用 */
	    close( pfds[1] ) ;
	    close( pfds[0] ) ;
	    execv( *args, args ) ;
	    execv( *args0, args0 ) ;
	    fprintf(stderr,"EGGX: [ERROR] Cannot exec 'ps' command.\n") ;
	    if( Pid == getppid() ){
		kill(Pid,SIGTERM) ;
	    }
	    _exit(-1) ;
	}
	close( pfds[1] ) ;
	fp = fdopen( pfds[0],"r" ) ;
	if( fp!=NULL ){
	    cnt=0 ;
	    line_cnt=0 ;
	    mlc = 64 ;
	    buf=(char *)_eggx_xmalloc(sizeof(char)*mlc) ;
	    while( (c=fgetc(fp))!=EOF ){
		if( cnt == mlc-1 ){
		    mlc += 64 ;
		    buf=(char *)_eggx_xrealloc(buf,sizeof(char)*mlc) ;
		}
		if( c == '\n' ) line_cnt++ ;
		if( c < 0x020 ) c=0x020 ;
		buf[cnt]=c ;
		cnt++ ;
	    }
	    buf[cnt]='\0' ;
	    fclose(fp) ;
	}
	else close( pfds[0] ) ;
	/* */
	if ( Pc_eggxhandlers_are_set ) {
	    while ( 0 < Cpid_tmp ) {
		eggx_msleep(10);
	    }
	}
	else {
	    int status;
	    while ( wait(&status) != pid );
	    Cpid_tmp = 0;
	}
	/* */
	if ( line_cnt > 1 ) break ;	/* /proc以下に書き込まれるまでに */
	eggx_msleep( 10 ) ;		/* 時間がかかることがある */
    } ;
    if( cnt == 0 ) goto err ;
    for( i=0 ; i<cnt ; i++ ){
	if( 0x020 < buf[cnt-1-i] ) break ;
	buf[cnt-1-i]='\0' ;
    }
    if( strlen(buf) < 2 ) goto err ;
    ptr=buf ;
    if( *ptr != 0x020 ) i=1 ;
    else i=0 ;
    while( *++ptr != '\0' ){
	if( *(ptr-1) == 0x020 && *ptr != 0x20 ){
	    i++ ;
	    if ( strncmp(ptr,"CMD ",4) == 0 ) cmd_idx=i;
	    else if ( strncmp(ptr,"COMMAND ",8) == 0 ) cmd_idx=i;
	    if( 0 <= cmd_idx && i == cmd_idx * 2 ) break ;
	}
    }
    if( *ptr == '\0' ) goto err ;
    eptr=ptr ;
    while( *++eptr != '\0' ){
	if( *eptr == 0x020 ) break ;
    }
    *eptr='\0' ;
    ptr=eptr ;
    while( *--ptr != 0x020 ){
	if( *ptr == '/' ) break ;
    }
    ptr++ ;

    if( name != NULL ) free(name) ;
    ret = _eggx_xstrdup(ptr);
    if( buf  != NULL ) free(buf) ;
    return( ret ) ;
 err:
    if( buf  != NULL ) free(buf) ;
    if( name != NULL ) free(name) ;
    return(NULL) ;
}

/* ============= 描画関連 ============= */

static void do_auto_flush( void )
{
    if ( Pc_nonflush == DISABLE && Pc_dis != NULL ) {
	XFlush( Pc_dis );
    }
    return;
}

/* cmapテーブルから pixel をゲット．新規の場合は登録する．8bpp の時に使う */
static void get_colorpixel_from_cmap( Colormap cmap, XColor *color_ptr )
{
    if ( Pc_cmapcolors != NULL ) {
	int i;
	for ( i=0 ; i < Pc_ncmapcolors ; i++ ) {
	    if ( Pc_cmapcolors[i].red == color_ptr->red &&
		 Pc_cmapcolors[i].green == color_ptr->green &&
		 Pc_cmapcolors[i].blue == color_ptr->blue ) {
		*color_ptr = Pc_cmapcolors[i];
		goto quit;
	    }
	}
    }
    if ( XAllocColor(Pc_dis, cmap, color_ptr) == 0 ) {
	color_ptr->pixel = 0;
	fprintf(stderr,"EGGX: [WARNING] XAllocColor() failed.\n");
	goto quit;
    }
    Pc_cmapcolors = (XColor *)_eggx_xrealloc(Pc_cmapcolors,
				  sizeof(*Pc_cmapcolors)*(Pc_ncmapcolors + 1));
    Pc_cmapcolors[Pc_ncmapcolors] = *color_ptr;
    Pc_ncmapcolors ++;

 quit:
    return;
}

/* cmapテーブルを開放する */
static void free_cmap_table()
{
    if ( Pc_cmapcolors != NULL ) {
	free(Pc_cmapcolors);
	Pc_cmapcolors = NULL;
	Pc_ncmapcolors = 0;
    }
    return;
}

/* 色の名前から pixel への変換 */
static unsigned long get_color_pixel( char *color_name )
{
    Colormap cmap;
    XColor exact_color;
    Status xs;

    cmap = Pc_cmap /* DefaultColormap( Pc_dis, 0 ) */ ;

    exact_color.pixel = 0;
    xs = XParseColor(Pc_dis, cmap, color_name, &exact_color);
    if ( xs == 0 ) {
	fprintf(stderr,"EGGX: [WARNING] XParseColor() failed.\n");
	exact_color.pixel = 0;
	goto quit;
    }
    if ( Pc_depth == 8 ) {	/* 256色時 */
	get_colorpixel_from_cmap(cmap, &exact_color);
    }
    else {
	int r = exact_color.red;
	int g = exact_color.green;
	int b = exact_color.blue;
	exact_color.pixel  = (r>>(16-Red_depth))<<Red_sft;
	exact_color.pixel |= (g>>(16-Green_depth))<<Green_sft;
	exact_color.pixel |= (b>>(16-Blue_depth))<<Blue_sft;
    }

 quit:
    return ( exact_color.pixel );
}

/* フォントセットを得る．新規の場合は，キャッシュに登録する */
static XFontSet get_fontset( const char *fontname )
{
    XFontSet rt;
    int mc, i;
    char *ds;
    char **ml = NULL;
    if ( Pc_fscache != NULL ) {
	/* キャッシュ内を探す */
	for ( i=0 ; i < Pc_nfscache ; i++ ) {
	    if ( strcmp(Pc_fscache[i].fontname,fontname) == 0 ) {
		rt = Pc_fscache[i].fontset;
		goto quit;
	    }
	}
    }
    /* 新規にフォントセットを作り，キャッシュに登録する */
    rt = XCreateFontSet(Pc_dis,fontname,&ml,&mc,&ds) ;
    if ( rt != NULL ) {
	Pc_fscache = (struct fontsettg *)_eggx_xrealloc(Pc_fscache,
					sizeof(*Pc_fscache)*(Pc_nfscache + 1));
	Pc_fscache[Pc_nfscache].fontname = _eggx_xstrdup(fontname);
	Pc_fscache[Pc_nfscache].fontset = rt;
	Pc_fscache[Pc_nfscache].missing_list = ml;
	Pc_nfscache ++;
    }
    else {
	if ( ml != NULL ) XFreeStringList(ml);
    }

 quit:
    return rt;
}

/* フォントセットのキャッシュを開放する */
/* ただし，Xlib のバグで，完全には開放されない…orz */
static void free_fontset_cache()
{
    if ( Pc_fscache != NULL ) {
	int i;
	for ( i=0 ; i < Pc_nfscache ; i++ ) {
	    if ( Pc_fscache[i].fontname != NULL )
		free(Pc_fscache[i].fontname);
	    if ( Pc_fscache[i].fontset != NULL )
		XFreeFontSet(Pc_dis, Pc_fscache[i].fontset);
	    if ( Pc_fscache[i].missing_list != NULL )
		XFreeStringList(Pc_fscache[i].missing_list);
	}
	free(Pc_fscache);
	Pc_fscache = NULL;
	Pc_nfscache = 0;
    }
    return;
}

/* 座標変換を行なう関数 */
static void xyconv( int wn, double x, double y, int *rx, int *ry )
{
    double f_rx, f_ry;
    f_rx = ((x-Pc[wn].acx0)*Pc[wn].scalex);
    f_ry = ((y-Pc[wn].acy0)*Pc[wn].scaley);
    if ( f_rx < -32767.0 ) f_rx = -32767.0;
    else if ( 32767.0 < f_rx ) f_rx = 32767.0;
    if ( f_ry < -32767.0 ) f_ry = -32767.0;
    else if ( 32767.0 < f_ry ) f_ry = 32767.0;

    *rx = _eggx_iround(f_rx);
    if ( (Pc[wn].attributes & BOTTOM_LEFT_ORIGIN) ) {
	int ry0 = Pc[wn].wszy - 1 - _eggx_iround(f_ry);
	if ( 32767 < ry0 ) *ry = 32767;
	else *ry = ry0;
    }
    else {
	*ry = _eggx_iround(f_ry);
    }
    return;
}

/* ========================== */

static int needs_pset( int wn )
{
    int pst = ( Pc[wn].linewidth == 1 && Pc[wn].linestyle == LineSolid &&
		/* この条件では，2回以上同じところを描画しても同じ結果になる */
		/* GC function だけを選んでいる */
		(Pc[wn].gcfunc == GXcopy || Pc[wn].gcfunc == GXclear ||
		 Pc[wn].gcfunc == GXand || Pc[wn].gcfunc == GXandReverse ||
		 Pc[wn].gcfunc == GXnoop || Pc[wn].gcfunc == GXor ||
		 Pc[wn].gcfunc == GXinvert || Pc[wn].gcfunc == GXorReverse ||
		 Pc[wn].gcfunc == GXset) );
    return pst;
}

static Window XX_CreateSimpleWindow(Display *display, Window parent,
				   int x, int y,
				   unsigned int width, unsigned int height,
				   unsigned int border_width,
				   unsigned long border, 
				   unsigned long background )
{
    XSetWindowAttributes attributes ;

    attributes.border_pixel = border ;
    attributes.background_pixel = background ;
    attributes.colormap = Pc_cmap ;

    return( XCreateWindow(display,parent,x,y,width,height,
			  border_width,
			  Pc_depth,InputOutput,Pc_visual,
			  CWBorderPixel | CWBackPixel | CWColormap,
			  &attributes) ) ;
}

/* 開始点と終了点に点を打ち，ラインを引く(特定のXサーバ対策) */
static int XX_DrawLine( Display *dis, int wn, int xs, int ys, int xe, int ye )
{
    int pst = needs_pset(wn) ;
    int i ;
    i=Pc[wn].wly ;
    if ( pst ) XDrawPoint( dis, Pc[wn].pix[i], Pc[wn].gc, xs, ys ) ;
    XDrawLine( dis, Pc[wn].pix[i], Pc[wn].gc, xs,ys,xe,ye ) ;
    if ( pst ) XDrawPoint( dis, Pc[wn].pix[i], Pc[wn].gc, xe, ye ) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    if ( pst ) XDrawPoint( dis, Pc[wn].win, Pc[wn].gc, xs, ys ) ;
    XDrawLine( dis, Pc[wn].win, Pc[wn].gc, xs,ys,xe,ye ) ;
    if ( pst ) XDrawPoint( dis, Pc[wn].win, Pc[wn].gc, xe, ye ) ;
    if( Pc[wn].iconwin != None ){
	if ( pst ) XDrawPoint( dis, Pc[wn].iconwin, Pc[wn].gc, xs, ys ) ;
	XDrawLine( dis, Pc[wn].iconwin, Pc[wn].gc, xs,ys,xe,ye ) ;
	if ( pst ) XDrawPoint( dis, Pc[wn].iconwin, Pc[wn].gc, xe, ye ) ;
    }
    return(1) ;
}

static int X_DrawLine(  Display *dis, int wn, int xs, int ys, int xe, int ye )
{
    XDrawLine( dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc, xs,ys,xe,ye ) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XDrawLine( dis, Pc[wn].win, Pc[wn].gc, xs,ys,xe,ye ) ;
    if( Pc[wn].iconwin != None ){
	XDrawLine( dis, Pc[wn].iconwin, Pc[wn].gc, xs,ys,xe,ye ) ;
    }
    return(1) ;
}

static int X_DrawLines(  Display *dis, int wn, XPoint *pts, int npts )
{
    XDrawLines( dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc,
		pts, npts, CoordModeOrigin ) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XDrawLines( dis, Pc[wn].win, Pc[wn].gc, pts, npts, CoordModeOrigin ) ;
    if( Pc[wn].iconwin != None ){
	XDrawLines( dis, Pc[wn].iconwin, Pc[wn].gc,
		    pts, npts, CoordModeOrigin ) ;
    }
    return(1) ;
}

static int X_DrawPoint( Display *dis, int wn, int x, int y )
{
    XDrawPoint( dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc, x, y ) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XDrawPoint( dis, Pc[wn].win, Pc[wn].gc, x, y ) ;
    if( Pc[wn].iconwin != None ){
	XDrawPoint( dis, Pc[wn].iconwin, Pc[wn].gc, x, y ) ;
    }
    return(1) ;
}

static int X_DrawPoints( Display *dis, int wn, XPoint *pts, int npts )
{
    XDrawPoints( dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc,
		 pts, npts, CoordModeOrigin ) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XDrawPoints( dis, Pc[wn].win, Pc[wn].gc, pts, npts, CoordModeOrigin ) ;
    if( Pc[wn].iconwin != None ){
	XDrawPoints( dis, Pc[wn].iconwin, Pc[wn].gc, 
		     pts, npts, CoordModeOrigin ) ;
    }
    return(1) ;
}

static int X_DrawString( Display *dis, int wn, int x, int y, char *str, int len )
{
    XDrawString( dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc, x, y, str, len ) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XDrawString( dis, Pc[wn].win, Pc[wn].gc, x, y, str, len ) ;
    if( Pc[wn].iconwin != None ){
	XDrawString( dis, Pc[wn].iconwin, Pc[wn].gc, x, y, str, len ) ;
    }
    return(1) ;
}

static int X_mbDrawString( Display *dis, int wn, int x, int y, char *str, int len )
{
    XmbDrawString( dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].fontset, Pc[wn].gc, x, y, str, len ) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XmbDrawString( dis, Pc[wn].win, Pc[wn].fontset, Pc[wn].gc, x, y, str, len ) ;
    if( Pc[wn].iconwin != None ){
	XmbDrawString( dis, Pc[wn].iconwin, Pc[wn].fontset, Pc[wn].gc, x, y, str, len ) ;
    }
    return(1) ;
}

static int X_FillArc( Display *dis, int wn, int x, int y, 
		      int sx, int sy, int s, int w )
{
    XFillArc( dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc, x, y, sx, sy, s, w ) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XFillArc( dis, Pc[wn].win, Pc[wn].gc, x, y, sx, sy, s, w ) ;
    if( Pc[wn].iconwin != None ){
	XFillArc( dis, Pc[wn].iconwin, Pc[wn].gc, x, y, sx, sy, s, w ) ;
    }
    return(1) ;
}

static int X_DrawArc( Display *dis, int wn, int x, int y, 
		      int sx, int sy, int s, int w )
{
    XDrawArc( dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc, x, y, sx, sy, s, w ) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XDrawArc( dis, Pc[wn].win, Pc[wn].gc, x, y, sx, sy, s, w ) ;
    if( Pc[wn].iconwin != None ){
	XDrawArc( dis, Pc[wn].iconwin, Pc[wn].gc, x, y, sx, sy, s, w ) ;
    }
    return(1) ;
}

static int X_FillPolygon( Display *dis, int wn, XPoint* points, 
			  int num, int shape, int mode )
{
    XFillPolygon( dis,Pc[wn].pix[Pc[wn].wly],Pc[wn].gc,points,num,shape,mode) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XFillPolygon( dis,Pc[wn].win,Pc[wn].gc,points,num,shape,mode) ;
    if( Pc[wn].iconwin != None ){
	XFillPolygon( dis,Pc[wn].iconwin,Pc[wn].gc,points,num,shape,mode) ;
    }
    return(1) ;
}

static int X_DrawRectangle( Display *dis, int wn, int x, int y, int w, int h )
{
    XDrawRectangle( dis,Pc[wn].pix[Pc[wn].wly],Pc[wn].gc,x,y,w,h) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XDrawRectangle( dis,Pc[wn].win,Pc[wn].gc,x,y,w,h) ;
    if( Pc[wn].iconwin != None ){
	XDrawRectangle( dis,Pc[wn].iconwin,Pc[wn].gc,x,y,w,h) ;
    }
    return(1) ;
}

static int X_FillRectangle( Display *dis, int wn, int x, int y, int w, int h )
{
    XFillRectangle( dis,Pc[wn].pix[Pc[wn].wly],Pc[wn].gc,x,y,w,h) ;
    if( Pc[wn].wly!=Pc[wn].sly ) return(0) ;
    XFillRectangle( dis,Pc[wn].win,Pc[wn].gc,x,y,w,h) ;
    if( Pc[wn].iconwin != None ){
	XFillRectangle( dis,Pc[wn].iconwin,Pc[wn].gc,x,y,w,h) ;
    }
    return(1) ;
}

/* ========================== */

static int bitcount( int d )
{
    int rt=0 ;
    while( (d & 0x01)==0x01 ){
	d>>=1 ;
	rt++ ;
    }
    return(rt) ;
}
static int sftcount( int *mask )
{
    int rt=0 ;
    while( (*mask & 0x01)==0 ){
	(*mask)>>=1 ;
	    rt++ ;
    }
    return(rt) ;
}

/* ========================== */

static void send_command_to_child( int wn, int mcode, Drawable value,
				   Bool flush )
{
    XGraphicsExposeEvent geev;
    /* */
    memset(&geev, 0, sizeof(geev));
    geev.type = GraphicsExpose;
    geev.send_event = True;
    geev.display = Pc_dis;
    geev.drawable = value;
    geev.x = 0;
    geev.y = 0;
    geev.major_code = wn;
    geev.minor_code = mcode;
    /* fprintf(stderr,"DEBUG: sending pixid: [%ld]\n",
       Pc[wn].pix[Pc[wn].wly]); */
    XSendEvent(Pc_dis, Pc_zwin, False, ExposureMask, (XEvent *)&geev);
    if ( flush ) XFlush(Pc_dis);
    return;
}

static void restore_xinput_selection( int wn )
{
    if ( Pc_nonblock != DISABLE ) {
	if ( Pc[wn].pwin != Pc[wn].win ) {
	    XSelectInput( Pc_dis, Pc[wn].pwin, KeyPressMask );
	    XSelectInput( Pc_dis, Pc[wn].win, 
		      ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
		      LeaveWindowMask | EnterWindowMask );
	}
	else {
	    XSelectInput( Pc_dis, Pc[wn].win, 
		       KeyPressMask | ButtonPressMask | ButtonReleaseMask |
		       PointerMotionMask | LeaveWindowMask | EnterWindowMask );
	}
    } else {
	if ( Pc[wn].pwin != Pc[wn].win ) {
	    XSelectInput( Pc_dis, Pc[wn].pwin, 0 );
	    XSelectInput( Pc_dis, Pc[wn].win, 0 );
	}
	else {
	    XSelectInput( Pc_dis, Pc[wn].win, 0 );
	}
    }
    return;
}


/* ≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡ */
/*            外部に提供する関数            */
/* ≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡≡ */

/* ============= flush まわりを扱う関数 ============= */

/* フラッシュモードの設定を行なう */
void eggx_gsetnonflush( int flag )
{
    if ( flag != DISABLE ) {
	Pc_nonflush = ENABLE;
    }
    else {
	Pc_nonflush = DISABLE;
    }
    return;
}

void gsetnonflush_( integer *flag )
{
    int i_flag = *flag;
    eggx_gsetnonflush(i_flag);
    return;
}

int eggx_ggetnonflush( void )
{
    return Pc_nonflush;
}

void ggetnonflush_( integer *flag )
{
    *flag = Pc_nonflush;
    return;
}

/* 手動flushを行なう */
void eggx_gflush( void )
{
    if ( Pc_dis != NULL ) XFlush( Pc_dis );
    return;
}

void gflush_( void )
{
    eggx_gflush();
    return;
}

/* ============= X サーバとのやりとりを扱う関数 ============= */

/* displayをopenし，depthなどを調べる */
int eggx_ggetdisplayinfo( int *rt_depth, int *rt_root_width, int *rt_root_height )
{
    if ( Pc_dis == NULL ) {
	XVisualInfo visual_info;
	XWindowAttributes xvi, root_xvi;
	int i, best, num_visual;
#ifdef USE_XINERAMA
	XineramaScreenInfo *scrns;
	int n_scrns;
#endif
	Pc_dis=XOpenDisplay(NULL) ;
	if( Pc_dis==NULL ){
	    if( rt_depth != NULL ) *rt_depth=-1 ;
	    if( rt_root_width != NULL ) *rt_root_width=-1 ;
	    if( rt_root_height != NULL ) *rt_root_height=-1 ;
	    return(-1) ;
	}
	/* Visualの決定 */
	visual_info.screen = DefaultScreen(Pc_dis) ;
	visual_info.class  = TrueColor ;

	if( Pc_vinfo_ptr!=NULL ) {
	    XFree((char *)Pc_vinfo_ptr) ;
	    Pc_vinfo_ptr=NULL ;
	}
	Pc_vinfo_ptr = XGetVisualInfo( Pc_dis, 
				       VisualScreenMask | VisualClassMask,
				       &visual_info, &num_visual ) ;
	best = -1 ;
	if( Pc_vinfo_ptr!=NULL ){     /* TrueColorを探す */
	    for( i=0 ; i < num_visual ; i++ ){
		if( 0 <= best ){
		    if( Pc_vinfo_ptr[best].depth < Pc_vinfo_ptr[i].depth ){
			if( Pc_vinfo_ptr[i].depth <= 24 ){
			    best = i ;
			}
		    }
		}
		else{
		    best = i ;
		}
	    }
	}
	if( 0 <= best ){	/* 結局同じなら… */
	    if( Pc_vinfo_ptr[best].depth 
		== DefaultDepth(Pc_dis,DefaultScreen(Pc_dis)) )
		best=-1 ;
	}
	if( best == -1 ){
	    Pc_visual=DefaultVisual(Pc_dis,DefaultScreen(Pc_dis));
	    Pc_depth =DefaultDepth(Pc_dis,DefaultScreen(Pc_dis));
	    Pc_cmap  =DefaultColormap(Pc_dis,DefaultScreen(Pc_dis));
	    if( Pc_vinfo_ptr!=NULL ){
		XFree((char *)Pc_vinfo_ptr) ;
		Pc_vinfo_ptr=NULL ;
	    }
	}
	else{
	    Pc_visual=Pc_vinfo_ptr[best].visual ;
	    Pc_depth =Pc_vinfo_ptr[best].depth ;
	    Pc_cmap  =XCreateColormap(Pc_dis, 
				      DefaultRootWindow(Pc_dis),
				      Pc_visual, AllocNone);
	}

	/* 試しにダミーのウィンドゥを作って */
	if ( Pc_zwin == None ) {
	    Pc_zwin = XX_CreateSimpleWindow( Pc_dis, RootWindow(Pc_dis,0), 
					     0, 0, 16, 16, 0,
					     get_color_pixel("#ffffff"),
					     get_color_pixel("#000000") ) ;
	}
	/* まず root の属性を調べる */
	XGetWindowAttributes( Pc_dis, RootWindow(Pc_dis,0) , &root_xvi ) ;
	Pc_root_width = root_xvi.width;
	Pc_root_height = root_xvi.height;
	Pc_display_width = root_xvi.width;
	Pc_display_height = root_xvi.height;
#ifdef USE_XINERAMA
	/* 一番小さいスクリーンのサイズをゲット */
	if ( XineramaIsActive(Pc_dis) ) {
	    if ( (scrns=XineramaQueryScreens(Pc_dis, &n_scrns)) != NULL ) {
		for ( i=0 ; i < n_scrns ; i++ ) {
		    if ( scrns[i].width < Pc_display_width )
			Pc_display_width = scrns[i].width;
		    if ( scrns[i].height < Pc_display_height )
			Pc_display_height = scrns[i].height;
		}
		XFree(scrns);
	    }
	}
#endif
	/* 次に window の属性を調べる */
	XGetWindowAttributes( Pc_dis, Pc_zwin, &xvi ) ;
	if( Pc_depth != xvi.depth ){
	    fprintf(stderr,
		    "EGGX: [WARNING] Detected invalid depth of X server!!\n") ;
	    Pc_depth = xvi.depth ;
	}
	if( 16<=Pc_depth ){
	    Red_depth=(xvi.visual)->red_mask ;
	    Green_depth=(xvi.visual)->green_mask ;
	    Blue_depth=(xvi.visual)->blue_mask ;
	    /* シフトの回数 */
	    Red_sft=sftcount(&(Red_depth)) ;
	    Green_sft=sftcount(&(Green_depth)) ;
	    Blue_sft=sftcount(&(Blue_depth)) ;
	    /* ビットあたりの深さ */
	    Red_depth=bitcount(Red_depth) ;
	    Green_depth=bitcount(Green_depth) ;
	    Blue_depth=bitcount(Blue_depth) ;
	    /*
	    printf("%d %d %d - %d %d %d\n",Red_sft,Green_sft,Blue_sft,
		   Red_depth,Green_depth,Blue_depth) ;
	    */
	}
    }
    if( rt_depth != NULL ) *rt_depth = Pc_depth ;
    if( rt_root_width != NULL ) *rt_root_width  = Pc_root_width ;
    if( rt_root_height != NULL ) *rt_root_height = Pc_root_height ;
    return( 0 ) ;
}

void ggetdisplayinfo_( integer *rt_d, integer *root_w, integer *root_h )
{
    int rt_depth,root_width,root_height ;
    eggx_ggetdisplayinfo(&rt_depth,&root_width,&root_height) ;
    *rt_d   = rt_depth ;
    *root_w = root_width ;
    *root_h = root_height ;
}

static void resize_Pc( int new_n )
{
    struct pctg * new_pc;
    struct pctg * old_pc;
    int i, j, old_n;

    if ( N_Pc == new_n ) return;

    //fprintf(stderr,"DEBUG: resize: %d -> %d\n",N_Pc,new_n);

    old_n = N_Pc;
    old_pc = Pc;
    /* */
    new_pc = (struct pctg *)_eggx_xmalloc(sizeof(struct pctg)*new_n);
    for ( i=0 ; i < old_n && i < new_n ; i++ ) {
	new_pc[i] = old_pc[i];
    }
    for ( i=old_n ; i < new_n ; i++ ) {
	new_pc[i].flg = 0;
	new_pc[i].pwin = None;
	new_pc[i].clipwin = None;
	new_pc[i].hsbarwin = None;
	new_pc[i].vsbarwin = None;
	new_pc[i].win = None;
	new_pc[i].iconwin = None;
	for ( j=0 ; j < MAX_NLAYER ; j++ ) new_pc[i].pix[j] = None;
	new_pc[i].pxgc = None;
	new_pc[i].bggc = None;
	new_pc[i].fsz = -1;
	new_pc[i].fontstruct = NULL;
	new_pc[i].fontset = NULL;
	new_pc[i].gc = None;
	new_pc[i].tmppix = None;
	new_pc[i].winname = NULL;
    }
    /* */
    Pc = new_pc ;
    N_Pc = new_n ;
    if ( old_pc != NULL ) free((void *)old_pc);	/* 最後の最後に free() する */

    return;
}

static void get_size_hints( int xsize, int ysize,
			    Bool fixed_size_window, XSizeHints *size_hints )
{
    size_hints->flags = 0 ;

    if ( Pc_geometry != NULL ) {
	int result, junk;

	size_hints->flags |= PMaxSize ;
	size_hints->max_width = xsize ;
	size_hints->max_height = ysize ;

	if ( fixed_size_window ) {
	    size_hints->flags |= PMinSize ;
	    size_hints->min_width = xsize ;
	    size_hints->min_height = ysize ;
	}

	size_hints->flags |= USSize ;
	size_hints->width = xsize ;
	size_hints->height = ysize ;

        size_hints->x = 0;
        size_hints->y = 0;

	result = XWMGeometry( Pc_dis, DefaultScreen(Pc_dis), 
			      Pc_geometry, NULL, 
			      Pc_bdr_width, size_hints,
			      &(size_hints->x), &(size_hints->y),
			      &(size_hints->width), &(size_hints->height), 
			      &junk );
	if ( (result & XValue) || (result & YValue) ) {
	    size_hints->flags |= USPosition ;
	}
	if ( (result & XValue) == 0 ) size_hints->x = 0;
	if ( (result & YValue) == 0 ) size_hints->y = 0;

	if ( (result & WidthValue) == 0 ) {
	    if ( result & XNegative )
		size_hints->x -= (xsize - size_hints->width);
	    size_hints->width = xsize;
	}
	if ( (result & HeightValue) == 0 ) {
	    if ( result & YNegative )
		size_hints->y -= (ysize - size_hints->height);
	    size_hints->height = ysize;
	}

	//fprintf(stderr,"debug: x,y=%d,%d\n",size_hints->x,size_hints->y);
	//fprintf(stderr,"debug: w,h=%d,%d\n",
	//	size_hints->width,size_hints->height);

	if ( (result & XValue) || (result & YValue) ) {
	    size_hints->flags |= PWinGravity;
	    if ( result & XNegative ) {
		if ( result & YNegative )
		    size_hints->win_gravity = SouthEastGravity;
		else 
		    size_hints->win_gravity = NorthEastGravity;
	    }
	    else {
		if ( result & YNegative )
		    size_hints->win_gravity = SouthWestGravity;
		else
		    size_hints->win_gravity = NorthWestGravity;
	    }
	}

    }
    else{
	size_hints->x = 0 ;
	size_hints->y = 0 ;

	if ( fixed_size_window ) {
	    size_hints->flags |= PMinSize ;
	    size_hints->min_width = xsize ;
	    size_hints->min_height = ysize ;
	}

	size_hints->flags |= PMaxSize ;
	size_hints->max_width = xsize ;
	size_hints->max_height = ysize ;

	size_hints->flags |= USSize ;
	size_hints->width = xsize ;
	size_hints->height = ysize ;

	if ( ! fixed_size_window ) {
	    if ( ROOT_MARGIN < Pc_display_width && 
		 Pc_display_width < xsize + ROOT_MARGIN ) {
		size_hints->width = Pc_display_width - ROOT_MARGIN;
	    }
	    if ( ROOT_MARGIN < Pc_display_height &&
		 Pc_display_height < ysize + ROOT_MARGIN ) {
		size_hints->height = Pc_display_height - ROOT_MARGIN;
	    }
	}
    }

    return;
}

/* 窓を開く */
void gopen_( integer *xsize, integer *ysize, integer *rtnum )
{
    char int_1[] = "1" ;
    Bool fixed_size_window;
    XGCValues gv ;
    XSetWindowAttributes att ;		/* 窓属性の変数 */
    XClassHint classHint ;
    XSizeHints size_hints = { 0 } ;
    /* XTextProperty name ; */
    int i, num=0 ;
#if 0
    int f ;
#endif
    /* */
    const char *pname_to_disp = NULL ;
    /* スクロールバー用のピクセル値 */
    unsigned long wd_background_pixel = 0 ;
    unsigned long wd_shadow_pixel = 0 ;
    unsigned long wd_trough_pixel = 0 ;
    unsigned long wd_highlight_pixel = 0 ;

    if( Wn < 0 ){
	if( Pc_dis==NULL ){	/* Xserverとの接続 */
	    if( eggx_ggetdisplayinfo(NULL,NULL,NULL) ){
		fprintf(stderr,"EGGX: [ERROR] Cannot open display.\n") ;
		exit(1) ;
	    }
	}
	if( Wn == -1 ){		/* 1回きりの実行 */
	    Pid=getpid() ;
#if 0
	    f=rmproperexec() ;	/* 前回のテンポラリファイルを削除 */
	    if( f ){
	      fprintf(stderr,"警告: /tmp ディレクトリがオープンできません．\n") ;
	    }
#endif
	    /* atexit( &rmexecfile ) ; */
#ifdef __CYGWIN__
	    atexit( &remove_removelist ) ;
#endif
	    atexit( &killallchld ) ;
	    Pname = get_procname(Pid) ;
	    if( Pname == NULL ) Pname = _Pname ;
	    if( setlocale(LC_CTYPE, "") == NULL /* && 0 */ )
		fprintf(stderr,"EGGX: [WARNING] Cannot set locale.\n") ;
	    /* atexit( &eggx_gcloseall ) ; */
	}
	set_eggxhandlers() ;		/* 割込みハンドラ登録 */
	Wn = 0 ;
    }

    wait_child() ;			/* imgsave中は待たせる */

    if ( N_Pc == 0 ) {
	resize_Pc(2);			/* 0はplots用に予約し最低2個は確保 */
    }
    if ( rtnum == NULL ) {		/* plotsから来た場合 */
	if ( Pc[0].flg == 1 ) return;	/* すでにオープン済み */
	num = 0;
	Pc[0].flg = 1;
    }
    else {				/* あいてる番号を探す */
	for ( i=1 ; i < N_Pc ; i++ ) {	/* 0はplots用に予約 */
	    if ( Pc[i].flg == 0 ) {
		num = i;
		Pc[i].flg = 1;
		break;
	    }
	}
	if ( i == N_Pc ) {		/* あいてない時 */
	    int old_n = N_Pc ;
	    resize_Pc(N_Pc + 1);	/* 1個だけ増やす */
	    num = old_n;
	    Pc[old_n].flg = 1;
	}
    }
    Pc[num].wly=0 ;
    Pc[num].sly=0 ;

    Pc[num].attributes = Pc_attributes ;

    fixed_size_window = ( (Pc[num].attributes & SCROLLBAR_INTERFACE) == 0 ||
	  (Pc[num].attributes & (OVERRIDE_REDIRECT | DOCK_APPLICATION)) != 0 );

    get_size_hints( *xsize, *ysize, fixed_size_window, &size_hints );

    if ( Pc_bgcolor == NULL ) {
	Pc[num].bgcolor = get_color_pixel("#000000");
    }
    else {
	Pc[num].bgcolor = get_color_pixel(Pc_bgcolor);
    }

    /* 窓の生成 */
    if ( fixed_size_window ) {
	Pc[num].pwin = XX_CreateSimpleWindow( Pc_dis, RootWindow(Pc_dis,0), 
					      size_hints.x, size_hints.y,
					      *xsize, *ysize, 
					      Pc_bdr_width,
					      get_color_pixel("#ffffff"),
					      Pc[num].bgcolor );
	Pc[num].clipwin = Pc[num].pwin;	/* 親窓と同じになる */
	Pc[num].hsbarwin = None;
	Pc[num].vsbarwin = None;
	Pc[num].win = Pc[num].clipwin;	/* 親窓と同じになる */
    }
    else {
	/* スクロールバーのピクセル値を保存 */
	wd_background_pixel = get_color_pixel(WD_BACKGROUND_COLOR);
	wd_shadow_pixel = get_color_pixel(WD_SHADOW_COLOR);
	wd_trough_pixel = get_color_pixel(WD_TROUGH_COLOR);
	wd_highlight_pixel = get_color_pixel(WD_HIGHLIGHT_COLOR);
	/* 親窓 */
	Pc[num].pwin = XX_CreateSimpleWindow( Pc_dis, RootWindow(Pc_dis,0), 
					      size_hints.x, size_hints.y,
					      size_hints.width,
					      size_hints.height,
					      Pc_bdr_width,
					      wd_background_pixel,
					      wd_background_pixel );
	/* 小窓... */
	Pc[num].clipwin = XX_CreateSimpleWindow( Pc_dis, Pc[num].pwin,
						 0, 0,
						 size_hints.width, 
						 size_hints.height,
						 0, get_color_pixel("#ffffff"),
						 Pc[num].bgcolor );
	Pc[num].hsbarwin = XX_CreateSimpleWindow( Pc_dis, Pc[num].pwin,
						  0, -SCROLLBAR_WIDTH,
						  size_hints.width,
						  SCROLLBAR_WIDTH,
						  0, wd_background_pixel, 
						  wd_trough_pixel );
	Pc[num].vsbarwin = XX_CreateSimpleWindow( Pc_dis, Pc[num].pwin,
						  -SCROLLBAR_WIDTH, 0,
						  SCROLLBAR_WIDTH,
						  size_hints.height,
						  0, wd_background_pixel, 
						  wd_trough_pixel );
	Pc[num].win = XX_CreateSimpleWindow( Pc_dis, Pc[num].clipwin,
					     0, 0,
					     *xsize, *ysize, 
					     0, get_color_pixel("#ffffff"),
					     Pc[num].bgcolor );
    }

    XSetWMNormalHints( Pc_dis, Pc[num].pwin, &size_hints) ;

    if( Pc[num].attributes & DOCK_APPLICATION ){
	XWMHints mywmhints ;
	Pc[num].iconwin = XX_CreateSimpleWindow( Pc_dis, RootWindow(Pc_dis,0), 
						 size_hints.x, size_hints.y,
						 *xsize, *ysize, Pc_bdr_width,
						 get_color_pixel("#ffffff"),
						 Pc[num].bgcolor ) ;
	XSetWMNormalHints( Pc_dis, Pc[num].iconwin, &size_hints ) ;
	mywmhints.initial_state = WithdrawnState ;
        mywmhints.icon_window = Pc[num].iconwin ;
        mywmhints.icon_x = size_hints.x ;
        mywmhints.icon_y = size_hints.y ;
        mywmhints.window_group = Pc[num].pwin ;
        mywmhints.flags = StateHint | IconWindowHint | 
	    IconPositionHint | WindowGroupHint ;
	XSetWMHints( Pc_dis, Pc[num].pwin, &mywmhints ) ;
    }
    else Pc[num].iconwin = None ;

    for ( i=0 ; i < MAX_NLAYER ; i++ ) Pc[num].pix[i] = None;
    Pc[num].pix[0] = XCreatePixmap(Pc_dis,Pc[num].win,*xsize,*ysize,Pc_depth);
    Pc[num].tmppix = None;

    gv.fill_style = FillTiled;
    gv.fill_rule = WindingRule;
    gv.tile = Pc[num].pix[0];
    Pc[num].pxgc = XCreateGC( Pc_dis, Pc[num].win, 
			      GCFillStyle | GCFillRule | GCTile, &gv );
    XSetGraphicsExposures( Pc_dis, Pc[num].pxgc, False ) ;
    
    /* 絵を保存する設定をする */
    /* att.backing_store = WhenMapped ; */
    /* Xサーバの負担を減らすため，子プロセスの再描画に頼る */
    /*
    att.backing_store = NotUseful ;
    XChangeWindowAttributes( Pc_dis, Pc[num].win, CWBackingStore, &att ) ;
    */

    /* ウィンドゥに枠をつけない */
    if ( Pc[num].attributes & OVERRIDE_REDIRECT ) {
	att.override_redirect = True;
	XChangeWindowAttributes( Pc_dis, Pc[num].pwin,
				 CWOverrideRedirect, &att );
    }

    if( Pc_storename != NULL ) pname_to_disp = Pc_storename ;
    else pname_to_disp = Pname ;

    XStoreName( Pc_dis, Pc[num].pwin, pname_to_disp ) ;
    
    if( Pc_iconname != NULL )
	XSetIconName( Pc_dis, Pc[num].pwin, Pc_iconname ) ;
    else
	XSetIconName( Pc_dis, Pc[num].pwin, Pname ) ;
    if( Pc_classhint_res_name != NULL )
	classHint.res_name = Pc_classhint_res_name ;
    else classHint.res_name = Pname ;
    if( Pc_classhint_res_class != NULL ){
	classHint.res_class = Pc_classhint_res_class ;
	XSetClassHint( Pc_dis, Pc[num].pwin , &classHint);
    }
    else{
	classHint.res_class = _eggx_xstrdup_toupper(Pname) ;
	XSetClassHint( Pc_dis, Pc[num].pwin , &classHint);
	free(classHint.res_class) ;
    }

    if( Pc[num].iconwin != None ){
	XStoreName( Pc_dis, Pc[num].iconwin, pname_to_disp ) ;
	if( Pc_iconname != NULL )
	    XSetIconName( Pc_dis, Pc[num].iconwin, Pc_iconname ) ;
	else
	    XSetIconName( Pc_dis, Pc[num].iconwin, Pname ) ;
	if( Pc_classhint_res_name != NULL )
	    classHint.res_name = Pc_classhint_res_name ;
	else classHint.res_name = Pname ;
	if( Pc_classhint_res_class != NULL ){
	    classHint.res_class = Pc_classhint_res_class ;
	    XSetClassHint( Pc_dis, Pc[num].iconwin , &classHint);
	}
	else{
	    classHint.res_class = _eggx_xstrdup_toupper(Pname) ;
	    XSetClassHint( Pc_dis, Pc[num].iconwin , &classHint);
	    free(classHint.res_class) ;
	}
    }

    /*
    if( XStringListToTextProperty(&Pname, 1, &name) != 0 ){
	XSetWMName( Pc_dis, Pc[num].pwin, &name);
    }
    */
    XSelectInput( Pc_dis, Pc[num].win, ExposureMask ) ;
    XMapWindow( Pc_dis, Pc[num].pwin ) ;	/* 窓の表示 */
    XMapWindow( Pc_dis, Pc[num].clipwin ) ;
    XMapWindow( Pc_dis, Pc[num].win ) ;
    if ( Pc[num].hsbarwin != None ) XMapWindow(Pc_dis, Pc[num].hsbarwin);
    if ( Pc[num].vsbarwin != None ) XMapWindow(Pc_dis, Pc[num].vsbarwin);

    while ( 1 ) {				/* 窓が開くの待つループ */
	XEvent ev;
	XNextEvent( Pc_dis, &ev );
	if ( ev.type == Expose ) break;
    };

    Pc[num].gc = XCreateGC( Pc_dis, Pc[num].pwin,0,0 ) ;
    XSetGraphicsExposures( Pc_dis, Pc[num].gc, False ) ;
    Pc[num].bggc = XCreateGC( Pc_dis, Pc[num].pwin,0,0 ) ;
    XSetGraphicsExposures( Pc_dis, Pc[num].bggc, False ) ;
    
    XSetForeground( Pc_dis, Pc[num].bggc, Pc[num].bgcolor ) ;
    XFillRectangle( Pc_dis, Pc[num].pix[0], Pc[num].bggc, 0,0,*xsize,*ysize ) ;
    //XDrawRectangle(Pc_dis, Pc[num].pix[0], Pc[num].bggc, 0,0,*xsize,*ysize);

    XSetForeground( Pc_dis, Pc[num].gc, get_color_pixel("#ffffff") ) ;

    Pc[num].fontstruct = NULL ;
    /* フォントセット */
    Pc[num].fontset = NULL ;
    
    XFlush( Pc_dis ) ;			/* リクエストの強制送信 */

    Pc[num].wszx = *xsize ;		/* グラフィックサイズ */
    Pc[num].wszy = *ysize ;
    Pc[num].acx0 = 0.0 ;		/* ウィンドゥ */
    Pc[num].acy0 = 0.0 ;
    Pc[num].scalex = 1 ;
    Pc[num].scaley = 1 ;

    Pc[num].linewidth = 1 ;
    Pc[num].linestyle = LineSolid ;
    Pc[num].gcfunc = GXcopy ;

    if( rtnum!=NULL ) *rtnum=num ;

    XSelectInput( Pc_dis, Pc[num].win, 0 ) ;

    if ( Cpid <= 0 ) {
	pid_t cpid_boot;
	char arg2[16],  arg3[16],  arg4[16],  arg5[16];
	char arg6[16],  arg7[16],  arg8[16];
	char *args[10];
	sprintf(arg2,"%ld",(long)Pid) ;
	sprintf(arg3,"%lu",wd_background_pixel) ;
	sprintf(arg4,"%lu",wd_shadow_pixel) ;
	sprintf(arg5,"%lu",wd_trough_pixel) ;
	sprintf(arg6,"%lu",wd_highlight_pixel) ;
	sprintf(arg7,"%d",SCROLLBAR_WIDTH) ;
	sprintf(arg8,"%lu",Pc_zwin) ;

	args[0]=Exec_slave_tmp ;
	args[1]=int_1 ;
	args[2]=arg2 ;
	args[3]=arg3 ;
	args[4]=arg4 ;
	args[5]=arg5 ;
	args[6]=arg6 ;
	args[7]=arg7 ;
	args[8]=arg8 ;
	args[9]=NULL ;
	/* */
	if ( mkexecfile() == -1 ) {
	    fprintf(stderr,"EGGX: [ERROR] Cannot create a file in /tmp/.\n");
	    exit(1);
	}
	if ( Exec_fd != -1 ) {
	    close(Exec_fd);
	    Exec_fd = -1;
	}
	/* */
	XSelectInput( Pc_dis, Pc_zwin, ExposureMask );
	XFlush( Pc_dis );
	XSync( Pc_dis, 0 );
	/* */
	if ( (Cpid=(cpid_boot=fork())) < 0 ) {
	    fprintf(stderr,"EGGX: [ERROR] fork() failed.\n");
	    exit(1);
	}
	if ( cpid_boot == 0 ) {	/* 子プロセス */
	    execv(*args,args);
	    fprintf(stderr,"EGGX: [ERROR] Cannot exec child process.\n");
	    kill(getppid(), SIGTERM);
	    _exit(-1);
	}
	/* ↓親↓ */
	/* 子から「準備完了」を示すイベントが来るまで待つ */
	while ( 1 ) {
	    XEvent ev;
	    XNextEvent( Pc_dis, &ev );
	    if ( ev.type == GraphicsExpose &&
		 ev.xgraphicsexpose.send_event == True &&
		 ev.xgraphicsexpose.minor_code == MCODE_DUMMY ) break;
	};
	XSelectInput( Pc_dis, Pc_zwin, 0 ) ;
	/* fprintf(stderr,"DEBUG: child's reply:[GraphicsExpose] is OK\n"); */
	rmexecfile();
	/* 子でボタンイベントを扱うかどうかを知らせる */
	eggx_gsetnonblock(Pc_nonblock);
    }

    if ( 1 /* Pc[num].attributes & AUTOREDRAW */ ) {

	send_command_to_child(num, MCODE_PWIN_ID, Pc[num].pwin, False);
	//fprintf(stderr,"eggx: [DEBUG] pwin=%ld\n", (long)Pc[num].pwin);
	send_command_to_child(num, MCODE_CLIPWIN_ID, Pc[num].clipwin, False);
	send_command_to_child(num, MCODE_HSBARWIN_ID, Pc[num].hsbarwin, False);
	send_command_to_child(num, MCODE_VSBARWIN_ID, Pc[num].vsbarwin, False);
	send_command_to_child(num, MCODE_WIN_ID, Pc[num].win, False);
	send_command_to_child(num, MCODE_ICONWIN_ID, Pc[num].iconwin, False);
	send_command_to_child(num, MCODE_BL_ORIGIN, 
			(Pc[num].attributes & BOTTOM_LEFT_ORIGIN) != 0, False);
	send_command_to_child(num, MCODE_PIXMAP_ID, Pc[num].pix[0], True);

	XSync( Pc_dis, 0 );

	{
	    /* eggx_msleep(100) ; */
	    /*
	      HACK to CYGWIN problem [2020.9.15]
	      Ad hoc bug fix for Xwin crash
	    */
	    char *pname1 = NULL ;
	    /* change window name --- force to update internal info */
	    pname1 = _eggx_asprintf("%s ",pname_to_disp) ;
	    XStoreName( Pc_dis, Pc[num].pwin, pname1 ) ;
	    if ( Pc[num].iconwin != None ) 
		XStoreName( Pc_dis, Pc[num].iconwin, pname1 ) ;
	    XFlush( Pc_dis ) ;
	    XSync( Pc_dis, 0 ) ;
	    /* */
	    XStoreName( Pc_dis, Pc[num].pwin, pname_to_disp ) ;
	    if ( Pc[num].iconwin != None ) 
		XStoreName( Pc_dis, Pc[num].iconwin, pname_to_disp ) ;
	    XFlush( Pc_dis ) ;
	    if ( pname1 != NULL ) free(pname1) ;
	}

	/*
	{
	    Status st;
	    XWindowAttributes junk;
	    st = XGetWindowAttributes( Pc_dis, Pc[num].pwin, &junk );
	    fprintf(stderr,"EGGX: [DEBUG] st=%d\n", (int)st);
	    st = XGetWindowAttributes( Pc_dis, Pc[num].clipwin, &junk );
	    fprintf(stderr,"EGGX: [DEBUG]  st=%d\n", (int)st);
	    st = XGetWindowAttributes( Pc_dis, Pc[num].win, &junk );
	    fprintf(stderr,"EGGX: [DEBUG]   st=%d\n", (int)st);
	}
	*/
	
	XSync( Pc_dis, 0 );

	send_command_to_child(num, MCODE_ENABLE, None, False);
	
	/* */
	XSelectInput( Pc_dis, Pc_zwin, ExposureMask );

	send_command_to_child(0, MCODE_NEEDS_REPLY, None, True);

	XSync( Pc_dis, 0 );

	/* 子から「準備完了」を示すイベントが来るまで待つ */
	while ( 1 ) {
	    XEvent ev;
	    XNextEvent( Pc_dis, &ev );
	    if ( ev.type == GraphicsExpose &&
		 ev.xgraphicsexpose.send_event == True &&
		 ev.xgraphicsexpose.minor_code == MCODE_DUMMY ) break;
	};

	XSelectInput( Pc_dis, Pc_zwin, 0 ) ;

	/* スクロールバーのキーマスクのデフォルト設定 */
	Pc[num].sbarkeymask = SCROLLBAR_DEFAULT_KEYMASK;
	/* キーマスク情報を子に送信 */
	send_command_to_child(num, MCODE_KEYMASK, Pc[num].sbarkeymask, True);

	
	XSync( Pc_dis, 0 );
    }

    restore_xinput_selection( num );

    Pc[num].winname = _eggx_xstrdup(pname_to_disp) ;	/* register */
    
    return;
}

void plots_( void )
{
    integer x=640,y=400 ;
    gopen_( &x, &y, NULL ) ;
}

int eggx_gopen( int xsize, int ysize )
{
    integer rtnum,xs,ys ;
    xs=(integer)xsize ;
    ys=(integer)ysize ;
    gopen_( &xs, &ys, &rtnum ) ;
    return( (int)rtnum ) ;
}

/* 窓を閉じる */
void eggx_gclose( int wn )
{
    int i, max_idx;
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    if ( 1 /* Pc[wn].attributes & AUTOREDRAW */ ) {
	send_command_to_child(wn, MCODE_DISABLE, None, False);
	XSelectInput( Pc_dis, Pc_zwin, ExposureMask );
	send_command_to_child(0, MCODE_NEEDS_REPLY, None, True);
	XSync( Pc_dis, 0 );
	/* 子の返事を示すイベントが来るまで待つ */
	while ( 1 ) {
	    XEvent ev;
	    XNextEvent( Pc_dis, &ev );
	    if ( ev.type == GraphicsExpose &&
		 ev.xgraphicsexpose.send_event == True &&
		 ev.xgraphicsexpose.minor_code == MCODE_DUMMY ) break;
	};
	XSelectInput( Pc_dis, Pc_zwin, 0 ) ;
    }
    /* */
    XFreeGC( Pc_dis, Pc[wn].pxgc ) ;
    Pc[wn].pxgc = None;
    XFreeGC( Pc_dis, Pc[wn].bggc ) ;
    Pc[wn].bggc = None;
    XFreeGC( Pc_dis, Pc[wn].gc ) ;
    Pc[wn].gc = None;
    /* */
    for ( i=0 ; i < MAX_NLAYER ; i++ ) {
	if ( Pc[wn].pix[i] != None ) {
	    XFreePixmap( Pc_dis, Pc[wn].pix[i] ) ;
	    Pc[wn].pix[i] = None;
	}
    }
    if ( Pc[wn].tmppix != None ) {
	XFreePixmap( Pc_dis, Pc[wn].tmppix ) ;
	Pc[wn].tmppix = None;
    }
    /* */
    Pc[wn].fontset = NULL;
    if ( Pc[wn].fontstruct != NULL ) {
	XFreeFont(Pc_dis,Pc[wn].fontstruct) ;
	Pc[wn].fontstruct = NULL ;
    }
    /* */
    //if (Pc[wn].win != Pc[wn].clipwin) XDestroyWindow(Pc_dis, Pc[wn].win);
    //if (Pc[wn].clipwin != Pc[wn].pwin) XDestroyWindow(Pc_dis, Pc[wn].clipwin);
    //if (Pc[wn].hsbarwin != None) XDestroyWindow(Pc_dis, Pc[wn].hsbarwin);
    //if (Pc[wn].vsbarwin != None) XDestroyWindow(Pc_dis, Pc[wn].vsbarwin);
    XDestroySubwindows(Pc_dis, Pc[wn].pwin);
    XDestroyWindow(Pc_dis, Pc[wn].pwin);
    Pc[wn].pwin = None;
    Pc[wn].clipwin = None;
    Pc[wn].hsbarwin = None;
    Pc[wn].vsbarwin = None;
    Pc[wn].win = None;
    if ( Pc[wn].iconwin != None ) {
	XDestroyWindow( Pc_dis, Pc[wn].iconwin ) ;
	Pc[wn].iconwin = None;
    }
    /* */
    if ( Pc[wn].winname != NULL ) {
	free(Pc[wn].winname) ;
	Pc[wn].winname = NULL ;
    }
    /* */
    Pc[wn].fsz=-1;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
    Pc[wn].flg = 0 ;
    /* */
    if ( Pc_evrec_wid == wn ) Pc_evrec_wid = -1;
    /* */
    max_idx = 0;
    for ( i=0 ; i < N_Pc ; i++ ) {
	if ( Pc[i].flg != 0 ) max_idx = i;
    }
    if ( max_idx < 1 ) max_idx = 1;	/* 最低2個は確保 */
    if ( 2 < N_Pc && N_Pc != max_idx + 1 ) resize_Pc(max_idx + 1);
    /* */
    return;
}

void gclose_( integer *wn )
{
    eggx_gclose( *wn ) ;
}

/* すべての窓を閉じて，Xサーバと断つ */
void eggx_gcloseall( void )
{
    int i;

    /* 接続されてない時 */
    if ( Wn < 0 ) return;
    if ( getpid() != Pid ) return;

    wait_child();	/* imgsave中は待たせる */

    /* すべての窓を閉じていく */
    for ( i=0 ; i < N_Pc ; i++ ) {
	if ( Pc[i].flg != 0 ) {
	    eggx_gclose(i);
	}
    }

    /* 子プロセスをターミネート */
    if ( 0 < Cpid ) {
	kill(Cpid, SIGTERM);
	while ( 0 < Cpid ) {
	    eggx_msleep( 10 );
	}
    }

#ifdef __CYGWIN__
    remove_removelist();
#endif
    /* 割込みハンドラ元に戻す */
    set_defaulthandlers();
    /* フォントセットのキャッシュを開放 */
    free_fontset_cache();
    /* cmapテーブルの開放 */
    free_cmap_table();
    /* */
    if ( Tmp_img24buf != NULL ) {
	Sz_tmp_img24buf = 0;
	free(Tmp_img24buf);
	Tmp_img24buf = NULL;
    }
    if ( 0 < Pix_mask_width ) {
	XFreeGC( Pc_dis, Gc_pix_mask );
	Gc_pix_mask = None;
	XFreePixmap( Pc_dis, Pix_mask );
	Pix_mask = None;
	Pix_mask_width = 0;
	Pix_mask_height = 0;
    }
    /* */
    if ( Pc_zwin != None ) {	/* 通信用窓 */
	XDestroyWindow( Pc_dis, Pc_zwin );
	Pc_zwin = None;
    }
    if ( Pc_cmap != DefaultColormap(Pc_dis,DefaultScreen(Pc_dis)) ) {
	XFreeColormap(Pc_dis,Pc_cmap);
	Pc_cmap = None;
    }
    if ( Pc_vinfo_ptr != NULL ) {
	XFree((char *)Pc_vinfo_ptr);
	Pc_vinfo_ptr = NULL;
    }
    XCloseDisplay( Pc_dis );	/* Xserverと断線 */
    Pc_dis = NULL;
    if ( 0 < N_Pc ) {		/* ウィンドゥ管理用バッファも開放する */
	N_Pc = 0;
	free((void *)Pc);
	Pc = NULL;
    }
    Wn = -2;

    return;
}

void gcloseall_( void )
{
    eggx_gcloseall();
    return;
}

/* ============= レイヤまわりを扱う関数 ============= */

static Pixmap create_layer( int wn )
{
    Pixmap rt;
    rt = XCreatePixmap(Pc_dis, Pc[wn].win, Pc[wn].wszx, Pc[wn].wszy, Pc_depth);
    XFillRectangle( Pc_dis, rt, Pc[wn].bggc, 0, 0, Pc[wn].wszx, Pc[wn].wszy );
    //XDrawRectangle(Pc_dis, rt, Pc[wn].bggc, 0, 0, Pc[wn].wszx, Pc[wn].wszy);
    return rt;
}

void eggx_gresize( int wn, int xsize, int ysize )
{
    Bool fixed_size_window;
    XSizeHints size_hints = { 0 } ;
    XGCValues gv;
    int i;
    int old_wszx, old_wszy;

    if ( xsize < 1 || ysize < 1 ) return;
    if ( xsize == Pc[wn].wszx && ysize == Pc[wn].wszy ) return;

    fixed_size_window = ( (Pc[wn].attributes & SCROLLBAR_INTERFACE) == 0 ||
	   (Pc[wn].attributes & (OVERRIDE_REDIRECT | DOCK_APPLICATION)) != 0 );

    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */

    old_wszx = Pc[wn].wszx;
    old_wszy = Pc[wn].wszy;
    Pc[wn].wszx = xsize;
    Pc[wn].wszy = ysize;

    /* Pixmap をコピーする */
    if ( (Pc[wn].attributes & BOTTOM_LEFT_ORIGIN) ) {
	gv.ts_x_origin = 0;
	gv.ts_y_origin = Pc[wn].wszy - old_wszy;
    }
    else {
	gv.ts_x_origin = 0;
	gv.ts_y_origin = 0;
    }
    XChangeGC(Pc_dis, Pc[wn].pxgc, GCTileStipXOrigin | GCTileStipYOrigin, &gv);
    for ( i=0 ; i < MAX_NLAYER ; i++ ) {
	if ( Pc[wn].pix[i] != None ) {
	    Pixmap px = create_layer(wn);
	    XFillRectangle( Pc_dis, px, Pc[wn].bggc,
			    0,0, Pc[wn].wszx,Pc[wn].wszy );
	    gv.tile = Pc[wn].pix[i];
	    XChangeGC( Pc_dis, Pc[wn].pxgc, GCTile, &gv ) ;
	    XFillRectangle( Pc_dis, px, Pc[wn].pxgc,
			    gv.ts_x_origin, gv.ts_y_origin,
			    old_wszx, old_wszy );
	    if ( i == Pc[wn].sly ) {	/* 表示レイヤの場合 */
		if ( 1 /* Pc[wn].attributes & AUTOREDRAW */ ) {
		    send_command_to_child(wn, MCODE_PIXMAP_ID, px, False);
		    XSelectInput( Pc_dis, Pc_zwin, ExposureMask );
		    send_command_to_child(0, MCODE_NEEDS_REPLY, None, True);
		    /* 子の返事を示すイベントが来るまで待つ */
		    while ( 1 ) {
			XEvent ev;
			XNextEvent( Pc_dis, &ev );
			if ( ev.type == GraphicsExpose &&
			     ev.xgraphicsexpose.send_event == True &&
			     ev.xgraphicsexpose.minor_code == MCODE_DUMMY ) break;
		    };
		    XSelectInput( Pc_dis, Pc_zwin, 0 ) ;
		}
	    }
	    /* 子の返事を待たずにこれをやるとエラい事になるぞい */
	    XFreePixmap(Pc_dis, Pc[wn].pix[i]);
	    Pc[wn].pix[i] = px;
	}
    }
    gv.ts_x_origin = 0;
    gv.ts_y_origin = 0;
    XChangeGC(Pc_dis, Pc[wn].pxgc, GCTileStipXOrigin | GCTileStipYOrigin, &gv);

    if ( Pc[wn].tmppix != None ) {
	XFreePixmap( Pc_dis, Pc[wn].tmppix ) ;
	Pc[wn].tmppix = None;
    }

    get_size_hints( xsize, ysize, fixed_size_window, &size_hints );

    if ( fixed_size_window ) {
	XSetWMNormalHints( Pc_dis, Pc[wn].win, &size_hints);
	XResizeWindow(Pc_dis, Pc[wn].win, xsize, ysize);
	if ( Pc[wn].iconwin != None ) {
	    XSetWMNormalHints( Pc_dis, Pc[wn].iconwin, &size_hints );
	    XResizeWindow(Pc_dis, Pc[wn].iconwin, xsize, ysize);
	}
    }
    else {
	XResizeWindow(Pc_dis, Pc[wn].win, xsize, ysize);
	XSetWMNormalHints( Pc_dis, Pc[wn].pwin, &size_hints);
	XResizeWindow(Pc_dis, Pc[wn].pwin,
		      size_hints.width, size_hints.height);
    }

    gv.tile = Pc[wn].pix[Pc[wn].sly];
    XChangeGC( Pc_dis, Pc[wn].pxgc, GCTile, &gv ) ;
    XFillRectangle( Pc_dis, Pc[wn].win, Pc[wn].pxgc,
		    0,0, Pc[wn].wszx,Pc[wn].wszy );

    XFlush( Pc_dis );

    {
	/*
	  HACK to CYGWIN problem [2020.9.15]
	  Ad hoc bug fix that child process cannot catch resizing event
	*/
	const char *pname_to_disp = " " ;
	char *pname1 = NULL ;
	if ( Pc[wn].winname != NULL ) {
	    pname_to_disp = Pc[wn].winname ;
	}

	/* change window name --- force to update internal info */
	pname1 = _eggx_asprintf("%s ",pname_to_disp) ;
	XStoreName( Pc_dis, Pc[wn].pwin, pname1 ) ;
	if ( Pc[wn].iconwin != None ) 
	    XStoreName( Pc_dis, Pc[wn].iconwin, pname1 ) ;
	XFlush( Pc_dis ) ;
	XSync( Pc_dis, 0 ) ;
	/* */
	XStoreName( Pc_dis, Pc[wn].pwin, pname_to_disp ) ;
	if ( Pc[wn].iconwin != None ) 
	    XStoreName( Pc_dis, Pc[wn].iconwin, pname_to_disp ) ;
	XFlush( Pc_dis ) ;
	if ( pname1 != NULL ) free(pname1) ;
    }
    
    Ihflg = 0;		/* 割り込み禁止解除 */
    chkexit();

    return;
}

void eggx_copylayer( int wn, int src, int dst )
{
    XGCValues gv ;

    if ( src < 0 || MAX_NLAYER <= src ||
	 dst < 0 || MAX_NLAYER <= dst ||
	 Pc[wn].pix[src] == None || Pc[wn].pix[dst] == None ) {
	fprintf(stderr,"EGGX: [ERROR] Invalid layer index at copylayer()\n");
	return;
    }

    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */

    gv.tile = Pc[wn].pix[src] ;
    XChangeGC( Pc_dis, Pc[wn].pxgc, GCTile, &gv ) ;

    XFillRectangle( Pc_dis, Pc[wn].pix[dst], Pc[wn].pxgc, 
		    0,0, Pc[wn].wszx,Pc[wn].wszy ) ;
    if( dst == Pc[wn].sly ){
	XFillRectangle( Pc_dis, Pc[wn].win, Pc[wn].pxgc, 
			0,0, Pc[wn].wszx,Pc[wn].wszy ) ;
	if( Pc[wn].iconwin != None ){
	    XFillRectangle( Pc_dis, Pc[wn].iconwin, Pc[wn].pxgc, 
			    0,0, Pc[wn].wszx,Pc[wn].wszy ) ;
	}
	do_auto_flush() ;
    }

    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

void copylayer_( integer *wn, integer *src, integer *dst )
{
    eggx_copylayer( *wn, *src, *dst ) ;
}

/* 表示，書き込むレイヤを選択する．sl:表示 wl:書込 */
void eggx_layer( int wn, int sl, int wl )
{
    if ( sl < 0 || MAX_NLAYER <= sl ) sl = Pc[wn].sly;
    if ( wl < 0 || MAX_NLAYER <= wl ) wl = Pc[wn].wly;
    /* */
    if ( Pc[wn].pix[sl] == None ) Pc[wn].pix[sl] = create_layer(wn);
    if ( Pc[wn].pix[wl] == None ) Pc[wn].pix[wl] = create_layer(wn);
    if( sl != Pc[wn].sly ){
	XGCValues gv ;
	wait_child() ;	/* imgsave中は待たせる */
	Ihflg=1 ;	/* 割り込み禁止 */
	if ( 1 /* Pc[wn].attributes & AUTOREDRAW */ ) {
	    send_command_to_child(wn, MCODE_PIXMAP_ID, Pc[wn].pix[sl], True);
	}
	gv.tile = Pc[wn].pix[sl] ;
	XChangeGC( Pc_dis, Pc[wn].pxgc, GCTile, &gv ) ;
	XFillRectangle( Pc_dis, Pc[wn].win, Pc[wn].pxgc, 
			0,0, Pc[wn].wszx,Pc[wn].wszy ) ;
	if( Pc[wn].iconwin != None ){
	    XFillRectangle( Pc_dis, Pc[wn].iconwin, Pc[wn].pxgc, 
			    0,0, Pc[wn].wszx,Pc[wn].wszy ) ;
	}
	do_auto_flush() ;
	Ihflg=0 ;	/* 割り込み禁止解除 */
	chkexit() ;
    }
    Pc[wn].sly = sl ;
    Pc[wn].wly = wl ;
}

void layer_( integer *wn, integer *sly, integer *wly )
{
    eggx_layer( *wn, *sly, *wly ) ;
}

void eggx_gscroll( int wn, int x, int y, int clr )
{
    int ac_x, ac_y;
    Pixmap tmp ;
    XGCValues gv ;

    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */

    ac_x = x;
    if ( (Pc[wn].attributes & BOTTOM_LEFT_ORIGIN) ) ac_y = -y;
    else ac_y = y;

    /* */
    if ( Pc[wn].tmppix == None ) Pc[wn].tmppix = create_layer(wn);

    gv.tile = Pc[wn].pix[Pc[wn].wly];
    gv.ts_x_origin = ac_x;
    gv.ts_y_origin = ac_y;

    XChangeGC( Pc_dis, Pc[wn].pxgc, 
	       GCTile | GCTileStipXOrigin | GCTileStipYOrigin, &gv ) ;

    XFillRectangle( Pc_dis, Pc[wn].tmppix, Pc[wn].pxgc, 
		    0,0, Pc[wn].wszx, Pc[wn].wszy ) ;

    tmp = Pc[wn].tmppix;
    Pc[wn].tmppix = Pc[wn].pix[Pc[wn].wly];
    Pc[wn].pix[Pc[wn].wly] = tmp;

    if ( Pc[wn].wly == Pc[wn].sly ) {
	if ( 1 /* Pc[wn].attributes & AUTOREDRAW */ ) {
	    send_command_to_child(wn, MCODE_PIXMAP_ID, 
				  Pc[wn].pix[Pc[wn].wly], True);
	}
    }
    gv.ts_x_origin = 0;
    gv.ts_y_origin = 0;
    XChangeGC(Pc_dis, Pc[wn].pxgc, GCTileStipXOrigin | GCTileStipYOrigin, &gv);

    /* */
    if ( clr ) {
	int xx, yy;

	xx = (0 < ac_x) ? 0 : (Pc[wn].wszx + ac_x);
	yy = (0 < ac_y) ? 0 : (Pc[wn].wszy + ac_y);

	XFillRectangle( Pc_dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].bggc, 
			xx,0, abs(ac_x), Pc[wn].wszy ) ;
	XFillRectangle( Pc_dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].bggc, 
			0,yy, Pc[wn].wszx, abs(ac_y) ) ;
    }

    if ( Pc[wn].wly == Pc[wn].sly ) {
	XGCValues gv;
	gv.tile = Pc[wn].pix[Pc[wn].wly];
	XChangeGC( Pc_dis, Pc[wn].pxgc, GCTile, &gv );
	XFillRectangle( Pc_dis, Pc[wn].win, Pc[wn].pxgc, 
			0,0, Pc[wn].wszx,Pc[wn].wszy );
	if ( Pc[wn].iconwin != None ) {
	    XFillRectangle( Pc_dis, Pc[wn].iconwin, Pc[wn].pxgc, 
			    0,0, Pc[wn].wszx,Pc[wn].wszy );
	}
	do_auto_flush();
    }

    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

void gscroll_( integer *wn, integer *x, integer *y, integer *clr )
{
    eggx_gscroll( *wn, *x, *y, *clr ) ;
}

/* ============= 属性まわりを扱う関数 ============= */

void eggx_gsetinitialattributes( int values, int att_msk )
{
    if ( values == ENABLE ) {
	Pc_attributes |= att_msk;
    }
    else if ( values == DISABLE ) {
	Pc_attributes &= ~att_msk;
    }
    else {
	Pc_attributes |= (values & att_msk) ;
	Pc_attributes &= (values | ~att_msk) ;
    }
    /*
    fprintf(stderr,"DEBUG: AUTOREDRAW: %d\n",
	    Pc_attributes & AUTOREDRAW);
    fprintf(stderr,"DEBUG: BOTTOMLEFTORIGIN: %d\n",
	    Pc_attributes & BOTTOM_LEFT_ORIGIN);
    fprintf(stderr,"DEBUG: DOCKAPP: %d\n",
	    Pc_attributes & DOCK_APPLICATION);
    fprintf(stderr,"DEBUG: OVERRIDE: %d\n",
	    Pc_attributes & OVERRIDE_REDIRECT);
    */
    return;
}

int eggx_ggetinitialattributes( void )
{
    return Pc_attributes;
}

void eggx_gsetborder( int wn, int width, const char *argsformat, ... )
{
    int fl = 0;
    unsigned long pix;
    char *color = NULL;
    va_list ap;

    if ( 0 <= width ) {
	XSetWindowBorderWidth( Pc_dis, Pc[wn].pwin, width );
	if ( Pc[wn].iconwin != None ) {
	    XSetWindowBorderWidth( Pc_dis, Pc[wn].iconwin, width );
	}
	fl = 1;
    }

    if ( argsformat != NULL ) {
	va_start(ap, argsformat);
	color = _eggx_vasprintf(argsformat,ap);
	va_end(ap);

	pix = get_color_pixel( color );
	XSetWindowBorder( Pc_dis, Pc[wn].pwin, pix );
	if ( Pc[wn].iconwin != None ) {
	    XSetWindowBorder( Pc_dis, Pc[wn].iconwin, pix );
	}
	fl = 1;
    }
    if ( fl ) do_auto_flush();
    if ( color != NULL ) free(color);
    return;
}

void eggx_gsetinitialborder( int width, const char *argsformat, ... )
{
    va_list ap;
    
    if ( 0 <= width ) Pc_bdr_width = width;
    
    if ( argsformat != NULL ) {
	if ( Pc_bordercolor != NULL ) {
	    free(Pc_bordercolor);
	    Pc_bordercolor = NULL;
	}
	va_start(ap, argsformat);
	Pc_bordercolor = _eggx_vasprintf(argsformat,ap);
	va_end(ap);
    }
    /*
    else{
	if( Pc_bgcolor != NULL ){
	    free(Pc_bordercolor) ;
	    Pc_bordercolor = NULL ;
	}
    }
    */
    return;
}

void eggx_gsetbgcolor( int wn, const char *argsformat, ... )
{
    char *color = NULL;
    va_list ap;

    if ( argsformat != NULL ) {
	va_start(ap, argsformat);
	color = _eggx_vasprintf(argsformat,ap);
	va_end(ap);

	Pc[wn].bgcolor = get_color_pixel(color);
	XSetForeground( Pc_dis, Pc[wn].bggc, Pc[wn].bgcolor );
	XSetWindowBackground( Pc_dis, Pc[wn].win, Pc[wn].bgcolor );
	if ( Pc[wn].iconwin != None ) {
	    XSetWindowBackground( Pc_dis, Pc[wn].iconwin, Pc[wn].bgcolor );
	}
    }
    if ( color != NULL ) free(color);
    return;
}

void gsetbgcolor_( integer *wn, char *colorname )
{
    eggx_gsetbgcolor( *wn, "%s", colorname ) ;
}

void eggx_gsetinitialbgcolor( const char *argsformat, ... )
{
    va_list ap;

    if ( Pc_bgcolor != NULL ) {
	free(Pc_bgcolor);
	Pc_bgcolor = NULL;
    }
    if ( argsformat != NULL ) {
	va_start(ap, argsformat);
	Pc_bgcolor = _eggx_vasprintf(argsformat,ap);
	va_end(ap);
    }

    return;
}

void eggx_gsetinitialgeometry( const char *argsformat, ... )
{
    va_list ap;

    if ( Pc_geometry != NULL ) {
	free(Pc_geometry);
	Pc_geometry = NULL;
    }
    if ( argsformat != NULL ) {
	va_start(ap, argsformat);
	Pc_geometry = _eggx_vasprintf(argsformat,ap);
	va_end(ap);
    }

    return;
}

void eggx_gsetinitialparsegeometry( const char *argsformat, ... )
{
    va_list ap;

    if ( Pc_geometry != NULL ) {
	free(Pc_geometry);
	Pc_geometry = NULL;
    }
    if ( argsformat != NULL ) {
	va_start(ap, argsformat);
	Pc_geometry = _eggx_vasprintf(argsformat,ap);
	va_end(ap);
    }

    return;
}

void eggx_gsetinitialwinname( const char *storename, const char *iconname,
			      const char *resname, const char *classname )
{
    if( Pc_storename!=NULL ){
	free(Pc_storename) ;
	Pc_storename=NULL ;
    }
    if( Pc_iconname!=NULL ){
	free(Pc_iconname) ;
	Pc_iconname=NULL ;
    }
    if( Pc_classhint_res_name!=NULL ){
	free(Pc_classhint_res_name) ;
	Pc_classhint_res_name=NULL ;
    }
    if( Pc_classhint_res_class!=NULL ){
	free(Pc_classhint_res_class) ;
	Pc_classhint_res_class=NULL ;
    }

    if( storename != NULL )
	Pc_storename = _eggx_xstrdup(storename) ;
    if( iconname != NULL )
	Pc_iconname = _eggx_xstrdup(iconname) ;
    if( resname != NULL )
	Pc_classhint_res_name = _eggx_xstrdup(resname) ;
    if( classname != NULL )
	Pc_classhint_res_class = _eggx_xstrdup(classname) ;
}

void gsetinitialwinname_( char *storename, char *iconname,
			  char *resname, char *classname )
{
    eggx_gsetinitialwinname( storename, iconname,
			     resname, classname ) ;
}

int eggx_winname( int wn, const char *argsformat, ... )
{
    char *name = NULL;
    int nn = -1;
    va_list ap;

    if ( argsformat == NULL ) goto quit;

    va_start(ap, argsformat);
    name = _eggx_vasprintf(argsformat,ap);
    va_end(ap);

    nn = strlen(name);

    if ( Pc[wn].winname != NULL ) {
	free(Pc[wn].winname) ;
	Pc[wn].winname = NULL ;
    }
    Pc[wn].winname = _eggx_xstrdup(name) ;	/* register */
    
    XStoreName( Pc_dis, Pc[wn].pwin, name ) ;
    XSetIconName( Pc_dis, Pc[wn].pwin, name ) ;
    if( Pc[wn].iconwin != None ){
	XStoreName( Pc_dis, Pc[wn].iconwin, name ) ;
	XSetIconName( Pc_dis, Pc[wn].iconwin, name ) ;
    }
    do_auto_flush() ;

 quit:
    if ( name != NULL ) free(name);
    return (nn);
}

/* ウィンドゥの名前を決める */
void winname_( integer *wn, char *name )
{
    if ( Pc[*wn].winname != NULL ) {
	free(Pc[*wn].winname) ;
	Pc[*wn].winname = NULL ;
    }
    Pc[*wn].winname = _eggx_xstrdup(name) ;	/* register */

    XStoreName( Pc_dis, Pc[*wn].pwin, name ) ;
    XSetIconName( Pc_dis, Pc[*wn].pwin, name ) ;
    if( Pc[*wn].iconwin != None ){
	XStoreName( Pc_dis, Pc[*wn].iconwin, name ) ;
	XSetIconName( Pc_dis, Pc[*wn].iconwin, name ) ;
    }
    do_auto_flush() ;
}

/* Windowを設定する[0] */
void eggx_window( int wn, double xs, double ys, double xe, double ye )
{
    Pc[wn].acx0=xs ;
    Pc[wn].acy0=ys ;
    if( Pc[wn].wszx <= 1 ) Pc[wn].scalex = 1 ;
    else Pc[wn].scalex = (double)Pc[wn].wszx/ 
	     ((double)(Pc[wn].wszx)*(xe-Pc[wn].acx0)/(double)(Pc[wn].wszx-1)) ;
    if( Pc[wn].wszy <= 1 ) Pc[wn].scaley = 1 ;
    else Pc[wn].scaley = (double)Pc[wn].wszy/
	     ((double)(Pc[wn].wszy)*(ye-Pc[wn].acy0)/(double)(Pc[wn].wszy-1)) ;
    return;
}

void newwindow_( integer *wn, real *xs, real *ys, real *xe, real *ye )
{
    eggx_window( *wn, *xs, *ys, *xe, *ye ) ;
}

/* Windowを設定する(互換routine) */
void window_( real *xs, real *ys, real *xe, real *ye )
{
    eggx_window( Wn,*xs,*ys,*xe,*ye ) ;
}

void eggx_coordinate( int wn, int xref, int yref,
		      double xs, double ys, double xscale, double yscale )
{
    Pc[wn].acx0 = (xs * xscale - xref) / xscale;
    Pc[wn].acy0 = (ys * yscale - yref) / yscale;
    Pc[wn].scalex = xscale;
    Pc[wn].scaley = yscale;

    return;
}

void newcoordinate_( integer *wn, integer *xref, integer *yref, 
		     real *xs, real *ys, real *xscale, real *yscale )
{
    eggx_coordinate(*wn, *xref, *yref, *xs, *ys, *xscale, *yscale);
}

void vport_( real *x0, real *y0, real *x1, real *y1 )
{
}

void setal_( integer *x, integer *y )
{
}

/* 端末のクリア */
void eggx_tclr( void )
{
    printf("\033[H\033[2J") ;
    fflush(stdout) ;
}

void tclr_()
{
    printf("\033[H\033[2J") ;
    fflush(stdout) ;
}

void clsc_()
{
    printf("\033[H\033[2J") ;
    fflush(stdout) ;
}

/* 窓を選択する */
void selwin_( integer *wn )
{
    if ( 0 <= *wn ) Wn = *wn ;
}

/* RGBで色を指定する */
void eggx_newrgbcolor( int wn, int r, int g, int b )
{
    XColor exact_color;
    Colormap cmap;
    unsigned long pxl;

    cmap = Pc_cmap /* DefaultColormap( dis, 0 ) */ ;

    if ( r < 0 ) r = 0;
    if ( g < 0 ) g = 0;
    if ( b < 0 ) b = 0;
    if ( r > 255 ) r = 255;
    if ( g > 255 ) g = 255;
    if ( b > 255 ) b = 255;

    wait_child();
    Ihflg=1;
    if ( Pc_depth == 8 ) {	/* 256色時カラーマップに登録 */
	exact_color.green = g << 8;
	exact_color.red   = r << 8;
	exact_color.blue  = b << 8;
	get_colorpixel_from_cmap(cmap, &exact_color);
	pxl = exact_color.pixel;
    }
    else{
	pxl  = (r>>(8-Red_depth))<<Red_sft;
	pxl |= (g>>(8-Green_depth))<<Green_sft;
	pxl |= (b>>(8-Blue_depth))<<Blue_sft;
    }
    XSetForeground( Pc_dis, Pc[wn].gc, pxl );
    Ihflg=0;
    chkexit();
}

void newrgbcolor_( integer *wn, integer *pr, integer *pg, integer *pb )
{
    eggx_newrgbcolor( *wn, *pr, *pg, *pb ) ;
}

/* HSVで色を指定する */
/* 安田様@京都産業大学からいだたきました．感謝 :-) */
void eggx_newhsvcolor( int win, int h, int s, int v )
{
    int c1, c2, c3, r, g, b, t;

    if ( s == 0 ) {
	r = v;
	g = v;
	b = v;
    } else {
	t  = (h*6) % 360;
	c1 = (v*(255-s))/255;
	c2 = (v*(255-(s*t)/360))/255;
	c3 = (v*(255-(s*(360-t))/360))/255;
	switch (h/60) {
	case 0: 
	    r=v; g=c3; b=c1;
	    break;
	case 1: 
	    r=c2; g=v; b=c1;
	    break;
	case 2: 
	    r=c1; g=v; b=c3;
	    break;
	case 3: 
	    r=c1; g=c2; b=v;
	    break;
	case 4:
	    r=c3; g=c1; b=v;
	    break;
	default: /* should be 5 */
	    r=v; g=c1; b=c2;
	    break;
	}
    }
    eggx_newrgbcolor(win, r, g, b);
    return;
}

void newhsvcolor_( integer *wn, integer *ph, integer *ps, integer *pv )
{
    eggx_newhsvcolor( *wn, *ph, *ps, *pv ) ;
}

/* 直接色を指定する[1] */
void eggx_newcolor( int wn, const char *argsformat, ... )
{
    char *color = NULL;
    va_list ap;

    if( argsformat != NULL ){
	va_start(ap, argsformat);
	color = _eggx_vasprintf(argsformat,ap);
	va_end(ap);

	wait_child() ;	/* imgsave中は待たせる */
	Ihflg=1 ;	/* 割り込み禁止 */
	XSetForeground( Pc_dis, Pc[wn].gc, get_color_pixel( color ) ) ;
	Ihflg=0 ;	/* 割り込み禁止解除 */
    }

    if ( color != NULL ) free(color);
    chkexit();
    return;
}

void newcolor_( integer *wn, char *color )
{
    eggx_newcolor( *wn, "%s", color ) ;
}

/* 色を指定する[2] */
void eggx_newpen( int wn, int n )
{
    /* { "Black","White","Red","Green",
         "Blue","Cyan","Magenta","Yellow",
         "DimGray","Grey","red4","green4",
         "blue4","cyan4","magenta4","yellow4" } */
    const int r[16] = { 0, 255, 255,   0,   0,   0, 255, 255, 105, 190, 139,   0,   0,   0, 139, 139 };
    const int g[16] = { 0, 255,   0, 255,   0, 255,   0, 255, 105, 190,   0, 139,   0, 139,   0, 139 };
    const int b[16] = { 0, 255,   0,   0, 255, 255, 255,   0, 105, 190,   0,   0, 139, 139, 139,   0 };
    if ( 0 <= n ) {
	n = n % 16;
	eggx_newrgbcolor(wn, r[n], g[n], b[n]);
    }
    return;
}

void newpencolor_( integer *wn, integer *n )
{
    eggx_newpen( *wn, *n ) ;
}

/* 色を指定する(互換routine) */
void newpen_( integer *n )
{
    eggx_newpen( Wn, *n ) ;
}

/* ライン幅を指定する */
void eggx_newlinewidth( int wn, int width )
{
    XGCValues gv ;
    gv.line_width = abs(width) ;
    Pc[wn].linewidth = gv.line_width ;

    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    XChangeGC( Pc_dis, Pc[wn].gc, GCLineWidth, &gv ) ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit();

    return;
}

void newlinewidth_( integer *wn, integer *width )
{
    eggx_newlinewidth( *wn, *width ) ;
}

/* ラインスタイルを指定する */
void eggx_newlinestyle( int wn, int style )
{
    XGCValues gv ;
    gv.line_style = style ;
    Pc[wn].linestyle = gv.line_style ;

    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    XChangeGC( Pc_dis, Pc[wn].gc, GCLineStyle, &gv ) ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit();

    return;
}

void newlinestyle_( integer *wn, integer *style )
{
    eggx_newlinestyle( *wn, *style ) ;
}

/* GC function を指定する */
/* drawarrow() とか drawsym() との併用の結果については未定義  */
void eggx_newgcfunction( int wn, int fnc )
{
    XGCValues gv ;
    gv.function = fnc ;
    Pc[wn].gcfunc = gv.function ;

    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    XChangeGC( Pc_dis, Pc[wn].gc, GCFunction, &gv ) ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit();

    return;
}

/* 消去する */
void eggx_gclr( int wn )
{
    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */

    XFillRectangle( Pc_dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].bggc, 
		   0, 0, Pc[wn].wszx, Pc[wn].wszy ) ;
    //XDrawRectangle( Pc_dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].bggc, 
    //		      0, 0, Pc[wn].wszx, Pc[wn].wszy );
    if( Pc[wn].wly==Pc[wn].sly ){
	XClearWindow( Pc_dis, Pc[wn].win ) ;
	if( Pc[wn].iconwin != None ){
	    XClearWindow( Pc_dis, Pc[wn].iconwin ) ;
	}
	do_auto_flush() ;
    }
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

void gclr_( integer *wn )
{
    eggx_gclr( *wn ) ;
}

/* 消去する(互換routine) */
void clsx_( void )
{
    eggx_gclr( Wn ) ;
}

/* ============= このあたりから描画関数 ============= */

/* 線を引く[3] */
void eggx_line( int wn, double xg, double yg, int mode )
{
    int xxg,yyg,f ;
    xyconv(wn,xg,yg,&xxg,&yyg) ;
    
    switch( mode ){
    case PENDOWN:
	wait_child() ;	/* imgsave中は待たせる */
	Ihflg=1 ;	/* 割り込み禁止 */
	f=XX_DrawLine( Pc_dis, wn, Pc[wn].prevx, Pc[wn].prevy, xxg, yyg ) ;
	Pc[wn].prevx=xxg ;
	Pc[wn].prevy=yyg ;
	if(f) do_auto_flush() ;
	Ihflg=0 ;	/* 割り込み禁止解除 */
	chkexit() ;
	break ;
    case PENUP:
	Pc[wn].prevx=xxg ;
	Pc[wn].prevy=yyg ;
	break ;
    case PSET:
	Pc[wn].prevx=xxg ;	/* 0.74 */
	Pc[wn].prevy=yyg ;	/* 0.74 */
	wait_child() ;	/* imgsave中は待たせる */
	Ihflg=1 ;	/* 割り込み禁止 */
	f=X_DrawPoint( Pc_dis, wn, xxg, yyg ) ;
	if(f) do_auto_flush() ;
	Ihflg=0 ;	/* 割り込み禁止解除 */
	chkexit() ;
	break ;
    }
}

void line_( integer *wn, real *xg, real *yg, integer *mode )
{
    eggx_line( *wn, *xg, *yg, *mode ) ;
}

void eggx_moveto( int wn, double xg, double yg )
{
    eggx_line(wn,xg,yg,PENUP);
}

void moveto_( integer *wn, real *xg, real *yg )
{
    eggx_line( *wn, *xg, *yg, PENUP ) ;
}

void eggx_lineto( int wn, double xg, double yg )
{
    eggx_line(wn,xg,yg,PENDOWN);
}

void lineto_( integer *wn, real *xg, real *yg )
{
    eggx_line( *wn, *xg, *yg, PENDOWN ) ;
}

void eggx_drawline( int wn, double xg0, double yg0, double xg1, double yg1 )
{
    int xx0,yy0,xx1,yy1,f=0 ;

    xyconv( wn,xg0,yg0,&xx0,&yy0 ) ;
    xyconv( wn,xg1,yg1,&xx1,&yy1 ) ;

    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    f |= XX_DrawLine( Pc_dis, wn, xx0, yy0, xx1, yy1 ) ;
    if ( f ) do_auto_flush() ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;

    return;
}

void drawline_( integer *wn, real *xg0, real *yg0, real *xg1, real *yg1 )
{
    eggx_drawline( *wn, *xg0, *yg0, *xg1, *yg1 ) ;
}

void eggx_pset( int wn, double xg, double yg )
{
    int xxg,yyg,f ;
    xyconv(wn,xg,yg,&xxg,&yyg) ;
    
    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    f=X_DrawPoint( Pc_dis, wn, xxg, yyg ) ;
    if(f) do_auto_flush() ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

void pset_( integer *wn, real *xg, real *yg )
{
    eggx_pset( *wn, *xg, *yg ) ;
}

/* 線を引く(互換routine) */
void plot_( real *xg, real *yg, integer *mode )
{
    eggx_line( Wn, *xg, *yg, *mode ) ;
}

/*
typedef struct _MyFont
{
  char *name;
  XFontStruct *font;
  XFontSet fontset;
  int height;
  int y;
} MyFont;
*/

/* フォントセットを生成する */
static int setfontsetv( int wn, const char *argsformat, va_list ap )
{
    XFontSetExtents *fset_extents;
    int rtv = 0;
    char *fontname_buf = NULL;
    const char *fontname = DEFAULTFONTSET;

    if ( argsformat != NULL ) {
	fontname_buf = _eggx_vasprintf(argsformat,ap);
	fontname = fontname_buf;
    }
    
    Pc[wn].fontset = get_fontset(fontname);
    if ( Pc[wn].fontset == NULL ) {
	rtv = 1;
	if ( (Pc[wn].fontset = get_fontset(DEFAULTFONTSET)) == NULL ) {
	    if ( (Pc[wn].fontset = get_fontset(FALLBACKFONTSET)) == NULL ) {
		rtv = -1;
	    }
	}
    }
    /* XFontsOfFontSet(Pc[wn].fontset, &fs_list, &sl); */
    /* Pc[wn].font = fs_list[0]; */
    if ( rtv != -1 ) {
	fset_extents = XExtentsOfFontSet(Pc[wn].fontset);
	Pc[wn].fontheight = fset_extents->max_logical_extent.height;
    }
    /* Pc[wn].fonty = Scr.StdFont.font->ascent; */
    /* Pc[wn].entryheight = Pc[wn].fontheight + HEIGHT_EXTRA +2; */

    if ( fontname_buf != NULL ) free(fontname_buf);
    return (rtv);
}

int eggx_newfontset( int wn, const char *argsformat, ... )
{
    int rtv;
    va_list ap;
    va_start(ap, argsformat);
    rtv = setfontsetv(wn, argsformat, ap);
    va_end(ap);
    return rtv;
}

int eggx_gsetfontset( int wn, const char *argsformat, ... )
{
    int rtv;
    va_list ap;
    va_start(ap, argsformat);
    rtv = setfontsetv(wn, argsformat, ap);
    va_end(ap);
    return rtv;
}

void newfontset_( integer *wn, char *fset, integer *status )
{
    *status = (integer)eggx_newfontset( *wn, "%s", fset ) ;
}

void gsetfontset_( integer *wn, char *fset, integer *status )
{
    *status = (integer)eggx_newfontset( *wn, "%s", fset ) ;
}

/* 文字を描く[4] */
void drawstr_( integer *wn, real *xg, real *yg, real *size,
	       char *str, real *theta, integer *in_len )
{
    static int msg_flag0 = 0;
    int fsize ;
    int xxg,yyg ;
    unsigned short font_height ;
    int i,f=0,lf=0,dlen,line=0,len=0 ;
    char *ptr,*ptr1,*nptr,*last0_ptr ;
    char *buf=NULL ;

    xyconv(*wn,*xg,*yg,&xxg,&yyg) ;
    if( 1.0 <= *size && *size <= 24.0 ) fsize=(int)(*size) ;
    else{
	if( *size < 1.0 ) fsize=FONTSET ;
	else fsize=24 ;
    }

    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */

    if( Pc[*wn].fsz!=fsize ){
	if( FONTSET < fsize ){
	    if( Pc[*wn].fontstruct != NULL ){
		XFreeFont( Pc_dis, Pc[*wn].fontstruct ) ;
		Pc[*wn].fontstruct = NULL ;
	    }
	    Pc[*wn].fontstruct = XLoadQueryFont( Pc_dis, Pc_fnt[fsize-1] ) ;
	    if ( Pc[*wn].fontstruct == NULL ) {
		char *xf_buf = NULL;
		const char *ptr_x = strchr(Pc_fnt[fsize-1], 'x');
		if ( ptr_x != NULL ) {
		    xf_buf = _eggx_asprintf(Pc_fallbackfnt_fmt, ptr_x + 1);
		    Pc[*wn].fontstruct = XLoadQueryFont( Pc_dis, xf_buf );
		}
		else {
		    fprintf(stderr,"EGGX: [ERROR] Internal error!!\n");
		}
		if ( xf_buf != NULL ) free(xf_buf);
	    }
	    /* フォントが見つからない場合 */
	    if ( Pc[*wn].fontstruct == NULL ) {
		if ( msg_flag0 < 1 ) {
		    fprintf(stderr,"EGGX: [WARNING] "
			    "Basic fonts for EGGX are not found. "
			    "Check your font setting of X server.\n");
		    msg_flag0++;
		}
		Pc[*wn].fontstruct = XLoadQueryFont( Pc_dis, Pc_fallbackfnt ) ;
		if ( Pc[*wn].fontstruct == NULL ) {
		    fprintf(stderr,"EGGX: [ERROR] "
			    "Default font 'fixed' is not found. "
			    "Check your font setting of X server.\n");
		    goto quit;
		}
	    }
	    XSetFont( Pc_dis, Pc[*wn].gc, Pc[*wn].fontstruct->fid ) ;
	}
	else{
	    if ( Pc[*wn].fontset == NULL ) {
		if ( eggx_newfontset(*wn,DEFAULTFONTSET) < 0 ) {
		    fprintf(stderr,
			    "EGGX: [WARNING] Cannot set default fontset.\n") ;
		}
	    }
#if 0	/* ver. 0.90: これはいらないらしい */
	    XFontStruct **fs_list;
	    char **sl;
	    XGCValues gv;
	    XFontsOfFontSet(Pc[*wn].fontset, &fs_list, &sl);
	    gv.font = fs_list[0]->fid;
	    XChangeGC( Pc_dis, Pc[*wn].gc, GCFont, &gv ) ;
#endif
	}
	Pc[*wn].fsz = fsize ;
    }

    if( FONTSET < fsize )
	font_height = Pc[*wn].fontstruct->ascent + Pc[*wn].fontstruct->descent ;
    else
	font_height = Pc[*wn].fontheight ;

    if( 0 <= (*in_len) ){
	len = (*in_len) ;
	buf=(char *)_eggx_xmalloc(sizeof(char)*(len+1)) ;
    }
    else{
	len = strlen(str) ;
	buf=(char *)_eggx_xmalloc(sizeof(char)*(len+1)) ;
    }

    for( i=0 ; i<len ; i++ ) buf[i]=str[i] ;
    /* strncpy(buf,str,len) ; */
    buf[len]='\0' ;
    nptr = buf ;
    last0_ptr = buf + len ;
    do {
	ptr = strchr(nptr,'\n') ;
	ptr1 = strchr(nptr,'\r') ;
	if( ptr != NULL || ptr1 != NULL ){
	    if( ptr != NULL && ptr1 != NULL ){
		if( ptr1-ptr < 0 ){
		    ptr=ptr1 ;
		    lf=1 ;
		}
		else lf=2 ;
	    }
	    else{
		if( ptr1 != NULL ){
		    ptr=ptr1 ;
		    lf=1 ;
		}
		else lf=2 ;
	    }
	    dlen = ptr - nptr ;
	}
	else{
	    dlen = last0_ptr - nptr ;
	    lf=0 ;
	}
	if( 0 < dlen ){
	    if( FONTSET < fsize ) {
		f=X_DrawString( Pc_dis, *wn, xxg, yyg + font_height*line, nptr, dlen ) ;
	    }
	    else {
		f=X_mbDrawString( Pc_dis, *wn, xxg, yyg + font_height*line, nptr, dlen ) ;
	    }
	}
	if( lf==2 ){
	    nptr = ptr + 1 ;
	    line++ ;
	}
	else if( lf==1 ){
	    nptr = ptr + 1 ;
	}
    } while( lf ) ;
    if(f) do_auto_flush() ;

 quit:
    if( buf != NULL ) free(buf) ;

    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

int eggx_drawstr( int wn, double x, double y, int size, double theta,
		  const char *argsformat, ... )
{
    va_list ap;
    char *str = NULL;
    integer w, l = -1;
    real sz,xx,yy,ttheta;

    if ( argsformat == NULL ) goto quit;

    va_start(ap, argsformat);
    str = _eggx_vasprintf(argsformat,ap);
    va_end(ap);

    w=(integer)wn ;
    xx=(real)x ;
    yy=(real)y ;
    sz=(real)size ;
    ttheta=(real)theta ;
    l=strlen(str) ;
    drawstr_(&w,&xx,&yy,&sz,str,&ttheta,&l) ;

 quit:
    if ( str != NULL ) free(str);
    return (l);
}

static int draw_a_symbol( int wn, double xg, double yg, int size, int sym )
{
    int x0,y0,x1,y1 ;
    int xx0=0,yy0=0,xx1=0,yy1=0 ;
    int hs,f=0 ;
    int xxg,yyg ;
    int pst ;
    double fhs;
    const double factors[10] = { 0.52,				/* 10 */
				0.125, 0.5, 0.55, 0.44, 0.42,	/* 1-5 */
				0.62, 0.68, 0.43, 0.42 };	/* 6-9 */
    XPoint xpts[8];

    if ( sym <= 0 ) return f;	/* 0 以下は却下 */
    else sym = sym % 10;	/* 10 番は 0 番になる */
    
    xyconv(wn,xg,yg,&xxg,&yyg) ;
#if 0	/* old: Version 0.85 まで (Pro-FORTRAN に見栄えを合わせた) */
    switch( sym ){
    case 1:
	hs=(int)(size/8.0) ;
	break ;
    default:
	if( sym==2 || sym==8 || sym==9 || sym==0 ){
	    hs = (int)(size/4.0) ;
	}
	else hs = (int)(size/2.0)-1 ;
	if( hs<0 ) hs=0 ;
	break ;
    }
    y0 = yyg -hs ;
    if( sym==7 ) hs = (int)(size/4.0) ;
    x0 = xxg -hs ;
    x1 = xxg +hs ;
    y1 = yyg +hs ;
    if( sym==3 || sym==6 ){
	hs = (int)((double)hs*0.7) ;
	xx0 = xxg -hs ;
	yy0 = yyg -hs ;
	xx1 = xxg +hs ;
	yy1 = yyg +hs ;
    }
#endif
    /* Version 0.90 で各シンボルの大きさのバランスを整えた */
    fhs = size * factors[sym] ;
    hs = _eggx_iround(fhs) ;
    if ( hs < 0 ) hs = 0 ;

    y0 = yyg - hs ;
    if ( sym == 7 ) hs = _eggx_iround(size * 0.4) ;
    x0 = xxg - hs ;
    x1 = xxg + hs ;
    y1 = yyg + hs ;
    if ( sym == 3 || sym == 6 ) {
	hs = _eggx_iround(fhs * 0.7) ;
	xx0 = xxg - hs ;
	yy0 = yyg - hs ;
	xx1 = xxg + hs ;
	yy1 = yyg + hs ;
    }

    switch ( sym ) {
    case 1:
	xpts[0].x = xxg;  xpts[0].y = y0;
	xpts[1].x = x1;   xpts[1].y = yyg;
	xpts[2].x = xxg;  xpts[2].y = y1;
	xpts[3].x = x0;   xpts[3].y = yyg;
	xpts[4].x = xxg;  xpts[4].y = y0;
	f = X_DrawLines( Pc_dis, wn, xpts, 5 );
	break;
    case 2:
	XX_DrawLine( Pc_dis, wn, xxg, y0, xxg, y1 );
	f = XX_DrawLine( Pc_dis, wn, x0, yyg, x1, yyg );
	break;
    case 3:
	XX_DrawLine( Pc_dis, wn, xxg, y0, xxg, y1 );
	XX_DrawLine( Pc_dis, wn, x0, yyg, x1, yyg );
	XX_DrawLine( Pc_dis, wn, xx0, yy0, xx1, yy1 );
	f = XX_DrawLine( Pc_dis, wn, xx1, yy0, xx0, yy1 );
	break;
    case 4:
	f = X_DrawArc( Pc_dis, wn, x0, y0, x1-x0, y1-y0, 0, 64*360 );
	break;
    case 5:
	XX_DrawLine( Pc_dis, wn, x0, y0, x1, y1 );
	f = XX_DrawLine( Pc_dis, wn, x1, y0, x0, y1 );
	break;
    case 6:
	pst = needs_pset(wn);
	if ( pst ) X_DrawPoint( Pc_dis, wn, xx0, yy0 );
	X_DrawLine( Pc_dis, wn, xx0, yy0, xxg, yyg );
	if ( pst ) X_DrawPoint( Pc_dis, wn, xx1, yy0 );
	X_DrawLine( Pc_dis, wn, xx1, yy0, xxg, yyg );
	if ( pst ) X_DrawPoint( Pc_dis, wn, xxg, yyg );
	f = X_DrawLine( Pc_dis, wn, xxg, y1, xxg, yyg );
	break;
    case 7:
	xpts[0].x = xxg;  xpts[0].y = yyg;
	xpts[1].x = xxg;  xpts[1].y = y0;
	xpts[2].x = x0;   xpts[2].y = y1;
	xpts[3].x = x1;   xpts[3].y = y1;
	xpts[4].x = xxg;  xpts[4].y = y0;
	f = X_DrawLines( Pc_dis, wn, xpts, 5 );
	break;
    case 8:
	xpts[0].x = xxg;  xpts[0].y = yyg;
	xpts[1].x = xxg;  xpts[1].y = y0;
	xpts[2].x = x1;   xpts[2].y = y0;
	xpts[3].x = x1;   xpts[3].y = y1;
	xpts[4].x = x0;   xpts[4].y = y1;
	xpts[5].x = x0;   xpts[5].y = y0;
	xpts[6].x = xxg;  xpts[6].y = y0;
	f = X_DrawLines( Pc_dis, wn, xpts, 7 );
	break;
    case 9:
	XX_DrawLine( Pc_dis, wn, x0, y0, x1, y1 );
	XX_DrawLine( Pc_dis, wn, x1, y0, x0, y1 );
	f = X_DrawLine( Pc_dis, wn, x0, y0, x1, y0 );
	break;
    case 0:
	xpts[0].x = xxg;  xpts[0].y = yyg;
	xpts[1].x = xxg;  xpts[1].y = y0;
	xpts[2].x = x1;   xpts[2].y = yyg;
	xpts[3].x = xxg;  xpts[3].y = y1;
	xpts[4].x = x0;   xpts[4].y = yyg;
	xpts[5].x = xxg;  xpts[5].y = y0;
	f = X_DrawLines( Pc_dis, wn, xpts, 6 );
	break;
    }
    return (f);
}

/* センターシンボルを描く[5] */
void eggx_drawsym( int wn, double xg, double yg, int size, int sym )
{
    int f;
    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    f = draw_a_symbol(wn,xg,yg,size,sym);
    if (f) do_auto_flush() ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

#if 0
void eggx_symbol( int wn, double xg, double yg, int size, int sym )
{
    eggx_drawsym(wn,xg,yg,size,sym);
}
#endif

void drawsym_( integer *wn, real *xg, real *yg, real *size, integer *sym )
{
    eggx_drawsym( *wn, *xg, *yg, *size, *sym ) ;
}

/* センターシンボルを描く(互換routine) */
void symbol_( real *xg, real *yg, real *size, 
	      void *k, real *theta, integer *len )
{
    integer *sym ;
    char *str ;
    if( *len < 0 ){
	sym = (integer *)k ;
	drawsym_( &(Wn),xg,yg,size,sym ) ;
    }
    else {
	str = (char *)k ;
	drawstr_( &(Wn),xg,yg,size,str,theta,len ) ;
    }
}

/* 複数のシンボルを描く */
void eggx_drawsyms( int wn, const double x[], const double y[], int n,
		    int size, int sym )
{
    int xx, yy, i, f=0;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*n);
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn, x[i], y[i], &xx, &yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    for ( i=0 ; i < n ; i++ ) {
	f |= draw_a_symbol( wn, points[i].x, points[i].y, size, sym );
    }
    if ( f ) do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void eggx_drawsymsf( int wn, const float x[], const float y[], int n,
		     int size, int sym )
{
    int xx, yy, i, f=0;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*n);
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn, x[i], y[i], &xx, &yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    for ( i=0 ; i < n ; i++ ) {
	f |= draw_a_symbol( wn, points[i].x, points[i].y, size, sym );
    }
    if ( f ) do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void drawsyms_( integer *wn, real x[], real y[], integer *n,
		real *size, integer *sym )
{
    int i ;
    float *xx ;
    float *yy ;
    if ( *n < 1 ) return;
    xx=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    yy=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    for( i=0 ; i < *n ; i++ ) xx[i] = (float)(x[i]) ;
    for( i=0 ; i < *n ; i++ ) yy[i] = (float)(y[i]) ;
    eggx_drawsymsf( *wn, xx, yy, *n, *size, *sym ) ;
    free(yy) ;
    free(xx) ;
}

static int podr( integer n )
{
    int rt=1 ;
    if( n==0 ) return(0) ;
    while( (n/=10)!=0 ) rt++ ;
    return(rt) ;
}
/* 変数を文字列に変換する */
/* 引き数: 変数，小数点数 */
static char *rtoc( real *v, integer *n )
{
    static int mlc1 = 0;
    static char *adr = NULL;
    static char *fmt = NULL;
    int f;
    float vv;

    if ( -1 <= *n ) vv = *v;
    else vv = *v * pow(10, 1 + *n);
    if ( fmt == NULL ) {
	mlc1 = RTOC_UNITLEN;
	fmt = (char *)_eggx_xmalloc(sizeof(char)*mlc1);
    }
    while ( 1 ) {
	if ( 0 < *n ) {
	    if ( podr(*n)+4 <= mlc1 ) {
		sprintf(fmt,"%%.%df",(int)(*n));
		f = 0;
	    }
	    else f = -1;
	}
	else {
	    if ( *n == 0 ) sprintf(fmt,"%%.0f.");
	    else sprintf(fmt,"%%.0f");
      	    break;
	}
	if ( f != -1 ) break;
	mlc1 += RTOC_UNITLEN;
	fmt = (char *)_eggx_xrealloc(fmt,sizeof(char)*mlc1);
    }
    if ( adr != NULL ) {
	free(adr);
	adr = NULL;
    }
    adr = _eggx_asprintf(fmt,vv);
    return ( adr );
}

/* 変数を文字列に変換(ProFORTRAN式) */
/* 変数,小数点数,文字数,格納先,あまり文字数 */
void rtoc_( real *v, integer *n, integer *dn, char *dist, integer *m )
{
    char *adr ;
    if ( *dn < 1 ) return;
    adr=rtoc( v, n ) ;
    strncpy( dist, adr, *dn-1 ) ;
    dist[*dn-1]='\0' ;
    *m=*dn-1-strlen(adr) ;
    return;
}

/* 変数の値を描く[6] */
void drawnum_( integer *wn, real *xg, real *yg, real *size, 
	       real *v, real *theta, integer *n )
{
    char *adr ;
    integer len ;
    adr = rtoc( v, n ) ;
    len = strlen(adr) ;
    drawstr_( wn, xg, yg, size, adr, theta, &len ) ;
}

/* 変数の値を描く(互換routine) */
void number_( real *xg, real *yg, real *size, 
	      real *v, real *theta, integer *n )
{
    drawnum_( &(Wn), xg,yg,size,v,theta,n ) ;
}

static int dwtop( int wn, int si, int sj,
		  int xx[], int yy[], 
		  int x1[], int y1[],
		  int x2[], int y2[],
		  int x3[], int y3[] )
{
    int i=1 ;
    int f=0 ;
    XPoint points[3] ;
    do{
	switch(sj){
	case 1:
	    f |= XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x1[i],y1[i] ) ;
	    XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x2[i],y2[i] ) ;
	    X_DrawLine( Pc_dis, wn, x1[i],y1[i],x2[i],y2[i] ) ;
	    break ;
	case 2:
	    f |= XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x1[i],y1[i] ) ;
	    XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x2[i],y2[i] ) ;
	    X_DrawLine( Pc_dis, wn, x1[i],y1[i],x2[i],y2[i] ) ;
	    if( si==0 ) X_DrawLine( Pc_dis, wn, xx[i],yy[i],x3[i],y3[i] ) ;
	    break ;
	case 3:
	    f |= XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x1[i],y1[i] ) ;
	    XX_DrawLine( Pc_dis, wn, x1[i],y1[i],x3[i],y3[i] ) ;
	    if( si==0 ) X_DrawLine( Pc_dis, wn, xx[i],yy[i],x3[i],y3[i] ) ;
	    break ;
	case 4:
	    f |= XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x1[i],y1[i] ) ;
	    XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x2[i],y2[i] ) ;
	    if( si==0 ) XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x3[i],y3[i] ) ;
	    break ;
	case 5:
	    f |= XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x1[i],y1[i] ) ;
	    if( si==0 ) XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x3[i],y3[i] ) ;
	    break ;
	case 6:
	    f |= XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x1[i],y1[i] ) ;
	    XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x2[i],y2[i] ) ;
	    X_DrawLine( Pc_dis, wn, x1[i],y1[i],x2[i],y2[i] ) ;
	    points[0].x=xx[i] ;
	    points[0].y=yy[i] ;
	    points[1].x=x1[i] ;
	    points[1].y=y1[i] ;
	    points[2].x=x2[i] ;
	    points[2].y=y2[i] ;
	    f |= X_FillPolygon( Pc_dis,wn,points,3,Convex,CoordModeOrigin) ;
	    break ;
	case 7:
	    f |= XX_DrawLine( Pc_dis, wn, xx[i],yy[i],x1[i],y1[i] ) ;
	    XX_DrawLine( Pc_dis, wn, x1[i],y1[i],x3[i],y3[i] ) ;
	    if( si==0 ) X_DrawLine( Pc_dis, wn, xx[i],yy[i],x3[i],y3[i] ) ;
	    points[0].x=xx[i] ;
	    points[0].y=yy[i] ;
	    points[1].x=x1[i] ;
	    points[1].y=y1[i] ;
	    points[2].x=x3[i] ;
	    points[2].y=y3[i] ;
	    f |= X_FillPolygon( Pc_dis,wn,points,3,Convex,CoordModeOrigin) ;
	    break ;
	}
    } while( --i > 1-si ) ;
    return(f) ;
}

/* 矢印を描く[7] */
void eggx_drawarrow( int wn, double xs, double ys, double xt, double yt, 
		     double s, double w, int shape )
{
    int pst = needs_pset(wn) ;
    int xx[2],yy[2] ;
    int si,sj ;		/* shape */
    double r ;
    double ss,ww ;
    double theta,phi ;
    double xl,yl ;
    double frx1,frx2,fry1,fry2 ;
    int x1[2],x2[2],y1[2],y2[2],x3[2],y3[2] ;
    int rx1,rx2,ry1,ry2,rx3,ry3 ;
    int f=0 ;

    xyconv( wn,xs,ys,xx,yy ) ;
    xyconv( wn,xt,yt,xx+1,yy+1 ) ;
    xl = xx[0]-xx[1] ;
    yl = yy[0]-yy[1] ;
    if( shape<100 ){
	ss = s ;
	ww = w * 0.5 ;
	si = shape/10 ;
    }
    else {
	ww = sqrt(xl*xl+yl*yl) ;
	ss = ww * s ;
	ww *= w ;
	ww /= 2.0 ;
	si = shape/10-10 ;
    }
	sj = shape % 10 ;
    r  = sqrt( ss*ss+ww*ww ) ;
    theta = atan2(yl,xl) ;
    phi = atan2(ww,ss) ;
    frx1 = r*cos(theta-phi);
    fry1 = r*sin(theta-phi);
    frx2 = r*cos(theta+phi);
    fry2 = r*sin(theta+phi);
    rx1 = _eggx_iround(frx1);
    ry1 = _eggx_iround(fry1);
    rx2 = _eggx_iround(frx2);
    ry2 = _eggx_iround(fry2);
    x1[1] = xx[1] + rx1 ;
    y1[1] = yy[1] + ry1 ;
    x2[1] = xx[1] + rx2 ;
    y2[1] = yy[1] + ry2 ;
    x3[1] = xx[1] + (rx3=_eggx_iround((frx1+frx2)*0.5)) ;
    y3[1] = yy[1] + (ry3=_eggx_iround((fry1+fry2)*0.5)) ;
    if( si==2 ){
	x1[0] = xx[0] - rx1 ;
	y1[0] = yy[0] - ry1 ;
	x2[0] = xx[0] - rx2 ;
	y2[0] = yy[0] - ry2 ;
	x3[0] = xx[0] - rx3 ;
	y3[0] = yy[0] - ry3 ;
    }
    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    f |= dwtop( wn, si, sj, xx,yy,x1,y1,x2,y2,x3,y3 ) ;
    switch( si ){
    case 1:
	if( 2<=sj ){
	    f |= X_DrawLine( Pc_dis, wn, xx[0],yy[0],xx[1],yy[1] ) ;
	}
	else f |= X_DrawLine( Pc_dis, wn, xx[0],yy[0],x3[1],y3[1] ) ;
	if ( pst ) f |= X_DrawPoint( Pc_dis, wn, xx[0], yy[0] ) ;
	break ;
    case 2:
	if( 2<=sj ){
	    f |= X_DrawLine( Pc_dis, wn, xx[0],yy[0],xx[1],yy[1] ) ;
	}
	else f |= X_DrawLine( Pc_dis, wn, x3[0],y3[0],x3[1],y3[1] ) ;
	break ;
    }
    if(f) do_auto_flush() ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

void drawarrow_( integer *wn, real *xs, real *ys, real *xt, real *yt,
		 real *s, real *w, integer *shape )
{
    eggx_drawarrow( *wn, *xs, *ys, *xt, *yt, 
		    *s, *w, *shape ) ;
}

/* 矢印を描く(互換routine) */
void arohd_( real *xs, real *ys, real *xt, real *yt,
	     real *s, real *w, integer *shape )
{
    eggx_drawarrow( Wn ,*xs,*ys,*xt,*yt,*s,*w,*shape ) ;
}

/* 円弧を塗り潰す */
void eggx_fillarc( int wn, double xcen, double ycen, double xrad, double yrad,
		   double sang, double eang, int idir )
{
    int sa,da ;
    int x0,y0,x1,y1,xsize,ysize ;

    xyconv(wn,xcen-xrad,ycen-yrad,&x0,&y0) ;
    xyconv(wn,xcen+xrad,ycen+yrad,&x1,&y1) ;
    xsize=abs(x1-x0) ;
    ysize=abs(y1-y0) ;
    if( x1<x0 ) x0=x1 ;
    if( y1<y0 ) y0=y1 ;
    
    if ( idir == -1 ) {
	sa = _eggx_iround(sang * 64.0) ;
	if ( eang < sang ) da = -sa + _eggx_iround(eang * 64.0) ;
	else da = -sa - _eggx_iround((360.0 - eang) * 64.0) ;
    }
    else {
	sa = _eggx_iround(sang * 64.0) ;
	da = _eggx_iround(eang * 64.0) - sa ;
    }
    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    if( X_FillArc( Pc_dis, wn, x0, y0, xsize, ysize, sa, da ) )
	do_auto_flush() ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

void fillarc_( integer *wn, real *xcen, real *ycen, real *xrad, real *yrad,
	       real *sang, real *eang, integer *idir )
{
    eggx_fillarc( *wn, *xcen, *ycen, *xrad, *yrad,
		  *sang, *eang, *idir ) ;
}

void eggx_fillcirc( int wn, double xcen, double ycen, double xrad, double yrad ) 
{
    eggx_fillarc( wn,xcen,ycen,xrad,yrad,0,360,1 ) ;
}

void fillcirc_( integer *wn, real *xcen, real *ycen, real *xrad, real *yrad )
{
    eggx_fillarc( *wn, *xcen, *ycen, *xrad, *yrad, 0,360,1 ) ;
}

/* 円弧を描く[8] */
void eggx_drawarc( int wn, double xcen, double ycen, double xrad, double yrad, 
		   double sang, double eang, int idir )
{
    int sa,da ;
    int x0,y0,x1,y1,xsize,ysize ;

    xyconv(wn,xcen-xrad,ycen-yrad,&x0,&y0) ;
    xyconv(wn,xcen+xrad,ycen+yrad,&x1,&y1) ;
    xsize=abs(x1-x0) ;
    ysize=abs(y1-y0) ;
    if( x1<x0 ) x0=x1 ;
    if( y1<y0 ) y0=y1 ;
    
    if ( idir == -1 ) {
	sa = _eggx_iround(sang * 64.0) ;
	if ( eang < sang ) da = -sa + _eggx_iround(eang * 64.0) ;
	else da = -sa - _eggx_iround((360.0 - eang) * 64.0) ;
    }
    else {
	sa = _eggx_iround(sang * 64.0) ;
	da = _eggx_iround(eang * 64.0) - sa ;
    }
    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    if( X_DrawArc( Pc_dis, wn, x0, y0, xsize, ysize, sa, da ) )
	do_auto_flush() ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

void drawarc_( integer *wn, real *xcen, real *ycen, real *xrad, real *yrad,
	       real *sang, real *eang, integer *idir )
{
    eggx_drawarc( *wn, *xcen, *ycen, *xrad, *yrad, 
		  *sang, *eang, *idir ) ;
}

void eggx_drawcirc( int wn, double xcen, double ycen, double xrad, double yrad ) 
{
    eggx_drawarc( wn,xcen,ycen,xrad,yrad,0,360,1 ) ;
}

void eggx_circle( int wn, double xcen, double ycen, double xrad, double yrad ) 
{
    eggx_drawarc( wn,xcen,ycen,xrad,yrad,0,360,1 ) ;
}

void drawcirc_( integer *wn, real *xcen, real *ycen, real *xrad, real *yrad )
{
    eggx_drawarc( *wn, *xcen, *ycen, *xrad, *yrad, 0,360,1 ) ;
}

/* 円弧を描く(互換routine) */
void arc_( real *xcen, real *ycen, real *rad,
	   real *sang, real *eang, integer *idir )
{
    eggx_drawarc( Wn,*xcen,*ycen,*rad,*rad,*sang,*eang,*idir ) ;
}

/* 円弧を描く(互換routine) */
void circ1_( real *xcen, real *ycen, real *rad )
{
    eggx_drawarc( Wn,*xcen,*ycen,*rad,*rad,0,360,1) ;
}

/* 長方形を描く */
void eggx_drawrect( int wn, double x, double y, double w, double h )
{
    int x0,y0,x1,y1,xs,ys,width,height ;
    xyconv( wn,x,y,&x0,&y0 ) ;
    xyconv( wn,x+w,y+h,&x1,&y1 ) ;
    if( x0<x1 ) xs=x0 ;
    else xs=x1 ;
    if( y0<y1 ) ys=y0 ;
    else ys=y1 ;
    width=abs(x1-x0) ;
    height=abs(y1-y0) ;
    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    if( X_DrawRectangle( Pc_dis,wn,xs,ys,width,height ) )
	do_auto_flush() ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

void drawrect_( integer *wn, real *x, real *y, real *w, real *h )
{
    eggx_drawrect( *wn, *x, *y, *w, *h ) ;
}

/* 長方形を塗り潰す */
void eggx_fillrect( int wn, double x, double y, double w, double h )
{
    int x0,y0,x1,y1,xs,ys,width,height,f ;
    xyconv( wn,x,y,&x0,&y0 ) ;
    xyconv( wn,x+w,y+h,&x1,&y1 ) ;
    if( x0<x1 ) xs=x0 ;
    else xs=x1 ;
    if( y0<y1 ) ys=y0 ;
    else ys=y1 ;
    width=abs(x1-x0) ;
    height=abs(y1-y0) ;
    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */
    f=X_FillRectangle( Pc_dis,wn,xs,ys,width,height ) ;
    /* X_DrawRectangle( Pc_dis,*wn,xs,ys,width,height ) ; */
    if(f) do_auto_flush() ;
    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;
}

void fillrect_( integer *wn, real *x, real *y, real *w, real *h )
{
    eggx_fillrect( *wn, *x, *y, *w, *h ) ;
}

/* 複数の点を描く */
void eggx_drawpts( int wn, const double x[], const double y[], int n )
{
    int xx, yy, i, f=0;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*n);
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn, x[i], y[i], &xx, &yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    f = X_DrawPoints( Pc_dis, wn, points, n );
    if ( f ) do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void eggx_drawptsf( int wn, const float x[], const float y[], int n )
{
    int xx, yy, i, f=0;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*n);
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn, x[i], y[i], &xx, &yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    f = X_DrawPoints( Pc_dis, wn, points, n );
    if ( f ) do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void drawpts_( integer *wn, real x[], real y[], integer *n )
{
    int i ;
    float *xx ;
    float *yy ;
    if ( *n < 1 ) return;
    xx=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    yy=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    for( i=0 ; i < *n ; i++ ) xx[i] = (float)(x[i]) ;
    for( i=0 ; i < *n ; i++ ) yy[i] = (float)(y[i]) ;
    eggx_drawptsf( *wn, xx, yy, *n ) ;
    free(yy) ;
    free(xx) ;
}

/* 折線を描く */
void eggx_drawlines( int wn, const double x[], const double y[], int n )
{
    int pst = needs_pset(wn);
    int xx, yy, i, f=0;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*n);
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn, x[i], y[i], &xx, &yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    if ( pst ) X_DrawPoint( Pc_dis, wn, points[0].x, points[0].y );
    f = X_DrawLines( Pc_dis, wn, points, n );
    if ( pst ) X_DrawPoint( Pc_dis, wn, points[n-1].x, points[n-1].y );
    if ( f ) do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void eggx_drawlinesf( int wn, const float x[], const float y[], int n )
{
    int pst = needs_pset(wn);
    int xx, yy, i, f=0;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*n);
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn, x[i], y[i], &xx, &yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    if ( pst ) X_DrawPoint( Pc_dis, wn, points[0].x, points[0].y );
    f = X_DrawLines( Pc_dis, wn, points, n );
    if ( pst ) X_DrawPoint( Pc_dis, wn, points[n-1].x, points[n-1].y );
    if ( f ) do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void drawlines_( integer *wn, real x[], real y[], integer *n )
{
    int i ;
    float *xx ;
    float *yy ;
    if ( *n < 1 ) return;
    xx=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    yy=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    for( i=0 ; i < *n ; i++ ) xx[i] = (float)(x[i]) ;
    for( i=0 ; i < *n ; i++ ) yy[i] = (float)(y[i]) ;
    eggx_drawlinesf( *wn, xx, yy, *n ) ;
    free(yy) ;
    free(xx) ;
}

/* 多角形を描く */
void eggx_drawpoly( int wn, const double x[], const double y[], int n )
{
    /* int pst = needs_pset(wn); */
    int xx, yy, i, f=0;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*(n+1));
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn, x[i], y[i], &xx, &yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    points[i].x = points[0].x;
    points[i].y = points[0].y;
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    f = X_DrawLines( Pc_dis, wn, points, n+1 );
    if ( f ) do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void eggx_drawpolyf( int wn, const float x[], const float y[], int n )
{
    /* int pst = needs_pset(wn); */
    int xx, yy, i, f=0;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*(n+1));
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn, x[i], y[i], &xx, &yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    points[i].x = points[0].x;
    points[i].y = points[0].y;
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    f = X_DrawLines( Pc_dis, wn, points, n+1 );
    if ( f ) do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void drawpoly_( integer *wn, real x[], real y[], integer *n )
{
    int i ;
    float *xx ;
    float *yy ;
    if ( *n < 1 ) return;
    xx=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    yy=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    for( i=0 ; i < *n ; i++ ) xx[i] = (float)(x[i]) ;
    for( i=0 ; i < *n ; i++ ) yy[i] = (float)(y[i]) ;
    eggx_drawpolyf( *wn, xx, yy, *n ) ;
    free(yy) ;
    free(xx) ;
}

/* 多角形を塗り潰す */
void eggx_fillpoly(int wn, const double x[], const double y[], int n, int shape)
{
    int sh = Complex;
    int xx, yy, i;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*n);
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn,x[i],y[i],&xx,&yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    if ( shape ) sh = Convex;
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    if ( X_FillPolygon( Pc_dis,wn,points,n,sh,CoordModeOrigin) )
	do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void eggx_fillpolyf(int wn, const float x[], const float y[], int n, int shape)
{
    int sh = Complex;
    int xx, yy, i;
    XPoint *points = NULL;
    if ( n < 1 ) return;
    points = (XPoint *)_eggx_xmalloc(sizeof(XPoint)*n);
    for ( i=0 ; i < n ; i++ ) {
	xyconv( wn,x[i],y[i],&xx,&yy );
	points[i].x = xx;
	points[i].y = yy;
    }
    if ( shape ) sh = Convex;
    wait_child();	/* imgsave中は待たせる */
    Ihflg = 1;		/* 割り込み禁止 */
    if ( X_FillPolygon( Pc_dis,wn,points,n,sh,CoordModeOrigin) )
	do_auto_flush();
    Ihflg = 0;		/* 割り込み禁止解除 */
    if ( points != NULL ) free(points);
    chkexit();
}

void fillpoly_( integer *wn, real x[], real y[], integer *n, integer *shape )
{
    int i ;
    float *xx ;
    float *yy ;
    if ( *n < 1 ) return;
    xx=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    yy=(float *)_eggx_xmalloc(sizeof(float)*(*n)) ;
    for( i=0 ; i < *n ; i++ ) xx[i] = (float)(x[i]) ;
    for( i=0 ; i < *n ; i++ ) yy[i] = (float)(y[i]) ;
    eggx_fillpolyf( *wn, xx, yy, *n, *shape ) ;
    free(yy) ;
    free(xx) ;
}

/* ============= ビットマップ画像を扱う関数 ============= */

/* for C... */
int eggx_putimg24( int wn, double x, double y,
		   int width, int height, unsigned char *buf )
{
    XImage image;
    int xx, yy;
    size_t npixels;
    npixels = width * height;

    if ( Pc_depth < 16 ) return (-1);
    if ( width <= 0 ) return (0);
    if ( height <= 0 ) return (0);
    if ( buf == NULL ) return (-1);

    xyconv( wn, x,y, &xx,&yy );
    if ( (Pc[wn].attributes & BOTTOM_LEFT_ORIGIN) ) yy -= height - 1;

    image.format           = ZPixmap;
    image.width            = width;
    image.height           = height;
    image.xoffset          = 0;
    image.byte_order       = MSBFirst;
    image.bitmap_bit_order = MSBFirst;

    if ( 16 < Pc_depth ) {	/* 24bppの場合 */
	image.bits_per_pixel   = 32;
	image.bytes_per_line   = width * 4;
	image.bitmap_unit      = 32;
	image.bitmap_pad       = 32;
	image.depth            = 24;
	/* fprintf(stderr, "[DEBUG] Red_sft = %d, Green_sft = %d, Blue_sft = %d\n",
		   Red_sft, Green_sft, Blue_sft); */
	if ( Red_sft == 16 && Green_sft == 8 && Blue_sft == 0 ) {
	    image.data             = (char *)buf;
	}
	else {
	    int i, j, k, fm=0;
	    size_t new_sz;
	    if ( Red_sft != 24 && Green_sft != 24 && Blue_sft != 24 ) fm = 0;
	    else if ( Red_sft != 0 && Green_sft != 0 && Blue_sft != 0 ) fm = 3;
	    else if ( Red_sft != 8 && Green_sft != 8 && Blue_sft != 8 ) fm = 2;
	    else if ( Red_sft != 16 && Green_sft != 16 && Blue_sft != 16 ) fm = 1;
 
	    new_sz = sizeof(unsigned char) * npixels * 4;
	    if ( Sz_tmp_img24buf < new_sz ) {
		Tmp_img24buf = (unsigned char *)_eggx_xrealloc(Tmp_img24buf, new_sz);
		Sz_tmp_img24buf = new_sz;
	    }
	    image.data             = (char *)Tmp_img24buf;
	    for ( j=0, i=0, k=0 ; i < npixels ; i++ ) {
		k++;
		Tmp_img24buf[j+3-Red_sft/8] = buf[k++];
		Tmp_img24buf[j+3-Green_sft/8] = buf[k++];
		Tmp_img24buf[j+3-Blue_sft/8] = buf[k++];
		Tmp_img24buf[j+fm] = 0;
		j += 4;
	    }
	}
    }
    else {
	int i, j, k;
	size_t new_sz;
	image.bits_per_pixel   = 16;
	image.bytes_per_line   = width * 2;
	image.bitmap_unit      = 16;
	image.bitmap_pad       = 16;
	image.depth            = 16;

	new_sz = sizeof(unsigned char) * npixels * 2;
	if ( Sz_tmp_img24buf < new_sz ) {
	    Tmp_img24buf = (unsigned char *)_eggx_xrealloc(Tmp_img24buf, new_sz);
	    Sz_tmp_img24buf = new_sz;
	}
	image.data             = (char *)Tmp_img24buf;
	for ( j=0, i=0, k=0 ; i < npixels ; i++ ) {
	    unsigned short tmp_img = 0;
	    k++;
	    tmp_img |=
		(buf[k++] >> (8-Red_depth)) << Red_sft;
	    tmp_img |=
		(buf[k++] >> (8-Green_depth)) << Green_sft;
	    tmp_img |=
		(buf[k++] >> (8-Blue_depth)) << Blue_sft;
	    Tmp_img24buf[j++] = (unsigned char)(tmp_img >> 8);
	    Tmp_img24buf[j++] = (unsigned char)(tmp_img & 0x0ff);
	}
    }
    wait_child();	/* imgsave中は待たせる */
    Ihflg=1;	/* 割り込み禁止 */
    XPutImage( Pc_dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc,
	       &image, 0,0, xx,yy, width,height );
    if ( Pc[wn].wly == Pc[wn].sly ) {
	XGCValues gv;
	gv.tile = Pc[wn].pix[Pc[wn].wly];
	XChangeGC( Pc_dis, Pc[wn].pxgc, GCTile, &gv );
	XFillRectangle( Pc_dis, Pc[wn].win, Pc[wn].pxgc, 
			0,0, Pc[wn].wszx,Pc[wn].wszy );
	if ( Pc[wn].iconwin != None ) {
	    XFillRectangle( Pc_dis, Pc[wn].iconwin, Pc[wn].pxgc, 
			    0,0, Pc[wn].wszx,Pc[wn].wszy );
	}
	do_auto_flush();
    }
    Ihflg=0;	/* 割り込み禁止解除 */
    chkexit();
    return (0);
}

/* for FORTRAN */
void putimg24_( integer *wn, real *x, real *y,
		integer *width, integer *height, integer *buf )
{
    unsigned char *lbuf ;
    unsigned char *cadr ;
    integer *ladr ;
    int i,n ;

    n=(*width)*(*height) ;
    if ( n < 1 ) return;
    lbuf=(unsigned char *)_eggx_xmalloc(sizeof(unsigned char)*n*4) ;
    cadr=lbuf ;
    ladr=buf ;
    for( i=0 ; i<n ; i++ ){
	*cadr++=0 ;
	*cadr++=(unsigned char)(*ladr++) ;
	*cadr++=(unsigned char)(*ladr++) ;
	*cadr++=(unsigned char)(*ladr++) ;
    }
    if( eggx_putimg24( *wn,*x,*y,*width,*height,lbuf ) == -1 ){
	fprintf(stderr,"EGGX: [WARNING] Depth of X server is not enough.\n") ;
    }
    free(lbuf) ;
}

/* マスク付き */
int eggx_putimg24m( int wn, double x, double y,
		    int width, int height, unsigned char *buf )
{
    XImage image;
    int xx, yy;
    size_t new_sz;
    size_t i, j, k;
    size_t npixels;
    npixels = width * height;

    if ( Pc_depth < 16 ) return (-1);
    if ( width <= 0 ) return (0);
    if ( height <= 0 ) return (0);
    if ( buf == NULL ) return (-1);

    xyconv( wn, x,y, &xx,&yy );
    if ( (Pc[wn].attributes & BOTTOM_LEFT_ORIGIN) ) yy -= height - 1;

    image.format           = ZPixmap;
    image.width            = width;
    image.height           = height;
    image.xoffset          = 0;
    image.byte_order       = MSBFirst;
    image.bitmap_bit_order = MSBFirst;

    if ( 16 < Pc_depth ) {	/* 24bppの場合 */
	image.bitmap_unit  = 32;
    } else {
	image.bitmap_unit  = 16;
    }

    if ( Pix_mask_width < width || Pix_mask_height < height ) {
	XGCValues tmp_gv;
	if ( 0 < Pix_mask_width ) {
	    XFreeGC( Pc_dis, Gc_pix_mask );
	    Gc_pix_mask = None;
	    XFreePixmap( Pc_dis, Pix_mask );
	    Pix_mask = None;
	    Pix_mask_width = 0;
	    Pix_mask_height = 0;
	}
	Pix_mask = XCreatePixmap( Pc_dis, RootWindow(Pc_dis,0),
				  width, height, 1 );
	Pix_mask_width = width;
	Pix_mask_height = height;
	tmp_gv.foreground = 1;
	tmp_gv.background = 0;
	Gc_pix_mask = XCreateGC( Pc_dis, Pix_mask,
				 GCForeground | GCBackground, &tmp_gv );
	XSetGraphicsExposures( Pc_dis, Gc_pix_mask, False ) ;
    }

    image.bits_per_pixel   = 1;
    image.bytes_per_line   = (width + 7) / 8;
    image.bitmap_pad       = 8;
    image.depth            = 1;

    new_sz = sizeof(unsigned char) * image.bytes_per_line * height;
    if ( Sz_tmp_img24buf < new_sz ) {
	Tmp_img24buf = (unsigned char *)_eggx_xrealloc(Tmp_img24buf, new_sz);
	Sz_tmp_img24buf = new_sz;
    }
    memset(Tmp_img24buf, 0, new_sz);

    image.data             = (char *)Tmp_img24buf;

    for ( i=0, k=0 ; i < height ; i++ ) {
	unsigned char mval = 1;
	unsigned char *ptr = Tmp_img24buf + i * image.bytes_per_line;
	for ( j=0 ; j < width ; j++ ) {
	    if ( buf[k] != 0 ) ptr[j/8] |= (mval << (7 - (j % 8)));
	    /* for test */
	    /* if ( buf[k+1] > 10 ) ptr[j/8] |= (mval << (7 - (j % 8))); */
	    k += 4;
	}
    }

    XPutImage( Pc_dis, Pix_mask, Gc_pix_mask, &image, 0,0, 0,0, width,height );

    if ( 16 < Pc_depth ) {	/* 24bppの場合 */
	image.bits_per_pixel   = 32;
	image.bytes_per_line   = width * 4;
	image.bitmap_pad       = 32;
	image.depth            = 24;
	if ( Red_sft == 16 && Green_sft == 8 && Blue_sft == 0 ) {
	    image.data             = (char *)buf;
	}
	else {
	    size_t fm = 0;
	    if ( Red_sft != 24 && Green_sft != 24 && Blue_sft != 24 ) fm = 0;
	    else if ( Red_sft != 0 && Green_sft != 0 && Blue_sft != 0 ) fm = 3;
	    else if ( Red_sft != 8 && Green_sft != 8 && Blue_sft != 8 ) fm = 2;
	    else if ( Red_sft != 16 && Green_sft != 16 && Blue_sft != 16 ) fm = 1;
 
	    new_sz = sizeof(unsigned char) * npixels * 4;
	    if ( Sz_tmp_img24buf < new_sz ) {
		Tmp_img24buf = (unsigned char *)_eggx_xrealloc(Tmp_img24buf, new_sz) ;
		Sz_tmp_img24buf = new_sz;
	    }
	    image.data             = (char *)Tmp_img24buf;
	    for ( j=0, i=0, k=0 ; i < npixels ; i++ ) {
		k++;
		Tmp_img24buf[j+3-Red_sft/8] = buf[k++];
		Tmp_img24buf[j+3-Green_sft/8] = buf[k++];
		Tmp_img24buf[j+3-Blue_sft/8] = buf[k++];
		Tmp_img24buf[j+fm] = 0;
		j+=4;
	    }
	}
    }
    else{
	image.bits_per_pixel   = 16;
	image.bytes_per_line   = width * 2;
	image.bitmap_pad       = 16;
	image.depth            = 16;

	new_sz = sizeof(unsigned char) * npixels * 2;
	if ( Sz_tmp_img24buf < new_sz ) {
	    Tmp_img24buf = (unsigned char *)_eggx_xrealloc(Tmp_img24buf, new_sz);
	    Sz_tmp_img24buf = new_sz;
	}
	image.data             = (char *)Tmp_img24buf;
	for ( j=0,i=0,k=0 ; i < npixels ; i++ ) {
	    unsigned short tmp_img = 0;
	    k++;
	    tmp_img |=
		(buf[k++] >> (8-Red_depth)) << Red_sft;
	    tmp_img |=
		(buf[k++] >> (8-Green_depth)) << Green_sft;
	    tmp_img |=
		(buf[k++] >> (8-Blue_depth)) << Blue_sft;
	    Tmp_img24buf[j++] = (unsigned char)(tmp_img >> 8);
	    Tmp_img24buf[j++] = (unsigned char)(tmp_img & 0x0ff);
	}
    }
    wait_child();	/* imgsave中は待たせる */
    Ihflg=1;	/* 割り込み禁止 */
    XSetClipMask( Pc_dis, Pc[wn].gc, Pix_mask );
    XSetClipOrigin( Pc_dis, Pc[wn].gc, xx, yy );
    XPutImage( Pc_dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc,
	       &image, 0,0, xx,yy, width,height );
    XSetClipMask( Pc_dis, Pc[wn].gc, None );
    if ( Pc[wn].wly == Pc[wn].sly ) {
	XGCValues gv;
	gv.tile = Pc[wn].pix[Pc[wn].wly];
	XChangeGC( Pc_dis, Pc[wn].pxgc, GCTile, &gv );
	XFillRectangle( Pc_dis, Pc[wn].win, Pc[wn].pxgc, 
			0,0, Pc[wn].wszx, Pc[wn].wszy );
	if ( Pc[wn].iconwin != None ) {
	    XFillRectangle( Pc_dis, Pc[wn].iconwin, Pc[wn].pxgc, 
			    0,0, Pc[wn].wszx,Pc[wn].wszy );
	}
	do_auto_flush();
    }
    Ihflg=0;	/* 割り込み禁止解除 */
    chkexit();
    return (0);
}

int eggx_gputimage( int wn, double x, double y,
		    unsigned char *buf, int width, int height, int msk )
{
    if ( msk == 0 ) return eggx_putimg24(wn,x,y,width,height,buf);
    else return eggx_putimg24m(wn,x,y,width,height,buf);
}

/* "cmd arg arg" といった文字列を分割して argv な形式にする */
/* 第二，第三引数で返される領域は，呼び出し側で開放が必要 */
static int create_args( const char *args_string, 
			char **args_buf_r, char ***args_r )
{
    char **args = NULL;
    char *f_ptr;
    int i, nargs;
    /* argsの作成 */
    *args_buf_r = _eggx_xstrdup(args_string);
    f_ptr = *args_buf_r;
    i = 0;
    while ( 1 ) {
	int spn;
	spn = strspn(f_ptr," ");
	if ( 0 < spn ) {
	    *f_ptr = '\0';
	    f_ptr += spn;
	}
	spn = strcspn(f_ptr," ");
	args = (char **)_eggx_xrealloc(args,sizeof(char *)*(i+1));
	if ( 0 < spn ) {
	    args[i++] = f_ptr;
	    f_ptr += spn;
	}
	else {
	    args[i] = NULL;
	    break;
	}
    }
    nargs = i;

    *args_r = args;
    return nargs;
}

/* pnm ファイルのヘッダ部を1行読む */
static char *read_next( FILE *fp, char *buf, size_t sz )
{
    char *rt = NULL;
    while ( 1 ) {
	int i;
	rt = fgets(buf,sz,fp);
	if ( rt == NULL ) break;
	if ( strchr(buf,'\n') == NULL ) {
	    while ( 1 ) {
		int ch = fgetc(fp);
		if ( ch == '\n' || ch == EOF ) break;
	    }
	}
	for ( i=0 ; buf[i] == ' ' || buf[i] == '\t' ; i++ );
	if ( buf[i] != '#' ) {
	    rt = buf + i;
	    break;
	}
    }
    return rt;
}

/* バイナリ pbm,pgm,ppm,pam 形式のファイルを読む．その他の形式は pngtopnm 等 */
/* の変換プログラムで読み込みが可能．変換プログラムは filter で指定する */
/* 返り値は確保され画像が保存されたバッファで，呼び出し側で開放が必要 */
unsigned char *eggx_readimage( const char *filter, const char *filename,
			       int *r_width, int *r_height, int *r_msk )
{
    unsigned char *rt_buf = NULL;
    char *fn = NULL;
    char *tmp1 = NULL;
    char **args = NULL;
    unsigned char *line_buf = NULL;
    char txt_line_buf[256];
    char pam_stdout[] = "PAM:-";		/* for convert */
#ifdef __CYGWIN__
    char str_convert_exe[12] = "convert.exe";
#endif
    const char *pos;
    FILE *fp = NULL;
    int width = 0, height = 0, maxv = 0, nchnl = 0, nbytes = 0, msk = 0;
    size_t ii, sz_line = 0;
    int i, pnm_format;				/* fmt: '4'〜'7' のいずれか */
    int found_alpha = 0;

    if ( filename == NULL ) goto quit;

    if ( filter != NULL &&				/* ImageMagick */
	 (strcmp(filter,"convert")==0 || strcmp(filter,"convert.exe")==0) ) {
	int nargs;
	int pfds[2];
	/* */
	nargs = create_args(filter, &tmp1, &args);
	args = (char **)_eggx_xrealloc(args,sizeof(char *)*(nargs+3));
	fn = _eggx_xstrdup(filename);
	args[nargs] = fn;
	nargs++;
	args[nargs] = pam_stdout;
	nargs++;
	args[nargs] = NULL;
	nargs++;
#ifdef __CYGWIN__
	args[0] = str_convert_exe;
#endif
	//{for(i=0;i<nargs;i++){fprintf(stderr,"DEBUG: [%s]\n",args[i]);}}
	/* */
	if ( pipe(pfds) < 0 ) {
	    fprintf(stderr,"EGGX: [ERROR] pipe() failed\n");
	    goto quit;
	}
	if ( (Cpid_tmp=fork()) < 0 ) {
	    fprintf(stderr,"EGGX: [ERROR] fork() failed\n");
	    goto quit;
	}
	if ( Cpid_tmp == 0 ) {	/* 子プロセス */
	    dup2( pfds[1],1 );	/* 読み込み用 */
	    close( pfds[1] );
	    close( pfds[0] );
	    execvp( *args, args );
	    fprintf(stderr,"EGGX: [ERROR] Cannot exec '%s' command.\n",*args);
	    _exit(-1);
	}
	/* 親 */
	close( pfds[1] );
	/* */
	fp = fdopen(pfds[0],"rb");
    }
    else if ( filter != NULL && *filter != '\0' ) {	/* netpbm */
	int nargs;
	int pfds[2];
	nargs = create_args(filter, &tmp1, &args);
	args = (char **)_eggx_xrealloc(args,sizeof(char *)*(nargs+2));
	fn = _eggx_xstrdup(filename);
	args[nargs] = fn;
	nargs++;
	args[nargs] = NULL;
	nargs++;
	//{for(i=0;i<nargs;i++){fprintf(stderr,"DEBUG: [%s]\n",args[i]);}}
	/* */
	if ( pipe(pfds) < 0 ) {
	    fprintf(stderr,"EGGX: [ERROR] pipe() failed\n");
	    goto quit;
	}
	if ( (Cpid_tmp=fork()) < 0 ) {
	    fprintf(stderr,"EGGX: [ERROR] fork() failed\n");
	    goto quit;
	}
	if ( Cpid_tmp == 0 ) {	/* 子プロセス */
	    dup2( pfds[1],1 );	/* 読み込み用 */
	    close( pfds[1] );
	    close( pfds[0] );
	    execvp( *args, args );
	    fprintf(stderr,"EGGX: [ERROR] Cannot exec '%s' command.\n",*args);
	    _exit(-1);
	}
	/* 親 */
	close( pfds[1] );
	/* */
	fp = fdopen(pfds[0],"rb");
    } else {
	fp = fopen(filename,"rb");
    }

    if ( fp == NULL ) {
	fprintf(stderr,"EGGX: [ERROR] Cannot open file: %s\n",filename);
	goto quit;
    }

    /* 読み込み開始 */
    pos = read_next(fp,txt_line_buf,256);
    if ( pos == NULL || pos[0] != 'P' ) {
	fprintf(stderr,"EGGX: [ERROR] Invalid stream (1)\n");
	goto quit;
    }

    pnm_format = pos[1];

    if ( '4' <= pnm_format && pnm_format <= '6' ) {	/* binary p[bgp]m */
	int n;
	pos = read_next(fp,txt_line_buf,256);
	if ( pos == NULL ) {
	    fprintf(stderr,"EGGX: [ERROR] Invalid stream (2)\n");
	    goto quit;
	}
	n = sscanf(pos,"%d %d\n",&width,&height);
	if ( n != 2 ) {
	    fprintf(stderr,"EGGX: [ERROR] Invalid stream (3)\n");
	    goto quit;
	}
	if ( pnm_format != '4' ) {			/* pgm,ppm */
	    pos = read_next(fp,txt_line_buf,256);
	    if ( pos == NULL ) {
		fprintf(stderr,"EGGX: [ERROR] Invalid stream (4)\n");
		goto quit;
	    }
	    n = sscanf(pos,"%d\n",&maxv);
	    if ( n != 1 ) {
		fprintf(stderr,"EGGX: [ERROR] Invalid stream (5)\n");
		goto quit;
	    }
	    nbytes = (maxv < 256) ? 1 : 2;
	    if ( pnm_format == '5' ) nchnl = 1;
	    else nchnl = 3;
	    msk = 0;
	}
	else {						/* pbm */
	    nchnl = 0;
	    msk = 0;
	    maxv = 1;
	}
    }
    else if ( pnm_format == '7' ) {			/* pam */
	nchnl = -1;
	while ( 1 ) {
	    int n = 1;
	    pos = read_next(fp,txt_line_buf,256);
	    if ( pos == NULL ) {
		fprintf(stderr,"EGGX: [ERROR] Invalid stream (2)\n");
		goto quit;
	    }
	    if ( strncmp(pos,"WIDTH",5)==0 && (ch_is_space(pos[5])) ) {
		n = sscanf(pos,"%*s %d",&width);
	    }
	    else if ( strncmp(pos,"HEIGHT",6)==0 && (ch_is_space(pos[6])) ) {
		n = sscanf(pos,"%*s %d",&height);
	    }
	    else if ( strncmp(pos,"DEPTH",5)==0 && (ch_is_space(pos[5])) ) {
		n = sscanf(pos,"%*s %d",&nchnl);
	    }
	    else if ( strncmp(pos,"MAXVAL",6)==0 && (ch_is_space(pos[6])) ) {
		n = sscanf(pos,"%*s %d",&maxv);
	    }
	    else if ( strncmp(pos,"ENDHDR",6)==0 &&
		      (ch_is_space(pos[6]) != 0 || pos[6] == '\0') ) {
		break;
	    }
	    if ( n != 1 ) {
		fprintf(stderr,"EGGX: [ERROR] Invalid stream (3)\n");
		goto quit;
	    }
	}
	nbytes = (maxv < 256) ? 1 : 2;
	if ( nchnl == 2 || nchnl == 4 ) msk = 1;
	else msk = 0;
    }
    else {
	fprintf(stderr,"EGGX: [ERROR] Unsupported type: P%c\n",pnm_format);
	goto quit;
    }

    if ( width < 1 ) {
	fprintf(stderr,"EGGX: [ERROR] Invalid width: %d\n",width);
	goto quit;
    }
    if ( height < 1 ) {
	fprintf(stderr,"EGGX: [ERROR] Invalid height: %d\n",height);
	goto quit;
    }
    if ( maxv < 1 || 65535 < maxv ) {
	fprintf(stderr,"EGGX: [ERROR] Unsupported depth: %d\n",maxv+1);
	goto quit;
    }
    if ( nchnl < 0 || 4 < nchnl ) {
	fprintf(stderr,"EGGX: [ERROR] Unsupported type\n");
	goto quit;
    }

    if ( pnm_format == '4' ) {					/* pbm */
	sz_line = (width + 7) / 8;
    }
    else {
	sz_line = nbytes * nchnl * width;
    }

    //fprintf(stderr,"DEBUG: format : P%c\n",pnm_format);

    line_buf = (unsigned char *)_eggx_xmalloc(sz_line);
    rt_buf = (unsigned char *)_eggx_xmalloc((4 * width) * height);

    ii = 0;
    for ( i=0 ; i < height ; i++ ) {
	if ( fread(line_buf, 1, sz_line, fp) != sz_line ) {
	    fprintf(stderr,"EGGX: [ERROR] Invalid bitmap stream\n");
	    free(rt_buf);
	    rt_buf = NULL;
	    goto quit;
	}
	if ( nchnl == 3 ) {			/* r,g,b */
	    size_t j;
	    if ( nbytes == 1 ) {
		for ( j=0 ; j < sz_line ; ) {
		    rt_buf[ii++] = 255;
		    rt_buf[ii++] = 255 * line_buf[j++] / maxv;
		    rt_buf[ii++] = 255 * line_buf[j++] / maxv;
		    rt_buf[ii++] = 255 * line_buf[j++] / maxv;
		}
	    }
	    else {	/* nbytes == 2 */
		int vl;
		for ( j=0 ; j < sz_line ; ) {
		    rt_buf[ii++] = 255;
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii++] = 255 * vl / maxv;
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii++] = 255 * vl / maxv;
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii++] = 255 * vl / maxv;
		}
	    }
	}
	else if ( nchnl == 4 ) {		/* r,g,b,a */
	    size_t j;
	    if ( nbytes == 1 ) {
		for ( j=0 ; j < sz_line ; ) {
		    rt_buf[ii+1] = 255 * line_buf[j++] / maxv;
		    rt_buf[ii+2] = 255 * line_buf[j++] / maxv;
		    rt_buf[ii+3] = 255 * line_buf[j++] / maxv;
		    rt_buf[ii+0] = 255 * line_buf[j++] / maxv;
		    if (0 < rt_buf[ii+0] && rt_buf[ii+0] < 255) found_alpha=1;
		    ii += 4;
		}
	    }
	    else {	/* nbytes == 2 */
		int vl;
		for ( j=0 ; j < sz_line ; ) {
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii+1] = 255 * vl / maxv;
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii+2] = 255 * vl / maxv;
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii+3] = 255 * vl / maxv;
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii+0] = 255 * vl / maxv;
		    if (0 < rt_buf[ii+0] && rt_buf[ii+0] < 255) found_alpha=1;
		    ii += 4;
		}
	    }
	}
	else if ( nchnl == 1 ) {		/* pgm */
	    size_t j;
	    if ( nbytes == 1 ) {
		for ( j=0 ; j < sz_line ; ) {
		    rt_buf[ii++] = 255;
		    rt_buf[ii++] = 255 * line_buf[j] / maxv;
		    rt_buf[ii++] = 255 * line_buf[j] / maxv;
		    rt_buf[ii++] = 255 * line_buf[j] / maxv;
		    j++;
		}
	    }
	    else {
		int vl;
		for ( j=0 ; j < sz_line ; ) {
		    rt_buf[ii++] = 255;
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii++] = 255 * vl / maxv;
		    rt_buf[ii++] = 255 * vl / maxv;
		    rt_buf[ii++] = 255 * vl / maxv;
		}
	    }
	}
	else if ( nchnl == 2 ) {		/* grey + a */
	    size_t j;
	    if ( nbytes == 1 ) {
		for ( j=0 ; j < sz_line ; ) {
		    rt_buf[ii] = 255 * line_buf[j+1] / maxv;
		    if (0 < rt_buf[ii] && rt_buf[ii] < 255) found_alpha=1;
		    ii++;
		    rt_buf[ii++] = 255 * line_buf[j] / maxv;
		    rt_buf[ii++] = 255 * line_buf[j] / maxv;
		    rt_buf[ii++] = 255 * line_buf[j] / maxv;
		    j += 2;
		}
	    }
	    else {
		int vl;
		for ( j=0 ; j < sz_line ; ) {
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii+1] = 255 * vl / maxv;
		    rt_buf[ii+2] = 255 * vl / maxv;
		    rt_buf[ii+3] = 255 * vl / maxv;
		    vl = line_buf[j++];  vl <<= 8;  vl |= line_buf[j++];
		    rt_buf[ii+0] = 255 * vl / maxv;
		    if (0 < rt_buf[ii+0] && rt_buf[ii+0] < 255) found_alpha=1;
		    ii += 4;
		}
	    }
	}
	else if ( nchnl == 0 ) {		/* pbm */
	    int j;
	    for ( j=0 ; j < width ; ) {
		int v = line_buf[j/8];
		v &= (1 << (7 - (j % 8)));
		if ( v != 0 ) v = 0;
		else v = 255;
		rt_buf[ii++] = 255;
		rt_buf[ii++] = v;
		rt_buf[ii++] = v;
		rt_buf[ii++] = v;
		j++;
	    }
	}
    }

    if ( found_alpha != 0 ) {
	msk = 256;
    }

 quit:
    if ( 0 < Cpid_tmp ) {
	if ( Pc_eggxhandlers_are_set ) {
	    while ( 0 < Cpid_tmp ) {
		eggx_msleep(10);
	    }
	}
	else {
	    int status;
	    while ( wait(&status) != Cpid_tmp );
	    Cpid_tmp = 0;
	}
    }
    if ( fp != NULL ) fclose(fp);
    if ( args != NULL ) free(args);
    if ( tmp1 != NULL ) free(tmp1);
    if ( fn != NULL ) free(fn);
    if ( line_buf != NULL ) free(line_buf);
    if ( rt_buf != NULL ) {
	if ( r_width != NULL ) *r_width = width;
	if ( r_height != NULL ) *r_height = height;
	if ( r_msk != NULL ) *r_msk = msk;
    }
    return rt_buf;
}

/* ppm or pam 形式のファイルを書く．filter を指定して pnmtopng 等で変換する */
/* 事で，様々な形式で保存が可能．msk の値がノンゼロかファイル名が *.pam なら */
/* pam 形式を出力する */
/* 返り値は正常終了なら 0 で，異常終了なら負の値 */
int eggx_writeimage( const unsigned char *buf, int width, int height, int msk,
		   const char *filter, int depth, const char *argsformat, ... )
{
    int rtv = -1;
    char *out_fname = NULL;
    char *tmp1 = NULL;
    char **args = NULL;
    /* for convert */
    char rgb_stdin[] = "RGB:-";
    char rgba_stdin[] = "RGBA:-";
    char ag_size[] = "-size";
    char *ag_size_vl = NULL;
    char ag_depth[] = "-depth";
    char ag_depth_vl[] = "8";
#ifdef __CYGWIN__
    char str_convert_exe[12] = "convert.exe";
#endif
    unsigned char *line_buf = NULL;
    const char *p0;
    FILE *fp = NULL;
    int out_is_pam;
    va_list ap;

    if ( argsformat == NULL ) goto quit;
    if ( buf == NULL ) goto quit;
    if ( width < 1 ) goto quit;
    if ( height < 1 ) goto quit;

    /* ファイル名を作る */
    va_start(ap, argsformat);
    out_fname = _eggx_vasprintf(argsformat,ap);
    va_end(ap);

    if ( depth < 2 ) depth = 2;
    else if ( 256 < depth ) depth = 256;

    p0 = strstr(out_fname,".pam");
    if ( p0 == NULL ) p0 = strstr(out_fname,".PAM");

    out_is_pam = 0;
    if ( p0 != NULL && p0[4] == '\0' ) out_is_pam = 1;
    if ( msk != 0 ) out_is_pam = 1;

    if ( filter != NULL &&				/* ImageMagick */
	 (strcmp(filter,"convert")==0 || strcmp(filter,"convert.exe")==0) ) {
	int nargs;
	int pfds[2];
	/* 引数 */
	nargs = create_args(filter, &tmp1, &args);
	/* */
	args = (char **)_eggx_xrealloc(args,sizeof(char *)*(nargs+2+2+3));
	args[nargs] = ag_size;
	nargs++;
	ag_size_vl = _eggx_asprintf("%dx%d",width,height);
	args[nargs] = ag_size_vl;
	nargs++;
	args[nargs] = ag_depth;
	nargs++;
	args[nargs] = ag_depth_vl;
	nargs++;
	if ( msk != 0 ) args[nargs] = rgba_stdin;
	else args[nargs] = rgb_stdin;
	nargs++;
	args[nargs] = out_fname;
	nargs++;
	args[nargs] = NULL;
	nargs++;
#ifdef __CYGWIN__
	args[0] = str_convert_exe;
#endif
	//{int i;for(i=0;i<nargs;i++){fprintf(stderr,"DEBUG: [%s]\n",args[i]);}}
	/* コンバータを起動する */
	if ( pipe(pfds) < 0 ) {
	    fprintf(stderr,"EGGX: [ERROR] pipe() failed.\n");
	    goto quit;
	}
	if ( (Cpid_tmp=fork()) < 0 ) {
	    fprintf(stderr,"EGGX: [ERROR] fork() failed.\n");
	    close( pfds[1] );
	    close( pfds[0] );
	    goto quit;
	}
	if ( Cpid_tmp == 0 ) {		/* 子プロセス */
	    dup2( pfds[0], 0 );
	    close( pfds[1] );
	    close( pfds[0] );
	    execvp( args[0], args );
	    fprintf(stderr,"EGGX: [ERROR] Cannot exec '%s'.\n",args[0]);
	    _exit(-1);
	}
	close(pfds[0]);
	fp = fdopen( pfds[1], "wb" );	/* 送信用 */
    }
    else if ( filter != NULL && *filter != '\0' ) {	/* netpbm */
	int nargs;
	int fd, pfds[2];
	/* 引数 */
	nargs = create_args(filter, &tmp1, &args);
	/* fileのcreate */
	fd = open( out_fname, O_WRONLY|O_CREAT|O_TRUNC,
		   S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );
	if ( fd == -1 ) {
	    fprintf(stderr,"EGGX: [ERROR] Cannot create a file.\n");
	    goto quit;
	}
	/* コンバータを起動する */
	if ( pipe(pfds) < 0 ) {
	    fprintf(stderr,"EGGX: [ERROR] pipe() failed.\n");
	    close(fd);
	    goto quit;
	}
	if ( (Cpid_tmp=fork()) < 0 ) {
	    fprintf(stderr,"EGGX: [ERROR] fork() failed.\n");
	    close(fd);
	    close( pfds[1] );
	    close( pfds[0] );
	    goto quit;
	}
	if ( Cpid_tmp == 0 ) {		/* 子プロセス */
	    dup2( fd, 1 );
	    dup2( pfds[0], 0 );
	    close( pfds[1] );
	    close( pfds[0] );
	    execvp( args[0], args );
	    fprintf(stderr,"EGGX: [ERROR] Cannot exec '%s'.\n",args[0]);
	    _exit(-1);
	}
	close(fd);
	close(pfds[0]);
	fp = fdopen( pfds[1], "wb" );	/* 送信用 */
    } else {
	fp = fopen(out_fname, "wb");
    }

    if ( fp == NULL ) {
	fprintf(stderr,"EGGX: [ERROR] cannot open file or fd.\n");
	goto quit;
    }

    if ( ag_size_vl == NULL ) {				/* not "convert" */
	if ( out_is_pam ) {
	    fprintf(fp,"P7\n");				/* pam */
	    fprintf(fp,"WIDTH %d\n",width);
	    fprintf(fp,"HEIGHT %d\n",height);
	    if ( msk == 0 ) {
		fprintf(fp,"DEPTH 3\n");		/* r,b,g */
	    }
	    else {
		fprintf(fp,"DEPTH 4\n");		/* r,b,g,alpha */
	    }
	    fprintf(fp,"MAXVAL %d\n",depth-1);
	    fprintf(fp,"ENDHDR\n");
	}
	else {
	    fprintf(fp,"P6\n");				/* binary ppm */
	    fprintf(fp,"%d %d\n",width,height);
	    fprintf(fp,"%d\n",depth-1);
	}
    }

    if ( msk == 0 ) {
	int i;
	size_t ii;
	line_buf = (unsigned char *)_eggx_xmalloc(width * 3);
	ii = 0;
	for ( i=0 ; i < height ; i++ ) {
	    unsigned char (*dst)[3] = (unsigned char (*)[3])line_buf;
	    int j;
	    for ( j=0 ; j < width ; j++ ) {
		ii++;
		dst[j][0] = depth * buf[ii++] / 256;
		dst[j][1] = depth * buf[ii++] / 256;
		dst[j][2] = depth * buf[ii++] / 256;
	    }
	    fwrite(line_buf, 1, 3 * width, fp);
	}
    }
    else {
	int i;
	size_t ii;
	line_buf = (unsigned char *)_eggx_xmalloc(width * 4);
	ii = 0;
	for ( i=0 ; i < height ; i++ ) {
	    unsigned char (*dst)[4] = (unsigned char (*)[4])line_buf;
	    int j;
	    for ( j=0 ; j < width ; j++ ) {
		dst[j][0] = depth * buf[ii+1] / 256;
		dst[j][1] = depth * buf[ii+2] / 256;
		dst[j][2] = depth * buf[ii+3] / 256;
		dst[j][3] = depth * buf[ii+0] / 256;
		ii += 4;
	    }
	    fwrite(line_buf, 1, 4 * width, fp);
	}
    }

    fclose(fp);
    fp = NULL;

    rtv = 0;
 quit:
    if ( 0 < Cpid_tmp ) {
	if ( Pc_eggxhandlers_are_set ) {
	    while ( 0 < Cpid_tmp ) {
		eggx_msleep(10);
	    }
	}
	else {
	    int status;
	    while ( wait(&status) != Cpid_tmp );
	    Cpid_tmp = 0;
	}
    }
    if ( fp != NULL ) fclose(fp);
    if ( args != NULL ) free(args);
    if ( tmp1 != NULL ) free(tmp1);
    if ( out_fname != NULL ) free(out_fname);
    if ( line_buf != NULL ) free(line_buf);
    if ( ag_size_vl != NULL ) free(ag_size_vl);
    return rtv;
}

/* 2点の座標 → [始点,大きさ] に変換 */
static void cnv_2p_to_pwh( int wn,
			   double xs, double ys, double xe, double ye,
			   int *r_sx, int *r_sy, int *r_width, int *r_height )
{
    int sx0, sy0, sx1, sy1, width, height;

    /* 座標変換 */
    xyconv(wn, xs,ys, &sx0,&sy0);
    xyconv(wn, xe,ye, &sx1,&sy1);

    /* 座標チェック */
    if ( Pc[wn].wszx <= sx0 ) sx0 = Pc[wn].wszx-1;
    if ( Pc[wn].wszx <= sx1 ) sx1 = Pc[wn].wszx-1;
    if ( Pc[wn].wszy <= sy0 ) sy0 = Pc[wn].wszy-1;
    if ( Pc[wn].wszy <= sy1 ) sy1 = Pc[wn].wszy-1;
    if ( sx0<0 ) sx0 = 0;
    if ( sx1<0 ) sx1 = 0;
    if ( sy0<0 ) sy0 = 0;
    if ( sy1<0 ) sy1 = 0;
    width  = abs(sx1 - sx0) + 1;
    height = abs(sy1 - sy0) + 1;
    if ( sx1 < sx0 ) sx0 = sx1;
    if ( sy1 < sy0 ) sy0 = sy1;

    *r_sx = sx0;
    *r_sy = sy0;
    *r_width = width;
    *r_height = height;

    return;
}

/* バックグラウンドで画像をsaveする */
static int save_img( int wn, int ly, double xs,double ys, double xe,double ye,
	    const char *filter, int depth, const char *argsformat, va_list ap )
{
    char int_2[] = "2";
    char *fn = NULL;
    char *tmp1 = NULL;
    char **args = NULL;
    int sx0, sy0, width, height;
    int j, nargs = 0;
    int rtv = -1;
    char **args_c = NULL;
    char ag2[16], ag3[16], ag4[16], ag5[16], ag6[16], ag7[16], ag8[16];
    char ag9[16], ag10[16], ag11[16], ag12[16], ag13[16], ag14[16], ag15[16];
    char ag16[16];
    pid_t cpid_boot;

    if ( argsformat == NULL ) goto quit;

    if ( ly < 0 || MAX_NLAYER <= ly || Pc[wn].pix[ly] == None ) {
	fprintf(stderr,"EGGX: [ERROR] Invalid layer index at gsaveimage()\n");
	goto quit;
    }
    
    wait_child();

    /* ファイル名を作る */
    fn = _eggx_vasprintf(argsformat,ap);

    /* 2点の座標 → [始点,大きさ] に変換 */
    cnv_2p_to_pwh( wn, xs, ys, xe, ye, &sx0, &sy0, &width, &height );

    if ( filter != NULL && *filter != '\0' ) {		/* コンバートする */
	nargs = create_args(filter, &tmp1, &args);
    } else {
	nargs = 0;
    }

    if ( depth < 2 ) depth = 2;
    else if ( 256 < depth ) depth = 256;

#define N_FIXED 18
    args_c=(char **)_eggx_xmalloc(sizeof(char *)*(N_FIXED+nargs+1)) ;
    sprintf(ag2,"%ld",(long)Pid) ;
    sprintf(ag3,"%lu",Pc_cmap) ;
    sprintf(ag4,"%d",Red_depth) ;
    sprintf(ag5,"%d",Green_depth) ;
    sprintf(ag6,"%d",Blue_depth) ;
    sprintf(ag7,"%d",Red_sft) ;
    sprintf(ag8,"%d",Green_sft) ;
    sprintf(ag9,"%d",Blue_sft) ;
    sprintf(ag10,"%lu",Pc_zwin) ;
    sprintf(ag11,"%lu",Pc[wn].pix[(ly)]) ;
    sprintf(ag12,"%d",sx0) ;
    sprintf(ag13,"%d",sy0) ;
    sprintf(ag14,"%d",width) ;
    sprintf(ag15,"%d",height) ;
    sprintf(ag16,"%d",depth) ;
    args_c[0]=Exec_slave_tmp ;
    args_c[1]=int_2 ;
    args_c[2]=ag2 ;
    args_c[3]=ag3 ;
    args_c[4]=ag4 ;
    args_c[5]=ag5 ;
    args_c[6]=ag6 ;
    args_c[7]=ag7 ;
    args_c[8]=ag8 ;
    args_c[9]=ag9 ;
    args_c[10]=ag10 ;
    args_c[11]=ag11 ;
    args_c[12]=ag12 ;
    args_c[13]=ag13 ;
    args_c[14]=ag14 ;
    args_c[15]=ag15 ;
    args_c[16]=ag16 ;
    args_c[17]=fn ;
    for( j=0 ; j < nargs ; j++ ){
	args_c[N_FIXED+j]=args[j] ;
    }
    args_c[N_FIXED+j]=NULL ;
#undef N_FIXED        

    if ( mkexecfile() == -1 ) {
	fprintf(stderr,"EGGX: [WARNING] Cannot create a file in /tmp/.\n");
	rtv = -1;
	goto quit;
    }
    if ( Exec_fd != -1 ) {
	close(Exec_fd);
	Exec_fd = -1;
    }
    /* */
    XSelectInput( Pc_dis, Pc_zwin, ExposureMask );
    XFlush( Pc_dis );
    XSync( Pc_dis, 0 );
    /* */
    Chld_imgsave_stat = 1;
    if ( (Cpid_saveimg=(cpid_boot=fork())) < 0 ) {
	Chld_imgsave_stat = 0;
	fprintf(stderr,"EGGX: [WARNING] fork() failed.\n");
	rtv = -1;
	goto quit;
    }
    if ( cpid_boot == 0 ) {		/* 子プロセス */
	execv(*args_c,args_c);
	fprintf(stderr,"EGGX: [WARNING] Cannot exec child process.\n");
	_exit(-1) ;
    }
    /* ↓親↓ */
    /* 子から「準備完了」を示すイベントが来るまで待つ */
    while ( 1 ) {
	XEvent ev;
	XNextEvent( Pc_dis, &ev );
	if ( ev.type == GraphicsExpose &&
	     ev.xgraphicsexpose.send_event == True &&
	     ev.xgraphicsexpose.minor_code == MCODE_DUMMY ) break;
    };
    XSelectInput( Pc_dis, Pc_zwin, 0 );
    //restore_xinput_selection(wn);
    /* fprintf(stderr,"DEBUG: child's reply:[GraphicsExpose] is OK\n"); */
    rmexecfile();

    rtv = 0;
 quit:
    if( args_c!=NULL ) free(args_c) ;
    if( args!=NULL ) free(args) ;
    if( tmp1!=NULL ) free(tmp1) ;
    if( fn!=NULL ) free(fn) ;
    return (rtv);
}

/* バックグラウンドで画像をsaveする(C言語) */
int eggx_gsaveimage( int wn, int ly, double xs,double ys, double xe,double ye,
		   const char *filter, int depth, const char *argsformat, ... )
{
    int ret;
    va_list ap;

    va_start(ap, argsformat);
    ret = save_img(wn, ly, xs,ys, xe,ye, filter, depth, argsformat, ap);
    va_end(ap);

    return ret;
}

int eggx_saveimg( int wn, int ly, double xs, double ys, double xe, double ye,
		  const char *filter, int depth, const char *argsformat, ... )
{
    int ret;
    va_list ap;

    va_start(ap, argsformat);
    ret = save_img(wn, ly, xs,ys, xe,ye, filter, depth, argsformat, ap);
    va_end(ap);

    return ret;
}

/* FORTRAN用 saveimg */
void saveimg_( integer *wn, integer *ly, real *xs, real *ys, real *xe, real *ye,
	       char *fname, integer *cnt, char *filter, integer *depth )
{
    char *tmp=NULL,*tmp1,*tmp2 ;
    tmp = _eggx_xstrdup(fname) ;
    tmp1 = strrchr(tmp,'.') ;
    if( tmp1 != NULL ){
	*tmp1 = '\0' ;
	tmp1 = tmp ;
	tmp2 = strrchr(fname,'.') ;
    }
    else{
	tmp1 = tmp ;
	tmp2 = tmp+strlen(tmp) ;
    }
    if( -1 < *cnt ){
	eggx_gsaveimage( *wn, *ly, *xs, *ys, *xe, *ye,
			 filter, *depth, "%s%d%s",tmp1,(int)(*cnt),tmp2 ) ;
    }
    else{
	eggx_gsaveimage( *wn, *ly, *xs, *ys, *xe, *ye,
			 filter, *depth, "%s",fname ) ;
    }
    if( tmp != NULL ) free(tmp) ;
}

#include "_eggx_get_ximage.c"

/* ウィンドゥ wn，レイヤ ly 上にある画像をバッファに取得 */
/* 返り値は画像バッファで，ユーザ側で開放が必要 */
unsigned char *eggx_ggetimage( int wn, int ly, 
			       double xs, double ys, double xe, double ye,
			       int *r_width, int *r_height )
{
    unsigned char *rt_buf = NULL;
    int sx0, sy0, width, height, i, j;
    XImage *image = NULL ;
    Colormap cmap;
    eggx_color_rbuf clbuf;

    if ( ly < 0 || MAX_NLAYER <= ly || Pc[wn].pix[ly] == None ) {
	fprintf(stderr,"EGGX: [ERROR] Invalid layer index at ggetimage()\n");
	goto quit;
    }

    /* 2点の座標 → [始点,大きさ] に変換 */
    cnv_2p_to_pwh( wn, xs, ys, xe, ye, &sx0, &sy0, &width, &height );

    wait_child() ;

    cmap = Pc_cmap /* DefaultColormap( dis, 0 ) */ ;

    XFlush( Pc_dis );
    XSync( Pc_dis, 0 );

    image = get_ximage(Pc_dis, cmap,
		       Red_depth, Green_depth, Blue_depth,
		       Red_sft, Green_sft, Blue_sft,
		       Pc[wn].pix[(ly)], sx0, sy0, width, height, &clbuf);

    rt_buf = (unsigned char *)_eggx_xmalloc(4 * width * height);

    for ( i=0 ; i < height ; i++ ) {
	unsigned char *p = rt_buf + (4 * width * i);
	unsigned char *p4 = p + 4 * width - 1;
	const unsigned char *p3 = p + 3 * width - 1;
	ximage_to_ppmline( Pc_dis, cmap, &clbuf,
			   image, width, height, i, 256, p );
	for ( j=0 ; j < width ; j++ ) {
	    *p4 = *p3;  p4--;  p3--;
	    *p4 = *p3;  p4--;  p3--;
	    *p4 = *p3;  p4--;  p3--;
	    *p4 = 255;  p4--;
	}
    }

 quit:
    if ( image != NULL ) {
	if ( image->data != NULL ) {
	    free( image->data ) ;
	    image->data = NULL ;
	}
	XFree( image ) ;
    }
    if ( rt_buf != NULL ) {
	if ( r_width != NULL ) *r_width = width;
	if ( r_height != NULL ) *r_height = height;
    }
    return rt_buf;
}

void eggx_gputarea( int wn, double x, double y,
	   int src_wn, int src_ly, double xs, double ys, double xe, double ye )
{
    int sx0, sy0, width, height;
    int xx, yy;
    Pixmap src_px = None ;
    XGCValues gv ;

    if (src_ly < 0 || MAX_NLAYER <= src_ly || Pc[src_wn].pix[src_ly] == None) {
	fprintf(stderr,"EGGX: [ERROR] Invalid layer index at gputarea()\n");
	return;
    }

    /* 2点の座標 → [始点,大きさ] に変換 */
    cnv_2p_to_pwh( src_wn, xs, ys, xe, ye, &sx0, &sy0, &width, &height );

    /* dest */
    xyconv( wn, x,y, &xx,&yy );
    if ( (Pc[wn].attributes & BOTTOM_LEFT_ORIGIN) ) yy -= height - 1;

    wait_child();	/* imgsave中は待たせる */
    Ihflg=1;		/* 割り込み禁止 */

    if ( wn == src_wn && Pc[wn].wly == src_ly ) {	/* 自分自身の場合 */
	if ( Pc[wn].tmppix == None ) Pc[wn].tmppix = create_layer(wn);
	/* まず領域部分だけそのまま tmppix にコピー */
	gv.tile = Pc[src_wn].pix[src_ly];
	XChangeGC( Pc_dis, Pc[wn].pxgc, GCTile, &gv );
	XFillRectangle( Pc_dis, Pc[wn].tmppix, Pc[wn].pxgc,
			sx0, sy0, width, height );
	src_px = Pc[wn].tmppix;
    }
    else {
	src_px = Pc[src_wn].pix[src_ly];
    }

    gv.fill_style = FillTiled;
    gv.fill_rule = WindingRule;
    gv.tile = src_px;
    gv.ts_x_origin = xx - sx0;
    gv.ts_y_origin = yy - sy0;
    XChangeGC( Pc_dis, Pc[wn].gc, 
	       GCFillStyle | GCFillRule | GCTile |
	       GCTileStipXOrigin | GCTileStipYOrigin, &gv );
    XFillRectangle( Pc_dis, Pc[wn].pix[Pc[wn].wly], Pc[wn].gc,
		    xx, yy, width, height );
    //fprintf(stderr,"debug: sx0,sy0 = %d,%d\n",sx0,sy0);
    //fprintf(stderr,"debug: xx,yy = %d,%d\n",xx,yy);

    if ( Pc[wn].wly == Pc[wn].sly ) {
	XFillRectangle( Pc_dis, Pc[wn].win, Pc[wn].gc,
			xx, yy, width, height );
	if ( Pc[wn].iconwin != None ) {
	    XFillRectangle( Pc_dis, Pc[wn].iconwin, Pc[wn].gc, 
			    xx, yy, width, height );
	}
	do_auto_flush();
    }

    gv.fill_style = FillSolid;
    gv.ts_x_origin = 0;
    gv.ts_y_origin = 0;
    XChangeGC( Pc_dis, Pc[wn].gc,
	       GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin, &gv );

    Ihflg=0;	/* 割り込み禁止解除 */
    chkexit();

    return;
}

/* ============= イベント関係を扱う関数 ============= */

void eggx_gsetnonblock( int flag )
{
    int i;
    if ( flag != DISABLE ){
	Pc_nonblock = ENABLE ;
	if ( 0 < Cpid ) {
	    send_command_to_child(0, MCODE_NOHANDLE_BTN_EV, None, False);
	    XSelectInput( Pc_dis, Pc_zwin, ExposureMask );
	    send_command_to_child(0, MCODE_NEEDS_REPLY, None, True);
	    /* 子の返事を示すイベントが来るまで待つ */
	    while ( 1 ) {
		XEvent ev;
		XNextEvent( Pc_dis, &ev );
		if ( ev.type == GraphicsExpose &&
		     ev.xgraphicsexpose.send_event == True &&
		     ev.xgraphicsexpose.minor_code == MCODE_DUMMY ) break;
	    };
	    XSelectInput( Pc_dis, Pc_zwin, 0 ) ;
	}
	/* */
	for ( i=0 ; i < N_Pc ; i++ ) {
	    if ( Pc[i].flg != 0 ) restore_xinput_selection( i );
	}
    }
    else {
	Pc_nonblock = DISABLE ;
	for ( i=0 ; i < N_Pc ; i++ ) {
	    if ( Pc[i].flg != 0 ) restore_xinput_selection( i );
	}
	XFlush(Pc_dis);
	if ( 0 < Cpid ) {
	    send_command_to_child(0, MCODE_HANDLE_BTN_EV, None, True);
	}
    }
    return;
}

void gsetnonblock_( integer *flag )
{
    int i_flag = *flag;
    eggx_gsetnonblock(i_flag);
    return;
}

static int handle_xpress_event( const XEvent *ev, int wn,
				int *code_r, double *x_r, double *y_r )
{
    int type = -1, code = -1;
    double x = 0, y = 0;
    char string[8] ;

    if ( ev->type == ButtonPress ) {
	code = ev->xbutton.button ;
	x = (double)(ev->xbutton.x) / Pc[wn].scalex + Pc[wn].acx0 ;
	if ( (Pc[wn].attributes & BOTTOM_LEFT_ORIGIN) ) {
	    y = (double)(Pc[wn].wszy-1-ev->xbutton.y) / Pc[wn].scaley + Pc[wn].acy0 ;
	}
	else {
	    y = (double)(ev->xbutton.y) / Pc[wn].scaley + Pc[wn].acy0 ;
	}
	type = ButtonPress ;
    }
    else if ( ev->type == KeyPress ) {
	KeySym key;
	XKeyEvent xkev = ev->xkey;
	unsigned int msk = Pc[wn].sbarkeymask;
	Bool is_scroll_bar = ( (xkev.state & msk) == msk );
	XLookupString(&xkev,string,sizeof(string),&key,NULL) ;
	/* printf("[%X]",key) ;  fflush(stdout) ; */
	switch ( key ) {
	case XK_Right:		/* カーソルキーを 1c〜1fに割り当てる */
	    if ( is_scroll_bar ) break;
	    code = 0x01c ;
	    break ;
	case XK_KP_Right:
	    if ( is_scroll_bar ) break;
	    code = 0x01c ;
	    break ;
	case XK_Left:
	    if ( is_scroll_bar ) break;
	    code = 0x01d ;
	    break ;
	case XK_KP_Left:
	    if ( is_scroll_bar ) break;
	    code = 0x01d ;
	    break ;
	case XK_Up:
	    if ( is_scroll_bar ) break;
	    code = 0x01e ;
	    break ;
	case XK_KP_Up:
	    if ( is_scroll_bar ) break;
	    code = 0x01e ;
	    break ;
	case XK_Down:
	    if ( is_scroll_bar ) break;
	    code = 0x01f ;
	    break ;
	case XK_KP_Down:
	    if ( is_scroll_bar ) break;
	    code = 0x01f ;
	    break ;
	case XK_Page_Up:		/* PageUp,PageDown を Ctrl+B,Ctrl+F に割り当てる */
	    if ( is_scroll_bar ) break;
	    code = 0x002 ;
	    break ;
	case XK_KP_Page_Up:
	    if ( is_scroll_bar ) break;
	    code = 0x002 ;
	    break ;
	case XK_Page_Down:
	    if ( is_scroll_bar ) break;
	    code = 0x006 ;
	    break ;
	case XK_KP_Page_Down:
	    if ( is_scroll_bar ) break;
	    code = 0x006 ;
	    break ;
	case XK_Home:		/* Home,End を Ctrl+A,Ctrl+E に割り当てる */
	    if ( is_scroll_bar ) break;
	    code = 0x001 ;
	    break ;
	case XK_KP_Home:
	    if ( is_scroll_bar ) break;
	    code = 0x001 ;
	    break ;
	case XK_End:
	    if ( is_scroll_bar ) break;
	    code = 0x005 ;
	    break ;
	case XK_KP_End:
	    if ( is_scroll_bar ) break;
	    code = 0x005 ;
	    break ;
	case XK_Pause:		/* Pause を Ctrl+C に割り当てる */
	    code = 0x003 ;
	    break ;
	case XK_KP_Delete:
	    code = 0x07f ;
	    break ;
	default:
	    code = string[0] ;
	    break ;
	}
	if ( 0 < code ) {
	    type = KeyPress ;
	}
    }

    if ( 0 <= type ) {
	*code_r = code;
	*x_r = x;
	*y_r = y;
    }

    return type;
}

#if 0	/* 作ったけど必要なくなった… */
/*
 * ウィンドゥマネージャが介入すると，XQueryPointer は必ずしも目的の window を
 * 返さず，その親を返す事があるので，子の中に目的の窓があるかを探す．
 */
Bool find_child_window( Window parent_win, Window to_find )
{
    Bool ret = False;
    Window r_win, p_win;
    Window *c_win_list = NULL;
    unsigned int n_c_win, j;
    if ( parent_win == None ) goto quit;	/* ← Cywgin だとよく起こる */
    if ( parent_win == to_find ) {
	ret = True;
	goto quit;
    }
    XQueryTree(Pc_dis, parent_win, &r_win, &p_win, &c_win_list, &n_c_win);
    for ( j=0 ; j < n_c_win ; j++ ) {
	/* fprintf(stderr,"debug: loop: [%lx]\n",c_win_list[j]); */
	if ( find_child_window(c_win_list[j], to_find) == True ) {
	    ret = True;
	    goto quit;
	}
    }
 quit:
    if ( c_win_list != NULL ) XFree(c_win_list);
    return ret;
}
#endif

/* XFree86 以外の X サーバでは，マトモにイベントが来ないので */
/* XNextEvent の前に Pointer の位置の変化を自前でチェックする */
/* (XFree86 でもボタン等のイベントはちゃんと来ない．仕様といえば仕様だが) */
static void check_mouse( Bool ck_button, Bool ck_motion, XEvent *ev )
{
    Bool tf;
    Window r_win, ptr_win;
    int r_x0, r_y0, r_x, r_y, w_x, w_y, current_wid = -1;
    unsigned int msk0, msk = 0;
    ev->type = None;
    do {
	/* RootWindow から順に探していく */
	Window ptr_win0 = RootWindow(Pc_dis,0);
	ptr_win = ptr_win0;
	tf = XQueryPointer(Pc_dis, ptr_win0,
			   &r_win, &ptr_win0,
			   &r_x, &r_y, &w_x, &w_y, &msk0);
	r_x0 = r_x;
	r_y0 = r_y;
	if ( tf == True ) {
	    msk |= msk0;
	    while ( ptr_win0 != None ) {
		ptr_win = ptr_win0;
		tf = XQueryPointer(Pc_dis, ptr_win0,
				   &r_win, &ptr_win0,
				   &r_x, &r_y, &w_x, &w_y, &msk0);
		if ( tf == False ) break;
		msk |= msk0;
		if ( r_x != r_x0 || r_y != r_y0 ) {
		    /* やりなおし */
		    ptr_win0 = RootWindow(Pc_dis,0);
		    r_x0 = r_x;
		    r_y0 = r_y;
		}
	    }
	    if ( ptr_win != RootWindow(Pc_dis,0) ) {
		int i;
		for ( i=0 ; i < N_Pc ; i++ ) {
		    if ( Pc[i].flg != 0 ) {
			if ( ptr_win == Pc[i].win ) {
			    current_wid = i;
			    break;
			}
		    }
		    /*
		      if (find_child_window(ptr_win, Pc[i].win)==True) {
		      current_wid = i;
		      break;
		      }
		    */
		}
	    }
	}
	if ( current_wid < 0 || tf == False ) {
	    /* マウスポインタは明後日のところにいる */
	    if ( 0 <= Pc_evrec_wid ) {
		if ( ck_motion == True ) {
		    if ( tf == True ) {
			tf = XQueryPointer(Pc_dis, Pc[Pc_evrec_wid].win,
					   &r_win, &ptr_win,
					   &r_x, &r_y, &w_x, &w_y, &msk0);
			if ( tf == False ) continue;
		    }
		    else {
			msk0 = 0;
			w_x = -32000;  w_y = -32000;
			r_x = -32000;  r_y = -32000;
		    }
		    ev->xany.window = Pc[Pc_evrec_wid].win;
		    ev->type = LeaveNotify;
		    ev->xcrossing.state = msk0;
		    ev->xcrossing.x = w_x;
		    ev->xcrossing.y = w_y;
		    ev->xcrossing.x_root = r_x;
		    ev->xcrossing.y_root = r_y;
		    break;
		}
	    }
	}
	else {	/* 0 <= current_wid */
	    if ( ck_button == True ) {
		if ( Pc_evrec_button_mask == 0 &&
		     (msk & Button_12345_Mask) != 0 ) {
		    tf = XQueryPointer(Pc_dis, Pc[current_wid].win,
				       &r_win, &ptr_win,
				       &r_x, &r_y, &w_x, &w_y, &msk0);
		    if ( tf == False ) continue;
		    if ( r_x != r_x0 || r_y != r_y0 ) continue;
		    ev->type = ButtonPress;
		    ev->xany.window = Pc[current_wid].win;
		    if ( msk & Button1Mask ) ev->xbutton.button = 1;
		    else if ( msk & Button2Mask ) ev->xbutton.button = 2;
		    else if ( msk & Button3Mask ) ev->xbutton.button = 3;
		    else if ( msk & Button4Mask ) ev->xbutton.button = 4;
		    else if ( msk & Button5Mask ) ev->xbutton.button = 5;
		    else ev->xbutton.button = 0;
		    ev->xbutton.x = w_x;
		    ev->xbutton.y = w_y;
		    ev->xbutton.x_root = r_x;
		    ev->xbutton.y_root = r_y;
		    break;
		}
		else if ( (Pc_evrec_button_mask != 0 &&
			   (msk & Button_12345_Mask) == 0) ) {
		    tf = XQueryPointer(Pc_dis, Pc[current_wid].win,
				       &r_win, &ptr_win,
				       &r_x, &r_y, &w_x, &w_y, &msk0);
		    if ( tf == False ) continue;
		    if ( r_x != r_x0 || r_y != r_y0 ) continue;
		    ev->type = ButtonRelease;
		    ev->xany.window = Pc[current_wid].win;
		    msk = Pc_evrec_button_mask;
		    if ( msk & Button1Mask ) ev->xbutton.button = 1;
		    else if ( msk & Button2Mask ) ev->xbutton.button = 2;
		    else if ( msk & Button3Mask ) ev->xbutton.button = 3;
		    else if ( msk & Button4Mask ) ev->xbutton.button = 4;
		    else if ( msk & Button5Mask ) ev->xbutton.button = 5;
		    else ev->xbutton.button = 0;
		    ev->xbutton.x = w_x;
		    ev->xbutton.y = w_y;
		    ev->xbutton.x_root = r_x;
		    ev->xbutton.y_root = r_y;
		    break;
		}
	    }
	    /* */
	    if ( ck_motion == True ) {
		if ( Pc_evrec_wid == current_wid ) {
		    tf = XQueryPointer(Pc_dis, Pc[current_wid].win,
				       &r_win, &ptr_win,
				       &r_x, &r_y, &w_x, &w_y, &msk0);
		    if ( tf == False ) continue;
		    if ( r_x != r_x0 || r_y != r_y0 ) continue;
		    if ( Pc_evrec_pointer_x != r_x ||
			 Pc_evrec_pointer_y != r_y ) {
			ev->type = MotionNotify;
			ev->xany.window = Pc[current_wid].win;
			ev->xmotion.state = msk0;
			ev->xmotion.x = w_x;
			ev->xmotion.y = w_y;
			ev->xmotion.x_root = r_x;
			ev->xmotion.y_root = r_y;
			break;
		    }
		}
		else {
		    if ( 0 <= Pc_evrec_wid ) {
			tf = XQueryPointer(Pc_dis, Pc[Pc_evrec_wid].win,
					   &r_win, &ptr_win,
					   &r_x, &r_y, &w_x, &w_y, &msk0);
			if ( tf == False ) continue;
			if ( r_x != r_x0 || r_y != r_y0 ) continue;
			ev->type = LeaveNotify;
			ev->xany.window = Pc[Pc_evrec_wid].win;
			ev->xcrossing.state = msk0;
			ev->xcrossing.x = w_x;
			ev->xcrossing.y = w_y;
			ev->xcrossing.x_root = r_x;
			ev->xcrossing.y_root = r_y;
			break;
		    }
		    else {
			tf = XQueryPointer(Pc_dis, Pc[current_wid].win,
					   &r_win, &ptr_win,
					   &r_x, &r_y, &w_x, &w_y, &msk0);
			if ( tf == False ) continue;
			if ( r_x != r_x0 || r_y != r_y0 ) continue;
			ev->type = EnterNotify;
			ev->xany.window = Pc[current_wid].win;
			ev->xcrossing.state = msk0;
			ev->xcrossing.x = w_x;
			ev->xcrossing.y = w_y;
			ev->xcrossing.x_root = r_x;
			ev->xcrossing.y_root = r_y;
			break;
		    }
		}
	    }
	}
    } while ( 0 );
}

int eggx_ggetevent( int *type_r, int *code_r, double *x_r, double *y_r )
{
    int wn = -1;
    int type = -1, code = -1;
    //int type_x = None;
    double x = 0, y = 0;
    int i;

    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;	/* 割り込み禁止 */

    if ( Pc_nonblock == DISABLE ) {
	/* ブロッキングモードでは，マウスのボタンイベントは子から受ける */
	XSelectInput( Pc_dis, Pc_zwin, ButtonPressMask | ButtonReleaseMask );
	for ( i=0 ; i < N_Pc ; i++ ) {
	    if ( Pc[i].flg != 0 ) {
		if ( Pc[i].pwin != Pc[i].win ) {
		    XSelectInput( Pc_dis, Pc[i].pwin, KeyPressMask );
		    XSelectInput( Pc_dis, Pc[i].win, PointerMotionMask |
				            LeaveWindowMask | EnterWindowMask);
		}
		else {
		    XSelectInput( Pc_dis, Pc[i].win, KeyPressMask |
			PointerMotionMask | LeaveWindowMask | EnterWindowMask);
		}
	    }
	}
    }

    while ( 1 ) {
	XEvent ev;
	if ( Pc_nonblock != DISABLE ) {
	    Bool tf = XCheckMaskEvent(Pc_dis,
		   KeyPressMask | ButtonPressMask | ButtonReleaseMask |
		   PointerMotionMask | LeaveWindowMask | EnterWindowMask, &ev);
	    if ( tf == False ) break;
	}
	else {
	    ev.type = None;
	    /* XFree86 以外の X サーバでは，マトモにイベントが来ないので */
	    /* XNextEvent の前に Pointer の位置の変化を自前でチェックし， */
	    /* ハリボテイベントを作成する */
	    check_mouse(True, True, &ev);
	    /* */
	    if ( ev.type == None ) {
		XNextEvent( Pc_dis, &ev );
	    }
	    else {
		XEvent junk_ev;
		/* 自前でマウスポインタの位置を調べた場合はキューをクリア */
		while ( XCheckMaskEvent( Pc_dis, 
					 PointerMotionMask | LeaveNotify |
					 EnterWindowMask | ButtonPressMask |
					 ButtonReleaseMask,
					 &junk_ev) == True );
	    }
	}
	for ( i=0 ; i < N_Pc ; i++ ) {
	    if ( Pc[i].flg != 0 ) {
		if ( ev.xany.window == Pc[i].win ) break;
		if ( ev.xany.window == Pc[i].pwin ) break;
	    }
	}
	if ( N_Pc <= i ) continue;
	else wn = i;
	/* */
	if ( ev.type == MotionNotify ) {
	    if ( (ev.xmotion.state & Button1Mask) != 0 ) code = 1;
	    else if ( (ev.xmotion.state & Button2Mask) != 0 ) code = 2;
	    else if ( (ev.xmotion.state & Button3Mask) != 0 ) code = 3;
	    else if ( (ev.xmotion.state & Button4Mask) != 0 ) code = 4;
	    else if ( (ev.xmotion.state & Button5Mask) != 0 ) code = 5;
	    else code = 0 ;
	    x = (double)(ev.xmotion.x) / Pc[wn].scalex + Pc[wn].acx0 ;
	    if ( (Pc[wn].attributes & BOTTOM_LEFT_ORIGIN) ) {
		y = (double)(Pc[wn].wszy-1-ev.xmotion.y) / Pc[wn].scaley + Pc[wn].acy0 ;
	    }
	    else {
		y = (double)(ev.xmotion.y) / Pc[wn].scaley + Pc[wn].acy0 ;
	    }
	    type = MotionNotify ;
    	    if ( Pc_nonblock == DISABLE ) {
		Pc_evrec_wid = wn;
		Pc_evrec_pointer_x = ev.xmotion.x_root;
		Pc_evrec_pointer_y = ev.xmotion.y_root;
	    }
	    break ;
	}
	else if ( ev.type == LeaveNotify || ev.type == EnterNotify ) {
	    if ( (ev.xcrossing.state & Button1Mask) != 0 ) code = 1;
	    else if ( (ev.xcrossing.state & Button2Mask) != 0 ) code = 2;
	    else if ( (ev.xcrossing.state & Button3Mask) != 0 ) code = 3;
	    else if ( (ev.xcrossing.state & Button4Mask) != 0 ) code = 4;
	    else if ( (ev.xcrossing.state & Button5Mask) != 0 ) code = 5;
	    else code = 0 ;
	    x = (double)(ev.xcrossing.x) / Pc[wn].scalex + Pc[wn].acx0 ;
	    if ( (Pc[wn].attributes & BOTTOM_LEFT_ORIGIN) ) {
		y = (double)(Pc[wn].wszy-1-ev.xcrossing.y) / Pc[wn].scaley + Pc[wn].acy0 ;
	    }
	    else {
		y = (double)(ev.xcrossing.y) / Pc[wn].scaley + Pc[wn].acy0 ;
	    }
	    type = ev.type ;
    	    if ( Pc_nonblock == DISABLE ) {
		if ( ev.type == LeaveNotify ) {
		    /* すでにLeaveしているかチェック */
		    if ( Pc_evrec_wid < 0 ) continue;
		    else Pc_evrec_wid = -1;
		}
		else {
		    /* すでにEnterしているかチェック */
		    if ( Pc_evrec_wid == wn ) type = MotionNotify;
		    else Pc_evrec_wid = wn;
		}
		Pc_evrec_pointer_x = ev.xcrossing.x_root;
		Pc_evrec_pointer_y = ev.xcrossing.y_root;
	    }
	    break ;
	}
	else if ( ev.type == ButtonRelease ) {
	    if ( Pc_nonblock == DISABLE ) {
		Pc_evrec_button_mask = 0;
	    }
	    /* ButtonRelease は報告せず，ループの先頭へ */
	    continue;
	}
	else {
	    int tp = handle_xpress_event( &ev, wn, &code, &x, &y );
	    if ( 0 <= tp ) {
		type = tp;
		if ( Pc_nonblock == DISABLE ) {
		   if ( type == ButtonPress ) {
		      if (code == 1) Pc_evrec_button_mask = Button1Mask;
		      else if (code == 2) Pc_evrec_button_mask = Button2Mask;
		      else if (code == 3) Pc_evrec_button_mask = Button3Mask;
		      else if (code == 4) Pc_evrec_button_mask = Button4Mask;
		      else if (code == 5) Pc_evrec_button_mask = Button5Mask;
		      else Pc_evrec_button_mask = 0;
		      /* */
		      if ( Pc_evrec_wid != wn ) {
			  /* まだEnterしてない場合は報告せずループの先頭へ */
			  continue;
		      }
		   }
		}
		break;
	    }
	}
    }	/* while(1)... */

    if ( Pc_nonblock == DISABLE ) {
	for ( i=0 ; i < N_Pc ; i++ ) {
	    if ( Pc[i].flg != 0 ) {
		if ( Pc[i].pwin != Pc[i].win ) {
		    XSelectInput( Pc_dis, Pc[i].pwin, 0 );
		    XSelectInput( Pc_dis, Pc[i].win, 0 );
		}
		else {
		    XSelectInput( Pc_dis, Pc[i].win, 0 );
		}
	    }
	}
	/* 子からのボタンイベントの入力を閉じる */
	XSelectInput( Pc_dis, Pc_zwin, 0 );
    }

    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;

    if ( 0 <= wn ) {
	if ( type_r != NULL ) *type_r = type;
	if ( code_r != NULL ) *code_r = code;
	if ( x_r != NULL ) *x_r = x;
	if ( y_r != NULL ) *y_r = y;
    }

    return( wn ) ;
}

int eggx_ggeteventf( int *type_r, int *code_r, float *x_r, float *y_r )
{
    double x, y;
    int rt;
    rt = eggx_ggetevent(type_r, code_r, &x, &y);
    if ( x_r != NULL ) *x_r = x;
    if ( y_r != NULL ) *y_r = y;
    return rt;
}

void ggetevent_( integer *wn, integer *type, integer *code, real *x, real *y )
{
    double xx ;
    double yy ;
    int i_code ;
    int i_type ;
    *wn = (integer)eggx_ggetevent(&i_type,&i_code,&xx,&yy) ;
    *type = (integer)i_type ;
    *code = (integer)i_code ;
    *x = (real)xx ;
    *y = (real)yy ;
}

int eggx_ggetxpress( int *type_r, int *code_r, double *x_r, double *y_r )
{
    int wn = -1;
    int type = -1, code = -1;
    double x = 0, y = 0;
    int i;

    wait_child() ;	/* imgsave中は待たせる */
    Ihflg=1 ;		/* 割り込み禁止 */

    if ( Pc_nonblock == DISABLE ) {
	/* ブロッキングモードでは，マウスのボタンイベントは子から受ける */
	XSelectInput( Pc_dis, Pc_zwin, ButtonPressMask | ButtonReleaseMask );
	for ( i=0 ; i < N_Pc ; i++ ) {
	    if ( Pc[i].flg != 0 ) {
		if ( Pc[i].pwin != Pc[i].win ) {
		    XSelectInput( Pc_dis, Pc[i].pwin, KeyPressMask );
		}
		else {
		    XSelectInput( Pc_dis, Pc[i].win, KeyPressMask );
		}
	    }
	}
    }

    while ( 1 ) {
	XEvent ev;
	if ( Pc_nonblock != DISABLE ) {
	    Bool tf = XCheckMaskEvent(Pc_dis,
			   KeyPressMask | ButtonPressMask | PointerMotionMask
			   | LeaveWindowMask | EnterWindowMask, &ev);
	    if ( tf == False ) break;
	}
	else {
	    XNextEvent( Pc_dis, &ev );
	}
	for ( i=0 ; i < N_Pc ; i++ ) {
	    if ( Pc[i].flg != 0 ) {
		if ( ev.xany.window == Pc[i].win ) break;
		if ( ev.xany.window == Pc[i].pwin ) break;
	    }
	}
	if ( N_Pc <= i ) continue;
	else wn = i;
	{
	    int tp = handle_xpress_event( &ev, wn, &code, &x, &y );
	    if ( 0 <= tp ) {
		type = tp;
		break;
	    }
	}
    }	/* while(1)... */

    if ( Pc_nonblock == DISABLE ) {
	for ( i=0 ; i < N_Pc ; i++ ) {
	    if ( Pc[i].flg != 0 ) {
		if ( Pc[i].pwin != Pc[i].win ) {
		    XSelectInput( Pc_dis, Pc[i].pwin, 0 );
		}
		else {
		    XSelectInput( Pc_dis, Pc[i].win, 0 );
		}
	    }
	}
	/* 子からのボタンイベントを閉じる */
	XSelectInput( Pc_dis, Pc_zwin, 0 );
    }

    Ihflg=0 ;	/* 割り込み禁止解除 */
    chkexit() ;

    if ( 0 <= wn ) {
	if ( type_r != NULL ) *type_r = type;
	if ( code_r != NULL ) *code_r = code;
	if ( x_r != NULL ) *x_r = x;
	if ( y_r != NULL ) *y_r = y;
    }

    return( wn ) ;
}

int eggx_ggetxpressf( int *type_r, int *code_r, float *x_r, float *y_r )
{
    double x, y;
    int rt;
    rt = eggx_ggetxpress(type_r, code_r, &x, &y);
    if ( x_r != NULL ) *x_r = x;
    if ( y_r != NULL ) *y_r = y;
    return rt;
}

void ggetxpress_( integer *wn, integer *type, integer *code, real *x, real *y )
{
    double xx ;
    double yy ;
    int i_code ;
    int i_type ;
    *wn = (integer)eggx_ggetxpress(&i_type,&i_code,&xx,&yy) ;
    *type = (integer)i_type ;
    *code = (integer)i_code ;
    *x = (real)xx ;
    *y = (real)yy ;
}

int eggx_ggetch( void )
{
    int type, rt = -1 ;
    double x,y ;
    while ( 1 ) {
	int win = eggx_ggetxpress(&type,&rt,&x,&y);
	if ( Pc_nonblock != DISABLE ) {
	    if ( win < 0 ) {
		rt = -1;
		break;
	    }
	    if ( type == KeyPress ) break;
	}
	else {
	    if ( type == KeyPress ) break;
	}
    }
    return(rt) ;
}

void ggetch_( integer *rt )
{
    *rt = (integer)eggx_ggetch() ;
}

void inkeydollar_( integer *rt )
{
    *rt = (integer)eggx_ggetch() ;
}

void eggx_gsetscrollbarkeymask( int wn, unsigned int key_mask )
{
    if ( Pc[wn].attributes & SCROLLBAR_INTERFACE ) {
	Pc[wn].sbarkeymask = key_mask;
	/* キーマスク情報を子に送信 */
	send_command_to_child(wn, MCODE_KEYMASK, Pc[wn].sbarkeymask, True);
    }
    return;
}

void gsetscrollbarkeymask_( integer *wn, integer *key_mask )
{
    eggx_gsetscrollbarkeymask( *wn, (unsigned int)(*key_mask) );
    return;
}

/* ============= オマケ的な関数 ============= */

void eggx_msleep( unsigned long msec )
{
#ifndef NO_USLEEP
    unsigned long t ;
    t=(unsigned long)(msec*1000) ;
    usleep( t ) ;
#else
    int rt ;
    struct timeval delay ;
    delay.tv_sec = (msec*1000) / 1000000L ;
    delay.tv_usec = (msec*1000) % 1000000L ;
    rt = select(0,
		(fd_set *) NULL,
		(fd_set *) NULL,
		(fd_set *) NULL,
		&delay) ;
    if( rt == -1 ){
        /* perror("select") ; */
        /* return(rt) ; */
	return ;
    }
    /* return(rt) ; */
    return ;
#endif	/* NO_USLEEP */
}

void msleep_( integer *time )
{
    eggx_msleep( (unsigned long)(*time) ) ;
}

/* ============= ここからはFORTRANの不便さを解消するための関数 ============= */

void isnan_( real *value, integer *flag )
{
    int rtf ;
    rtf = isnan( *value ) ;
    rtf |= ISINF( *value ) ;
    *flag = (integer)rtf ;
}

/* ============= 互換性用 : 将来削除予定 ============= */

#if 0
void xwindow_( integer *wn, real *xs, real *ys, real *xe, real *ye )
{
    eggx_window( *wn, *xs, *ys, *xe, *ye ) ;
}

void xnewpen_( integer *wn, integer *n )
{
    eggx_newpen( *wn, *n ) ;
}

void xclsx_( integer *wn )
{
    eggx_gclr( *wn ) ;
}

void xplot_( integer *wn, real *xg, real *yg, integer *mode )
{
    eggx_line( *wn, *xg, *yg, *mode ) ;
}

void xarohd_( integer *wn, real *xs, real *ys, real *xt, real *yt,
	      real *s, real *w, integer *shape )
{
    eggx_drawarrow( *wn, *xs, *ys, *xt, *yt, 
		    *s, *w, *shape ) ;    
}

void xnumber_( integer *wn, real *xg, real *yg, real *size, 
	       real *v, real *theta, integer *n )
{
    char *adr ;
    integer len ;
    adr = rtoc( v, n ) ;
    len = strlen(adr) ;
    drawstr_( wn, xg, yg, size, adr, theta, &len ) ;
}

int eggx_depthinfo( void )
{
    int d,w,h,f ;
    f=eggx_ggetdisplayinfo(&d,&w,&h) ;
    if( f ) return(f) ;
    else return(d) ;
}

void depthinfo_( integer *rt )
{
    *rt = (integer)eggx_depthinfo() ;
}

void eggx_drawnum( int wn, double x, double y, int size, double v, double theta, int n )
{
    integer w,nn ;
    real xx,yy,sz,vv,ttheta ;
    w=(integer)wn ;
    xx=(real)x ;
    yy=(real)y ;
    sz=(real)size ;
    vv=(real)v ;
    ttheta=(real)theta ;
    nn=(integer)n ;
    drawnum_(&w,&xx,&yy,&sz,&vv,&ttheta,&nn) ;
}
#endif	/* #if 0 */
