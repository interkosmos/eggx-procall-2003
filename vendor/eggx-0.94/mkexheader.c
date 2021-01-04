/* -*- Mode: C ; Coding: euc-japan -*- */
/* Time-stamp: <2010-02-21 02:43:05 cyamauch> */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int readexecfile( const unsigned char *filename, unsigned char **buffer )
{
    int fd;
    struct stat st;
    unsigned char *buf;
    const char *fname = (const char *)filename;
    if ( stat(fname, &st) ) return -1;
    buf = malloc(sizeof(unsigned char)*st.st_size);
    if ( buf == NULL ) return -1;
    fd = open(fname, O_RDONLY);
    if ( fd == -1 ) return -1;
    read( fd, buf, st.st_size );
    close(fd);
    *buffer = buf;
    return st.st_size;
}

int main( int argc, char *argv[] )
{
    int i, len;
    unsigned char *buffer;
    unsigned char filename[256];
    unsigned char vname[256];
#ifdef __CYGWIN__
    const char *dot_exe = ".exe";
    int j;
#endif
    if ( argc < 2 ) return 1;
    for ( i=0 ; i < 256 ; i++ ) {
	int ch = ((const unsigned char *)argv[1])[i];
	if ( ch == '\0' ) break;
	filename[i] = ch;
    }
#ifdef __CYGWIN__
    for ( j=0 ; i < 256 ; i++,j++ ) {
	int ch = ((const unsigned char *)dot_exe)[j];
	if ( ch == '\0' ) break;
	filename[i] = ch;
    }
#endif
    filename[i] = '\0';
    if ( (len=readexecfile(filename,&buffer)) < 0 ) return -1;
    for ( i=0 ; i < 256 ; i++ ) {
	int ch = ((const unsigned char *)filename)[i];
	if ( ch == '\0' || ch == '.' ) break;
	vname[i] = ch;
    }
    vname[i] = '\0';
    printf("unsigned char %s[] = {", vname);
    for ( i=0 ; i < len ; i++ ) {
	if ( i != 0 ) printf(",");
	if ( (i % 16) == 0 ) {
	    printf("\n");
	}
	printf("0x%02x", buffer[i]);
    }
    printf(" } ;\n\n");
    free(buffer);
    return 0;
}
