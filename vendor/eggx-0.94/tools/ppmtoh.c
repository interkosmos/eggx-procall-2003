/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-02-28 05:52:02 cyamauch> */

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

int main( int argc, char *argv[] )
{
    int ret = -1;
    const char *filename;
    const char *pos;
    char imgname_l[256];
    char imgname_u[256];
    char line_buf[256];
    int i, width, height, maxv, ppmascii=0 ;
    FILE *fp = NULL ;

    if( argc < 2 ){
	fprintf(stderr,"Specify ppm file\n") ;
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

    /* 本体オープン */
    fp = fopen(filename,"rb") ;
    if( fp == NULL ) {
	fprintf(stderr,"Cannot open file: %s\n",filename) ;
	goto quit;
    }

    /* ここからファイル読み込み */
    pos = next(fp,line_buf,256) ;
    if ( pos == NULL || pos[0] != 'P' ) {
	fprintf(stderr,"ERROR: Invalid stream (1)\n");
	goto quit;
    }

    if ( pos[1] != '6' && pos[1] != '3' ) {
	fprintf(stderr,"ERROR: Invalid stream (2)\n");
	goto quit;
    }
    if ( pos[1] == '3' ) ppmascii = 1;

    pos = next(fp,line_buf,256) ;
    if ( pos == NULL ) {
	fprintf(stderr,"ERROR: Invalid stream (3)\n");
	goto quit;
    }

    i = sscanf(pos,"%d %d\n",&width,&height) ;
    if( i!=2 ) {
	fprintf(stderr,"ERROR: Invalid stream (4)\n");
	goto quit;
    }

    pos = next(fp,line_buf,256) ;
    if ( pos == NULL ) {
	fprintf(stderr,"ERROR: Invalid stream (5)\n");
	goto quit;
    }

    i = sscanf(pos,"%d\n",&maxv) ;
    if( i!=1 ) {
	fprintf(stderr,"ERROR: Invalid stream (6)\n");
	goto quit;
    }

    printf("#define PPM_WIDTH_%s %d\n",imgname_u,width) ;
    printf("#define PPM_HEIGHT_%s %d\n",imgname_u,height) ;
    printf("unsigned char Ppm_image_%s[] = { \n",imgname_l) ;

    if ( ppmascii ) {
	for( i=0 ; i < width*height*3 ; i++ ){
	    if( (i % 3)==0 ) printf("0xff,") ;
	    printf("0x%02x",255 * numread(fp) / maxv) ;
	    if( i!=(width*height*3-1) ) printf(",") ;
	    if( ((i+1) % 12)==0 ) printf("\n") ;
	}
    }
    else {
	int vl;
	int nbytes = (maxv < 256) ? 1 : 2;
	for( i=0 ; i < width*height*3 ; i++ ){
	    if( (i % 3)==0 ) printf("0xff,") ;
	    vl = fgetc(fp);
	    if ( 1 < nbytes ) {
		vl <<= 8;  vl |= fgetc(fp);
	    }
	    printf("0x%02x",255 * vl / maxv) ;
	    if( i!=(width*height*3-1) ) printf(",") ;
	    if( ((i+1) % 12)==0 ) printf("\n") ;
	}
    }
    printf("} ;\n") ;
    fclose(fp) ;
    fp = NULL;

    ret = 0;
 quit:
    if ( fp != NULL ) fclose(fp);
    return ret;
}
