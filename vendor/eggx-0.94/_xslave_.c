/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-03-16 01:28:24 cyamauch> */

#include <X11/Xlib.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

#include "_xslave_.h"
#include "_eggx_scrollbar.h"

struct pctg {
    int flg ;			/* 有効かどうかのフラグ */
    Window pwin ;		/* 親 Window ID */
    Window clipwin ;		/* 描画Window を載せるための子窓 */
    Window hsbarwin ;		/* 水平スクロールバーのための小窓 */
    Window vsbarwin ;		/* 垂直スクロールバーのための小窓 */
    Window win ;		/* 描画Window (scrollbar無し時はpwinと同じ) */
    Window iconwin ;		/* Icon Window ID */
    XWindowAttributes pwin_att;
    XWindowAttributes clipwin_att;
    XWindowAttributes win_att;
    int bl_origin;
    Pixmap pix ;		/* Pixmap ID */
    eggx_scrollbar vbar;
    eggx_scrollbar hbar;
    GC bar_gc;
} ;

static volatile pid_t Cpid=0 ;
static pid_t Ppid ;

static Display* Dis ;

static struct pctg *Pc = NULL ;		/* ウィンドゥ管理用rec */
static int N_Pc = 0 ;			/* Pc のバッファの個数 */

static int Scrollbar_width = 19 ;

static void _m_sleep( unsigned long msec )
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

/* Only for saveimg */
static void chldexithandler( int dummy )
{
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    if ( pid == Cpid ) Cpid = 0;
    return;
}

#include "_eggx_get_ximage.c"

