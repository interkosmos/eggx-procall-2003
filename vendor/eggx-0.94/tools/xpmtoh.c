/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-02-28 05:51:58 cyamauch> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *next( FILE *fp, char *buf, size_t sz )
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

int numread( FILE *fp )
{
    int c,i ;
    char buf[16] ;

    while( 1 ){
	c=fgetc(fp) ;
	if( isdigit(c) ){
	    break ;
	}
    }
    i=0 ;
    buf[i]=c ;
    for( i=1 ; i<15 ; i++ ){
	c=fgetc(fp) ;
	if( isdigit(c)==0 ){
	    break ;
	}
	buf[i]=c ;
    }
    if( i==15 ) exit(-1) ;
    buf[i]='\0' ;
    return( atoi(buf) ) ;
}

void output_maskinfo( const unsigned char *mask_buf, size_t bytes_per_line,
		      int x, int y )
{
    const unsigned char *ptr ;
    ptr = mask_buf + bytes_per_line * y;
    ptr += x/8;

    if ( (*ptr & ((unsigned char)1) << (7 - (x % 8))) ) printf("0x00,") ;
    else printf("0xff,") ;

    return;
}

int main( int argc, char *argv[] )
{
    int ret = -1;
    const char *filename;
    const char *pos;
    char cmd[512];
    char imgname_l[256];
    char imgname_u[256];
    char line_buf[256];
    int i,j,k,l, maxv, ppmascii=0 ;
    int width, height, bytes_per_line ;
    size_t sz ;
    FILE *fp = NULL ;
    unsigned char *mask_buf = NULL ;

    if( argc < 2 ){
	fprintf(stderr,"Specify xpm file\n") ;
	return(0) ;
    }

    filename = argv[1];

    /* basename からマクロ名と変数名で使う文字列を作る */
    pos = strrchr(filename,'/');
    if ( pos ) snprintf(imgname_l,256,"%s",pos+1);
    else snprintf(imgname_l,256,"%s",filename);

    for ( i=0 ; imgname_l[i] != '\0' ; i++ ) {
	int valid = 0;
	int ch = ((unsigned char *)imgname_l)[i];
	if ( '0' <= ch && ch <= '9' ) valid = 1;
	else if ( 'A' <= ch && ch <= 'Z' ) valid = 1;
	else if ( 'a' <= ch && ch <= 'z' ) valid = 1;
	else if ( ch == '_' ) valid = 1;
	else if ( ch == '.' ) break;
	if ( valid == 0 ) imgname_l[i] = '_';
    }
    imgname_l[i] = '\0';
    for ( i=0 ; imgname_l[i] != '\0' ; i++ ) {
	int ch = ((unsigned char *)imgname_l)[i];
	if ( 'a' <= ch && ch <= 'z' ) imgname_u[i] = ch - 0x0020;
	else imgname_u[i] = ch;
    }
    imgname_u[i] = '\0';

    /* マスクオープン */
    snprintf(cmd,512,"xpmtoppm --alphaout=- %s",filename);
    fp = popen(cmd,"r") ;
    if( fp == NULL ) {
	fprintf(stderr,"Cannot exec command: %s\n",cmd) ;
	goto quit;
    }

    /* ここからファイル読み込み */
    pos = next(fp,line_buf,256) ;
    if ( pos == NULL || pos[0] != 'P' ) {
	fprintf(stderr,"ERROR: Invalid stream (1)\n");
	goto quit;
    }
    if ( pos[1] != '4' ) {
	fprintf(stderr,"ERROR: Invalid stream (2)\n");
	goto quit;
    }

    pos = next(fp,line_buf,256) ;
    if ( pos == NULL ) {
	fprintf(stderr,"ERROR: Invalid stream (3)\n");
	goto quit;
    }

    i = sscanf(pos,"%d %d\n",&width,&height) ;
    if ( i != 2 ) {
	fprintf(stderr,"ERROR: Invalid stream (4)\n");
	goto quit;
    }

    bytes_per_line = (width + 7)/8;

    sz = bytes_per_line * height;
    mask_buf = (unsigned char *)malloc(sz);
    if ( mask_buf == NULL ) {
	fprintf(stderr,"ERROR: malloc() failed\n");
	goto quit;
    }

    if ( fread(mask_buf, 1, sz, fp) != sz ) {
	fprintf(stderr,"ERROR: Invalid stream (5)\n");
	goto quit;
    }

    fclose(fp);
    fp = NULL;

    /* 本体オープン */
    snprintf(cmd,512,"xpmtoppm %s",filename);
    fp = popen(cmd,"r") ;
    if( fp == NULL ) {
	fprintf(stderr,"Cannot exec command: %s\n",cmd) ;
	goto quit;
    }

    /* ここからファイル読み込み */
    pos = next(fp,line_buf,256) ;
    if ( pos == NULL || pos[0] != 'P' ) {
	fprintf(stderr,"ERROR: Invalid stream (6)\n");
	goto quit;
    }

    if ( pos[1] != '6' && pos[1] != '3' ) {
	fprintf(stderr,"ERROR: Invalid stream (7)\n");
	goto quit;
    }
    if ( pos[1] == '3' ) ppmascii=1 ;

    pos = next(fp,line_buf,256) ;
    if ( pos == NULL ) {
	fprintf(stderr,"ERROR: Invalid stream (8)\n");
	goto quit;
    }

    i = sscanf(pos,"%d %d\n",&width,&height) ;
    if ( i != 2 ) {
	fprintf(stderr,"ERROR: Invalid stream (9)\n");
	goto quit;
    }

    pos = next(fp,line_buf,256) ;
    if ( pos == NULL ) {
	fprintf(stderr,"ERROR: Invalid stream (8)\n");
	goto quit;
    }

    i = sscanf(pos,"%d\n",&maxv) ;
    if ( i != 1 ) {
	fprintf(stderr,"ERROR: Invalid stream (9)\n");
	goto quit;
    }

    printf("#define XPM_WIDTH_%s %d\n",imgname_u,width) ;
    printf("#define XPM_HEIGHT_%s %d\n",imgname_u,height) ;
    printf("unsigned char Xpm_image_%s[] = { \n",imgname_l) ;

    if ( ppmascii ) {
	i=0;
	for( k=0 ; k < height ; k++ ){
	    for( j=0 ; j < width ; j++ ){
		output_maskinfo(mask_buf,bytes_per_line,j,k) ;
		for ( l=0 ; l < 3 ; l++ ) {
		    printf("0x%02x",255 * numread(fp) / maxv) ;
		    if( i!=(width*height*3-1) ) printf(",") ;
		    i++;
		}
		if( ((j+1) % 4)==0 ) printf("\n") ;
	    }
	    printf("\n");
	}
    }
    else {
	int vl;
	int nbytes = (maxv < 256) ? 1 : 2;
	i=0;
	for( k=0 ; k < height ; k++ ){
	    for( j=0 ; j < width ; j++ ){
		output_maskinfo(mask_buf,bytes_per_line,j,k) ;
		for ( l=0 ; l < 3 ; l++ ) {
		    vl = fgetc(fp);
		    if ( 1 < nbytes ) {
			vl <<= 8;  vl |= fgetc(fp);
		    }
		    printf("0x%02x",255 * vl / maxv) ;
		    if( i!=(width*height*3-1) ) printf(",") ;
		    i++;
		}
		if( ((j+1) % 4)==0 ) printf("\n") ;
	    }
	    printf("\n");
	}
    }
    printf("} ;\n") ;
    fclose(fp) ;
    fp = NULL;

    ret = 0;
 quit:
    if ( mask_buf != NULL ) free(mask_buf);
    if ( fp != NULL ) fclose(fp);
    return ret;
}
