#!/bin/sh

#cat Makefile.default | \
#  sed -e 's|\(^LLINKS[ ]*=\)\(.*[-]lX11 \)\(.*\)|\1\2-lXinerama \3|' \
#      -e 's|\(^#[ ]*\)\(XDEFS[ ]*=.*\)|\2|' > Makefile.linux

cat Makefile.default > Makefile.linux

#cat Makefile.default | \
#  sed -e 's/\(^DEFS[ ]*=.*\)/\1 -DCYGWIN/' \
#      -e 's/\(_xslave_\)\([^.]\)/\1.exe\2/g' \
#      -e 's/\(_xslave_$\)/\1.exe/g' \
#      -e 's/\(mkexheader\)\([^.]\)/\1.exe\2/g' \
#      -e 's/\(mkexheader$\)/\1.exe/g' > Makefile.cygwin

cat Makefile.default | \
  sed -e 's|\(^IINC[ ]*=\)\(.*\)|\1 -I/usr/include/X11R6|' \
      -e 's|\(^LLIB[ ]*=\)\(.*\)|\1 -L/usr/lib/X11R6|' \
      -e 's|\(^#[ ]*\)\(VDEFS[ ]*=.*\)|\2|' > Makefile.hp

cat Makefile.default | \
  sed -e 's|\(^IINC[ ]*=\)\(.*\)|\1 -I/usr/include/X11R6|' \
      -e 's|\(^LLIB[ ]*=\)\(.*\)|\1 -L/usr/lib/X11R6/pa20_64|' \
      -e 's|\(^INCDIR[ ]*=\)\(.*\)|\1 -I$(PREFIX)/pa20_64/include|' \
      -e 's|\(^LIBDIR[ ]*=\)\(.*\)|\1 -L$(PREFIX)/pa20_64/lib|' \
      -e 's|\(^CC[ ]*=\)\(.*\)|\1 /usr/local/pa20_64/bin/gcc|' \
      -e 's|\(^USERCC[ ]*=\)\(.*\)|\1 /usr/local/pa20_64/bin/gcc|' \
      -e 's|ranlib|/usr/local/pa20_64/bin/ranlib|' \
      -e 's|\(^#[ ]*\)\(VDEFS[ ]*=.*\)|\2|' > Makefile.hp64

cat Makefile.default | \
  sed -e 's|\(^LLIB[ ]*=\)\(.*\)|#\1\2|' \
      -e 's|\(^#\)\(LLIB[ ]*=[ ]*.*[-]L/usr/X11R6/lib64$\)|\2|' \
      -e 's|\(^LIBDIR[ ]*=\)\(.*\)|#\1\2|' \
      -e 's|\(^#\)\(LIBDIR[ ]*=[ ]* [$][(]PREFIX[)]/lib64$\)|\2|' > Makefile.linux64

cat Makefile.default | \
  sed -e 's/\(^DEFS[ ]*=.*\)/\1 -DANOTHERPS/' > Makefile.netbsd

cat Makefile.default | \
  sed -e 's|\(^USERFC[ ]*=\)\(.*\)|#\1\2|' \
      -e 's|\(^#\)\(USERFC[ ]*=[ ]*\)\(f77$\)|\2\3|' \
      -e 's|\(^USERFCFLAGS[ ]*=\)\(.*\)|\1 -O2|' \
      -e 's|\(^#[ ]*\)\(VDEFS[ ]*=.*\)|\2|' \
      -e 's|\(^#[ ]*\)\(TDEFS[ ]*=.*\)|\2|' \
      -e 's|\(^#[ ]*\)\(UDEFS[ ]*=.*\)|\2|' > Makefile.old

cat Makefile.default | \
  sed -e 's|\(^IINC[ ]*=\)\(.*\)|#\1\2|' \
      -e 's|\(^#\)\(IINC[ ]*=[ ]* [-]I/usr/openwin/include$\)|\2|' \
      -e 's|\(^LLIB[ ]*=\)\(.*\)|#\1\2|' \
      -e 's|\(^#\)\(LLIB[ ]*=[ ]* [-]L/usr/openwin/lib$\)|\2|' \
      -e 's|\(^USERFC[ ]*=\)\(.*\)|#\1\2|' \
      -e 's|\(^#\)\(USERFC[ ]*=[ ]*\)\(f77$\)|\2\3|' \
      -e 's|\(^USERFCFLAGS[ ]*=\)\(.*\)|\1 -O2|' \
      -e 's|\(^LLINKS[ ]*=\)\(.*\)|\1\2 -lsocket|' \
      -e 's|\(^#[ ]*\)\(VDEFS[ ]*=.*\)|\2|' > Makefile.sun

cat Makefile.default | \
  sed -e 's|\(^CFLAGS[ ]*=\)\(.*\)|\1 -m64 -O2 -Wall|' \
      -e 's|\(^USERCCFLAGS[ ]*=\)\(.*\)|\1 -m64 -O2 -Wall|' \
      -e 's|\(^IINC[ ]*=\)\(.*\)|#\1\2|' \
      -e 's|\(^#\)\(IINC[ ]*=[ ]* [-]I/usr/openwin/include$\)|\2|' \
      -e 's|\(^LLIB[ ]*=\)\(.*\)|#\1\2|' \
      -e 's|\(^#\)\(LLIB[ ]*=[ ]* [-]L/usr/openwin/lib$\)|\2/64|' \
      -e 's|\(^USERFC[ ]*=\)\(.*\)|#\1\2|' \
      -e 's|\(^#\)\(USERFC[ ]*=[ ]*\)\(f77$\)|\2\3|' \
      -e 's|\(^USERFCFLAGS[ ]*=\)\(.*\)|\1 -m64 -O2|' \
      -e 's|\(^LIBDIR[ ]*=\)\(.*\)|\1 $(PREFIX)/lib/64|' \
      -e 's|\(^LLINKS[ ]*=\)\(.*\)|\1\2 -lsocket|' \
      -e 's|\(^#[ ]*\)\(VDEFS[ ]*=.*\)|\2|' > Makefile.sun64