/* 画像をsaveする */
static int imgsv( char **argv )
{
    int rt_val = 1;
    int i, sx0, sy0, width, height, depth;
    int red_depth, green_depth, blue_depth;
    int red_sft, green_sft, blue_sft;
    int imagc_argc = 0;
    char *fn;
    char **fargv;
    char ppm_stdin[] = "PPM:-";
#ifdef __CYGWIN__
    char str_convert_exe[12] = "convert.exe";
#endif
    unsigned char *line_buf = NULL;
    char **imagc_argv = NULL;
    /* struct stat *file_stat ; */
    FILE *fp = NULL;
    Window pc_zwin;
    Pixmap pix_id;
    XImage *image = NULL;
    eggx_color_rbuf clbuf;
    Colormap cmap;
    XGraphicsExposeEvent geev;

    cmap = strtoul(*++argv, (char **)NULL, 10);
    red_depth = atoi(*++argv);
    green_depth = atoi(*++argv);
    blue_depth = atoi(*++argv);
    red_sft = atoi(*++argv);
    green_sft = atoi(*++argv);
    blue_sft = atoi(*++argv);
    pc_zwin = strtoul(*++argv, (char **)NULL, 10);
    pix_id = strtoul(*++argv, (char **)NULL, 10);
    sx0 = atoi(*++argv);
    sy0 = atoi(*++argv);
    width = atoi(*++argv);
    height = atoi(*++argv);
    depth = atoi(*++argv);
    fn = *++argv;
    fargv = ++argv;

    /* 準備が完了した事を親に送信 */
    memset(&geev, 0, sizeof(geev));
    geev.type = GraphicsExpose;
    geev.send_event = True;
    geev.display = Dis;
    geev.drawable = None;
    geev.minor_code = MCODE_DUMMY;
    /* fprintf(stderr,"debug: sending [GraphicsExpose]\n"); */
    XSendEvent( Dis, pc_zwin, False, ExposureMask, (XEvent *)&geev);
    XFlush( Dis );

    if ( fargv[0] == NULL ) {				/* ppm */
	fp = fopen(fn, "wb");
	if ( fp == NULL ) {
	    fprintf(stderr,"eggx: [ERROR] Cannot create a file.\n");
	    goto quit;
	}
    }
    else if ( strcmp(fargv[0],"convert") == 0 ||	/* ImageMagick */
	      strcmp(fargv[0],"convert.exe") == 0 ) {
	int pfds[2];
	/* */
	for ( i=0 ; fargv[i] != NULL ; i++ );
	imagc_argc = i + 2;
	/* */
	imagc_argv = (char **)malloc(sizeof(char *)*(imagc_argc+1));
	if ( imagc_argv == NULL ) {
	    fprintf(stderr,"eggx: [ERROR] malloc() failed.\n");
	    goto quit ;
	}
	for ( i=0 ; i < imagc_argc-2 ; i++ ) imagc_argv[i] = fargv[i];
	imagc_argv[i++] = ppm_stdin;
	imagc_argv[i++] = fn;
	imagc_argv[i++] = NULL;
#ifdef __CYGWIN__
	imagc_argv[0] = str_convert_exe;
#endif
	/* コンバータを起動する */
	if ( pipe(pfds) < 0 ) {
	    fprintf(stderr,"eggx: [ERROR] pipe() failed.\n");
	    goto quit;
	}
	if ( (Cpid=fork()) < 0 ) {
	    fprintf(stderr,"eggx: [ERROR] fork() failed.\n");
	    close( pfds[1] );
	    close( pfds[0] );
	    goto quit;
	}
	if ( Cpid == 0 ) {		/* 子プロセス */
	    dup2( pfds[0], 0 );
	    close( pfds[1] );
	    close( pfds[0] );
	    execvp( imagc_argv[0], imagc_argv );
	    fprintf(stderr,"eggx: [ERROR] Cannot exec '%s'.\n",imagc_argv[0]);
	    if ( Ppid == getppid() ) {
		kill(Ppid, SIGTERM);
	    }
	    _exit(-1);
	}
	close(pfds[0]);
	fp = fdopen( pfds[1], "wb" );	/* 送信用 */
	if ( fp == NULL ) {
	    fprintf(stderr,"eggx: [ERROR] Pipe connection failed.\n");
	    close( pfds[1] );
	    goto quit;
	}
    }
    else {						/* netppm */
	int fd;
	int pfds[2];
	/* fileのcreate */
	fd = open(fn,O_WRONLY|O_CREAT|O_TRUNC,
		  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );
	if ( fd == -1 ) {
	    fprintf(stderr,"eggx: [ERROR] Cannot create a file.\n");
	    goto quit;
	}
	/* コンバータを起動する */
	if ( pipe(pfds) < 0 ) {
	    fprintf(stderr,"eggx: [ERROR] pipe() failed.\n");
	    close(fd);
	    goto quit;
	}
	if ( (Cpid=fork()) < 0 ) {
	    fprintf(stderr,"eggx: [ERROR] fork() failed.\n");
	    close(fd);
	    close( pfds[1] );
	    close( pfds[0] );
	    goto quit;
	}
	if ( Cpid == 0 ) {		/* 子プロセス */
	    dup2( fd, 1 );
	    dup2( pfds[0], 0 );
	    close( pfds[1] );
	    close( pfds[0] );
	    execvp( fargv[0], fargv );
	    fprintf(stderr,"eggx: [ERROR] Cannot exec '%s'.\n",fargv[0]);
	    if ( Ppid == getppid() ) {
		kill(Ppid, SIGTERM);
	    }
	    _exit(-1);
	}
	close(fd);
	close(pfds[0]);
	fp = fdopen( pfds[1], "wb" );	/* 送信用 */
	if ( fp == NULL ) {
	    fprintf(stderr,"eggx: [ERROR] Pipe connection failed.\n");
	    close( pfds[1] );
	    goto quit;
	}
    }

    image = get_ximage(Dis, cmap, 
		       red_depth, green_depth, blue_depth,
		       red_sft, green_sft, blue_sft,
		       pix_id, sx0, sy0, width, height, &clbuf);

    /*
    fprintf(stderr,"debug: bits_per_pixel = %d\n",image->bits_per_pixel) ;
    fprintf(stderr,"debug: bytes_per_line = %d\n",image->bytes_per_line) ;
    fprintf(stderr,"debug: width*wd = %d\n",width*wd) ;
    */

    line_buf = (unsigned char *)malloc(width * 3) ;
    if ( line_buf == NULL ) {
	fprintf(stderr,"eggx: [ERROR] malloc() failed.\n");
	goto quit;
    }

    fprintf(fp,"P6\n") ;
    fprintf(fp,"%d %d\n",width,height) ;
    fprintf(fp,"%d\n",depth-1) ;

    /* ppmを作成 */

    /*
    if ( image->byte_order == MSBFirst )
	fprintf(stderr,"debug: This Display is MSBFirst\n");
    else
	fprintf(stderr,"debug: This Display is LSBFirst\n");
    */

    for ( i=0 ; i < height ; i++ ) {
	ximage_to_ppmline(Dis, cmap, &clbuf, image, width, height, i, depth,
			  line_buf);
	fwrite(line_buf, 1, 3 * width, fp);
    }
    fclose(fp);
    fp = NULL;

    if ( fargv[0] != NULL ) {		/* netpbm or ImageMagick */
	while ( 0 < Cpid ) {
	    _m_sleep(10);
	}
    }

    rt_val = 0;
 quit:
    if ( fp != NULL ) fclose(fp);
    if ( image != NULL ) {
	if ( image->data != NULL ) {
	    free( image->data ) ;
	    image->data = NULL ;
	}
	XFree( image ) ;
    }
    if ( imagc_argv != NULL ) free(imagc_argv) ;
    if ( line_buf != NULL ) free(line_buf) ;
    return(rt_val) ;
}

/* 親ウィンドゥの大きさにあわせて，子ウィンドゥ達を調整する */
static void configure_windows( int new_p_width, int new_p_height,
			       XWindowAttributes *clipwin_att,
			       XWindowAttributes *win_att,
			       eggx_scrollbar *vbar,
			       eggx_scrollbar *hbar, Window clipwin,
			       Window hsbarwin, Window vsbarwin, 
			       Window win )
{
    int hbar_width, vbar_width;

    /* バーの幅 */
    if ( new_p_width < win_att->width ) {
	hbar_width = Scrollbar_width;
    }
    else {
	hbar_width = 0;
    }
    if ( new_p_height < win_att->height + hbar_width ) {
	vbar_width = Scrollbar_width;
    }
    else {
	vbar_width = 0;
    }
    if ( new_p_width < win_att->width + vbar_width ) {
	hbar_width = Scrollbar_width;
    }
    else {
	hbar_width = 0;
    }
    if ( new_p_width < 2 * vbar_width ) vbar_width = 0;
    if ( new_p_height < 2 * hbar_width ) hbar_width = 0;
    /* */
    if ( win_att->width <= new_p_width - vbar_width ) {
	clipwin_att->width = new_p_width - vbar_width;
	win_att->x =	/* 中心に鎮座させる */
	    (clipwin_att->width - win_att->width) / 2;
    } else {
	int center;
	center = clipwin_att->width / 2 - win_att->x;
	clipwin_att->width = new_p_width - vbar_width;
	win_att->x = clipwin_att->width / 2 - center ;
    }
    if ( win_att->height <= new_p_height - hbar_width ) {
	clipwin_att->height = new_p_height - hbar_width;
	win_att->y =	/* 中心に鎮座させる */
	    (clipwin_att->height - win_att->height) / 2;
    } else {
	int center;
	center = clipwin_att->height / 2 - win_att->y;
	clipwin_att->height = new_p_height - hbar_width;
	win_att->y = clipwin_att->height / 2 - center ;
    }
    if ( 0 < vbar_width ) {
	XMoveWindow( Dis, vsbarwin, clipwin_att->width, 0 );
	XResizeWindow( Dis, vsbarwin, 
		       vbar_width, clipwin_att->height );
    }
    else {		/* 画面の外に追いやる */
	XMoveWindow( Dis, vsbarwin, -Scrollbar_width, 0 );
    }
    if ( 0 < hbar_width ) {
	XMoveWindow(Dis, hsbarwin, 0, clipwin_att->height);
	XResizeWindow( Dis, hsbarwin, 
		       clipwin_att->width, hbar_width );
    }
    else {		/* 画面の外に追いやる */
	XMoveWindow(Dis, hsbarwin, 0, -Scrollbar_width);
    }
    /* */
    eggx_scrollbar_update_data_params(hbar,
			    win_att->width, clipwin_att->width, -win_att->x);
    eggx_scrollbar_update_data_params(vbar,
			    win_att->height, clipwin_att->height, -win_att->y);
    win_att->x = -eggx_scrollbar_get_cliporigin(hbar);
    win_att->y = -eggx_scrollbar_get_cliporigin(vbar);
    /* */
    XResizeWindow( Dis, clipwin, 
		   clipwin_att->width, clipwin_att->height );
    XMoveWindow( Dis, win, win_att->x, win_att->y );

    return;
}

static void set_input_selection( int wid, int handle_btn_ev )
{
    if ( Pc[wid].iconwin != None ) {
	XSelectInput( Dis, Pc[wid].iconwin, ExposureMask );
    }
    /* スクロールバーつき */
    if ( Pc[wid].pwin != Pc[wid].win ) {
	XSelectInput( Dis, Pc[wid].pwin, StructureNotifyMask | KeyPressMask );
	XSelectInput( Dis, Pc[wid].clipwin, KeyPressMask );
	if ( handle_btn_ev ) {
	    XSelectInput( Dis, Pc[wid].win, StructureNotifyMask | ExposureMask
			| KeyPressMask | ButtonPressMask | ButtonReleaseMask );
	}
	else {
	    XSelectInput( Dis, Pc[wid].win, StructureNotifyMask | ExposureMask
			| KeyPressMask );
	}
    }
    /* スクロールバーなし */
    else {
	if ( handle_btn_ev ) {
	    XSelectInput(Dis, Pc[wid].win, StructureNotifyMask | ExposureMask
			 | ButtonPressMask | ButtonReleaseMask );
	}
	else {
	    XSelectInput(Dis, Pc[wid].win, StructureNotifyMask | ExposureMask);
	}
    }
    return;
}

/* スレーブプロセスのメイン */
int main( int argc, char **argv )
{
    int rt = 0;

    if ( 1 < argc ) {
	Window pc_zwin;
	XGraphicsExposeEvent geev;
	int handle_btn_ev = 1;
	unsigned long wd_background_pixel = 0;
	unsigned long wd_shadow_pixel = 0;
	unsigned long wd_trough_pixel = 0;
	unsigned long wd_highlight_pixel = 0;
	int cmd;

	cmd = atoi(*++argv);

	switch ( cmd ) {
	case 1:
	    Ppid = atol(*++argv);				/* 2 */
	    wd_background_pixel = strtoul(*++argv, (char **)NULL, 10);
	    wd_shadow_pixel = strtoul(*++argv, (char **)NULL, 10);
	    wd_trough_pixel = strtoul(*++argv, (char **)NULL, 10);
	    wd_highlight_pixel = strtoul(*++argv, (char **)NULL, 10);
	    Scrollbar_width = atoi(*++argv);
	    pc_zwin = strtoul(*++argv, (char **)NULL, 10);

	    _eggx_signal( SIGINT, 0, SIG_IGN );

	    /* fprintf(stderr,"debug: Pix=%ld\n",Pix); */

	    /* */
	    Dis = XOpenDisplay(NULL);
	    if ( Dis == NULL ) {
		fprintf(stderr,"eggx: [ERROR] Cannot open display.\n");
		return (1);
	    }
	    /* 初期ウィンドゥ: コマンドを受け付けるために必要 */
	    XSelectInput( Dis, pc_zwin, ExposureMask );
	    XFlush( Dis );
	    /* 準備が完了した事を親に送信 */
	    memset(&geev, 0, sizeof(geev));
	    geev.type = GraphicsExpose;
	    geev.send_event = True;
	    geev.display = Dis;
	    geev.drawable = None;
	    geev.minor_code = MCODE_DUMMY;
	    XSendEvent( Dis, pc_zwin, False, ExposureMask, (XEvent *)&geev);
	    XFlush( Dis );
	    /* メインループ */
	    while ( 1 ) {
		XEvent ev;
		int i;
		XNextEvent( Dis, &ev );
		/* fprintf(stderr,"debug: ev.type=%d\n",(int)ev.type); */
		if ( ev.type == GraphicsExpose ) {	/* 親からのコマンド */
		  XGraphicsExposeEvent *evp = &(ev.xgraphicsexpose);
		  if ( evp->send_event == True && 
		       evp->minor_code != MCODE_DUMMY ) {
		    int wid = evp->major_code;
		    //fprintf(stderr,"debug: major_code=%d\n",evp->major_code);
		    //fprintf(stderr,"debug:  minor_code=%d\n",evp->minor_code);
		    if ( N_Pc <= wid ) {
		      void *tmp_p;
		      int new_n_pc = wid + 1;
		      //fprintf(stderr,"debug: resize: %d -> %d\n",N_Pc,new_n_pc);
		      tmp_p = realloc(Pc, sizeof(*Pc)*(new_n_pc));
		      if ( tmp_p == NULL ) {
			fprintf(stderr,"eggx: [ERROR] realloc() failed.\n");
			if ( Ppid == getppid() ) {
			  kill(Ppid, SIGTERM);
			  while ( 1 ) sleep(1);
			}
			return (1);
		      }
		      Pc = tmp_p;
		      memset(Pc + N_Pc, 0, sizeof(*Pc) * (new_n_pc - N_Pc));
		      N_Pc = new_n_pc;
		    }
		    if ( evp->minor_code == MCODE_PIXMAP_ID ) {
			Pc[wid].pix = evp->drawable;
		    }
		    else if ( evp->minor_code == MCODE_NEEDS_REPLY ) {
			/* 親が返信を要求しているので，返事を送信 */
			memset(&geev, 0, sizeof(geev));
			geev.type = GraphicsExpose;
			geev.send_event = True;
			geev.display = Dis;
			geev.drawable = None;
			geev.minor_code = MCODE_DUMMY;
			XSendEvent( Dis, pc_zwin, False, ExposureMask, 
				    (XEvent *)&geev);
			XFlush( Dis );
		    }
		    else if ( evp->minor_code == MCODE_KEYMASK ) {
			if ( Pc[wid].flg == 0 ) {
			    fprintf(stderr,"eggx: [ERROR] Invalid wid.\n");
			}
			else {
			    unsigned int msk = (unsigned int)(evp->drawable);
			    if ( Pc[wid].vsbarwin != None ) {
				eggx_scrollbar_update_key_mask(&(Pc[wid].vbar),
							       msk);
			    }
			    if ( Pc[wid].hsbarwin != None ) {
				eggx_scrollbar_update_key_mask(&(Pc[wid].hbar),
							 msk);
			    }
			}
		    }
		    else if ( evp->minor_code == MCODE_HANDLE_BTN_EV ) {
			handle_btn_ev = 1;
			for ( i=0 ; i < N_Pc ; i++ ) {
			    if ( Pc[i].flg != 0 ) {
				set_input_selection(i, handle_btn_ev);
			    }
			}
		    }
		    else if ( evp->minor_code == MCODE_NOHANDLE_BTN_EV ) {
			handle_btn_ev = 0;
			for ( i=0 ; i < N_Pc ; i++ ) {
			    if ( Pc[i].flg != 0 ) {
				set_input_selection(i, handle_btn_ev);
			    }
			}
			XFlush( Dis );
		    }
		    else if ( evp->minor_code == MCODE_PWIN_ID ) {
			Pc[wid].pwin = evp->drawable;
		    }
		    else if ( evp->minor_code == MCODE_CLIPWIN_ID ) {
			Pc[wid].clipwin = evp->drawable;
		    }
		    else if ( evp->minor_code == MCODE_HSBARWIN_ID ) {
			Pc[wid].hsbarwin = evp->drawable;
		    }
		    else if ( evp->minor_code == MCODE_VSBARWIN_ID ) {
			Pc[wid].vsbarwin = evp->drawable;
		    }
		    else if ( evp->minor_code == MCODE_WIN_ID ) {
			Pc[wid].win = evp->drawable;
		    }
		    else if ( evp->minor_code == MCODE_ICONWIN_ID ) {
			Pc[wid].iconwin = evp->drawable;
		    }
		    else if ( evp->minor_code == MCODE_BL_ORIGIN ) {
			Pc[wid].bl_origin = evp->drawable;
		    }
		    /* 終了処理 */
		    else if ( evp->minor_code == MCODE_DISABLE ) {
			int max_idx;
			if ( Pc[wid].flg == 0 ) {
			    fprintf(stderr,"eggx: [ERROR] Invalid wid.\n");
			}
			if ( Pc[wid].bar_gc != None )
			    XFreeGC( Dis, Pc[wid].bar_gc );
			if ( Pc[wid].pwin != None )
			    XSelectInput( Dis, Pc[wid].pwin, 0 );
			if ( Pc[wid].clipwin != None )
			    XSelectInput( Dis, Pc[wid].clipwin, 0 );
			if ( Pc[wid].hsbarwin != None )
			    XSelectInput( Dis, Pc[wid].hsbarwin, 0 );
			if ( Pc[wid].vsbarwin != None )
			    XSelectInput( Dis, Pc[wid].vsbarwin, 0 );
			if ( Pc[wid].win != None )
			    XSelectInput( Dis, Pc[wid].win, 0 );
			if ( Pc[wid].iconwin != None )
			    XSelectInput( Dis, Pc[wid].iconwin, 0 );
			/* */
			memset(Pc + wid, 0, sizeof(*Pc));
			/* */
			XFlush( Dis );
			/* */
			max_idx = 0;
			for ( i=0 ; i < N_Pc ; i++ ) {
			    if ( Pc[i].flg != 0 ) max_idx = i;
			}
			if ( max_idx < 1 ) max_idx = 1;	/* 最低2個は確保 */
			if ( 2 < N_Pc && N_Pc != max_idx + 1 ) {
			    void *tmp_p;
			    int new_n_pc = max_idx + 1;
			    //fprintf(stderr,"debug: resize2: %d -> %d\n",N_Pc,new_n_pc);
			    tmp_p = realloc(Pc, sizeof(*Pc)*(new_n_pc));
			    if ( tmp_p == NULL ) {
				fprintf(stderr,
					"eggx: [ERROR] realloc() failed.\n");
				if ( Ppid == getppid() ) {
				    kill(Ppid, SIGTERM);
				    while ( 1 ) sleep(1);
				}
				return (1);
			    }
			    Pc = tmp_p;
			    N_Pc = new_n_pc;
			}
		    }
		    /* 開始処理 */
		    else if ( evp->minor_code == MCODE_ENABLE ) {
			if ( Pc[wid].flg != 0 ) {
			    fprintf(stderr,"eggx: [ERROR] Invalid wid.\n");
			}
			/* */
			XGetWindowAttributes( Dis, Pc[wid].pwin, 
					      &(Pc[wid].pwin_att) );
			XGetWindowAttributes( Dis, Pc[wid].clipwin, 
					      &(Pc[wid].clipwin_att) );
			XGetWindowAttributes( Dis, Pc[wid].win, 
					      &(Pc[wid].win_att) );
			/* */
			if ( Pc[wid].vsbarwin != None ||
			     Pc[wid].hsbarwin != None ) {
			    Pc[wid].bar_gc = XCreateGC(Dis, Pc[wid].pwin, 0,0);
			    XSetGraphicsExposures(Dis, Pc[wid].bar_gc, False);
			}
			else {
			    Pc[wid].bar_gc = None;
			}
			/* */
			if ( Pc[wid].vsbarwin != None ) {
			    eggx_scrollbar_init( &(Pc[wid].vbar), False, Dis,
						 Pc[wid].vsbarwin, 
						 Pc[wid].bar_gc, 
						 wd_background_pixel,
						 wd_shadow_pixel,
						 wd_trough_pixel, 
						 wd_highlight_pixel,
						 Pc[wid].win_att.height,
						 Pc[wid].clipwin_att.height,
						 2, 0, Pc[wid].pwin );
			}
			if ( Pc[wid].hsbarwin != None ) {
			    eggx_scrollbar_init( &(Pc[wid].hbar), True, Dis,
						 Pc[wid].hsbarwin,
						 Pc[wid].bar_gc, 
						 wd_background_pixel, 
						 wd_shadow_pixel,
						 wd_trough_pixel, 
						 wd_highlight_pixel,
						 Pc[wid].win_att.width,
						 Pc[wid].clipwin_att.width,
						 2, 0, Pc[wid].pwin );
			}
			/* */
			set_input_selection(wid, handle_btn_ev);
			/* */
			if ( Pc[wid].pwin != Pc[wid].win ) {
			    configure_windows(Pc[wid].pwin_att.width, 
					      Pc[wid].pwin_att.height,
					      &(Pc[wid].clipwin_att),
					      &(Pc[wid].win_att),
					      &(Pc[wid].vbar), &(Pc[wid].hbar),
					      Pc[wid].clipwin,
					      Pc[wid].hsbarwin,
					      Pc[wid].vsbarwin, 
					      Pc[wid].win );
			}
			/* */
			Pc[wid].flg = 1;	/* 有効! */
			XFlush( Dis );
		    }
		  }
		}
		else if ( ev.type == Expose ) {
		    XExposeEvent *evp = &(ev.xexpose);
		    for ( i=0 ; i < N_Pc ; i++ ) {
			if ( Pc[i].flg != 0 ) {
			    if ( evp->window == Pc[i].win ||
				 evp->window == Pc[i].iconwin ) break;
			}
		    }
		    if ( i < N_Pc ) {
			GC gc;
			XGCValues gv;
			gv.tile = Pc[i].pix;
			gv.fill_style = FillTiled;
			gv.fill_rule = WindingRule;
			gv.ts_x_origin = 0;
			gv.ts_y_origin = 0;
			gc = XCreateGC( Dis, evp->window,
					GCFillStyle | GCFillRule | GCTile |
					GCTileStipXOrigin | GCTileStipYOrigin,
					&gv );
			XSetGraphicsExposures(Dis, gc, False);
			XFillRectangle( Dis, evp->window, gc, evp->x, evp->y,
					evp->width, evp->height );
			XFreeGC( Dis, gc );
			XFlush( Dis );
		    }
		}
		else if ( ev.type == DestroyNotify ) {
		    for ( i=0 ; i < N_Pc ; i++ ) {
			if ( Pc[i].flg != 0 && Pc[i].bar_gc != None ) {
			    XFreeGC( Dis, Pc[i].bar_gc );
			}
		    }
		    XFlush( Dis );
		    if ( Pc != NULL ) {
			N_Pc = 0;
			free(Pc);
			Pc = NULL;
		    }
		    XCloseDisplay(Dis);
		    if ( Ppid == getppid() ) {
			kill(Ppid, SIGTERM);
			while ( 1 ) sleep(1);
		    }
		    return (1);
		    break;
		}
		else if (ev.type == ButtonPress || ev.type == ButtonRelease) {
		    Window xbwin = ev.xbutton.window;
		    Window dest_win = None;
		    for ( i=0 ; i < N_Pc ; i++ ) {
			if ( Pc[i].flg != 0 ) {
			    if ( xbwin == Pc[i].win ) {
				dest_win = xbwin;
				break;
			    }
			}
		    }
		    if ( dest_win != None ) {
			/* zwin に forward する */
			if ( ev.type == ButtonRelease ) {
			    XSendEvent(Dis, pc_zwin, False, ButtonReleaseMask,
			    	       &ev);
			    XFlush(Dis);
			}
			else {	/*  ButtonPress */
			    XUngrabPointer(Dis, CurrentTime);
			    XSendEvent(Dis, pc_zwin, False, ButtonPressMask,
			    	       &ev);
			    XFlush(Dis);
			}
		    }
		}
		else if ( ev.type == KeyPress ) {
		    Window xkwin = ev.xkey.window;
		    Window dest_pwin = None;
		    for ( i=0 ; i < N_Pc ; i++ ) {
			if ( Pc[i].flg != 0 ) {
			    if ( Pc[i].pwin != Pc[i].win ) {
				if ( xkwin == Pc[i].win || 
				     xkwin == Pc[i].clipwin ||
				     xkwin == Pc[i].hsbarwin ||
				     xkwin == Pc[i].vsbarwin ||
				     xkwin == Pc[i].pwin ) {
				    dest_pwin = Pc[i].pwin;
				    break;
				}
			    }
			    else {
				if ( xkwin == Pc[i].win ) {
				    dest_pwin = xkwin;
				    break;
				}
			    }
			}
		    }
		    if ( dest_pwin != None ) {
			/* 小窓のキーイベントは，すべて親窓にforwardする */
			if ( dest_pwin != xkwin ) {
			    XEvent n_ev = ev;
			    n_ev.xkey.window = dest_pwin;
			    XSendEvent(Dis, dest_pwin, False, KeyPressMask,
				       &n_ev);
			}
			/* 親窓の場合 */
			else {
			    /* do nothing */
			}
		    }
		}
		/* リサイズされたとき */
		else if ( ev.type == ConfigureNotify ) {
		    Window xcwin = ev.xconfigure.window;
		    for ( i=0 ; i < N_Pc ; i++ ) {
			if ( Pc[i].flg != 0 && Pc[i].pwin != Pc[i].win ) {
			    if ( xcwin == Pc[i].pwin ) break;
			    else if ( xcwin == Pc[i].win ) break;
			}
		    }
		    if ( i < N_Pc ) {
			int changed = 0;
			if ( xcwin == Pc[i].pwin ) {
			   if (Pc[i].pwin_att.width != ev.xconfigure.width ||
			       Pc[i].pwin_att.height != ev.xconfigure.height) {
			      Pc[i].pwin_att.width = ev.xconfigure.width;
			      Pc[i].pwin_att.height = ev.xconfigure.height;
			      changed = 1;
			   }
			}
			else {
			   if (Pc[i].win_att.width != ev.xconfigure.width ||
			       Pc[i].win_att.height != ev.xconfigure.height) {
			      if ( Pc[i].win_att.width <= Pc[i].clipwin_att.width ) {
				 Pc[i].clipwin_att.width = ev.xconfigure.width;
			      }
			      if ( Pc[i].win_att.height <= Pc[i].clipwin_att.height ) {
				 Pc[i].clipwin_att.height = ev.xconfigure.height;
			      }
			      else {
				  if ( Pc[i].bl_origin ) {
				      Pc[i].win_att.y -= 
				   ev.xconfigure.height - Pc[i].win_att.height;
				  }
			      }
			      Pc[i].win_att.width = ev.xconfigure.width;
			      Pc[i].win_att.height = ev.xconfigure.height;
			      changed = 1;
			   }
			}
			if ( changed ) {
			    configure_windows( Pc[i].pwin_att.width,
					       Pc[i].pwin_att.height,
					       &(Pc[i].clipwin_att), 
					       &(Pc[i].win_att),
					       &(Pc[i].vbar), &(Pc[i].hbar),
					       Pc[i].clipwin, Pc[i].hsbarwin,
					       Pc[i].vsbarwin, Pc[i].win );
			    XFlush(Dis);
			}
		    }
		}
		/* スクロールバーに対するイベントを扱う */
		for ( i=0 ; i < N_Pc ; i++ ) {
		    if ( Pc[i].flg != 0 && Pc[i].pwin != Pc[i].win ) {
			int changed_hbar = 0, changed_vbar = 0;
			if ( Pc[i].hsbarwin != None ) {
			  changed_hbar = 
			    eggx_scrollbar_handle_event(&(Pc[i].hbar), &ev);
			}
			if ( Pc[i].vsbarwin != None ) {
			  changed_vbar =
			    eggx_scrollbar_handle_event(&(Pc[i].vbar), &ev);
			}
			if ( changed_hbar != 0 || changed_vbar != 0 ) {
			    Pc[i].win_att.x =
				-eggx_scrollbar_get_cliporigin(&(Pc[i].hbar));
			    Pc[i].win_att.y = 
				-eggx_scrollbar_get_cliporigin(&(Pc[i].vbar));
			    XMoveWindow(Dis, Pc[i].win, 
					Pc[i].win_att.x, Pc[i].win_att.y);
			    XFlush(Dis);
			}
		    }
		}
	    }
	    break ;
	case 2:
	    Ppid = atol(*++argv);
	    _eggx_signal( SIGINT, 0, SIG_IGN );
	    _eggx_signal( SIGCHLD, SA_NOCLDSTOP|SA_RESTART, &chldexithandler );
	    Dis = XOpenDisplay(NULL);
	    if ( Dis == NULL ) {
		fprintf(stderr,"eggx: [ERROR] Cannot open display.\n");
		return (1);
	    }
	    rt = imgsv(argv);
	    XCloseDisplay(Dis);
	    break;
	default:
	    rt=1;
	    break;
	}
    }
    else rt=1;

    return (rt);
}
