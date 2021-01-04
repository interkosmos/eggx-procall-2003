#!/bin/sh

USERCC="@USERCC@"
USERCCFLAGS="@USERCCFLAGS@"

USERCPPC="g++"
USERCPPCFLAGS=$USERCCFLAGS

USERFC="@USERFC@"
USERFCFLAGS="@USERFCFLAGS@"

IINC="@IINC@"
LLIB="@LLIB@"

LOCALINC="@LOCALINC@"
LOCALLIB="@LOCALLIB@"

#LOCALINC=""
#LOCALLIB=""
#LOCALINC="-I${HOME}/EGGX"
#LOCALLIB="-L${HOME}/EGGX"
#LOCALINC="-I${HOME}/include"
#LOCALLIB="-L${HOME}/lib"

LLINKS="@LLINKS@"

LST=$IINC
IINC=""
for i in $LST ; do
  if [ -d `echo $i|sed -e 's/^-I//'` ] ; then
    IINC="$IINC $i"
  fi
done

LST=$LLIB
LLIB=""
for i in $LST ; do
  if [ -d `echo $i|sed -e 's/^-L//'` ] ; then
    LLIB="$LLIB $i"
  fi
done

LST=$LOCALINC
LOCALINC=""
for i in $LST ; do
  if [ -f `echo $i/eggx.h|sed -e 's/^-I//'` ] ; then
    LOCALINC="$LOCALINC $i"
  fi
done

LST=$LOCALLIB
LOCALLIB=""
for i in $LST ; do
  if [ -f `echo $i/libeggx.a|sed -e 's/^-L//'` ] ; then
    LOCALLIB="$LOCALLIB $i"
  fi
done

BASENAME=`basename $0`
if [ "$LOCALINC" = "" -o "$LOCALLIB" = "" ] ; then
  EGGXDIR=`echo $0 | sed -e "s/\/${BASENAME}\$//"`
  if [ "$LOCALINC" = "" ] ; then
    LOCALINC="-I$EGGXDIR"
  fi
  if [ "$LOCALLIB" = "" ] ; then
    LOCALLIB="-L$EGGXDIR"
  fi
fi

s_match () { case $1 in $2) return 0; ;; *) return 1; ;; esac ; }

get_bin_name() {
  echo $1 | sed -e 's/\(.*\)\(\.\)\([^\.]*\)/\1/'
}

COMPILER=""
FIRST_CX=""
ARG_OC=0
for i in $@ ; do
  if s_match "$i" "*.c" ; then
    COMPILER="C"
    if [ "$FIRST_CX" = "" ]; then
      FIRST_CX=$i
    fi
  elif s_match "$i" "*.cc" ; then
    COMPILER="C++"
    if [ "$FIRST_CX" = "" ]; then
      FIRST_CX=$i
    fi
  elif s_match "$i" "*.cpp" ; then
    COMPILER="C++"
    if [ "$FIRST_CX" = "" ]; then
      FIRST_CX=$i
    fi
  elif s_match "$i" "*.cc" ; then
    COMPILER="FORTRAN"
    if [ "$FIRST_CX" = "" ]; then
      FIRST_CX=$i
    fi
  elif [ "$i" = "-o" ]; then
    ARG_OC=1
  elif [ "$i" = "-c" ]; then
    ARG_OC=1
  fi
done

if [ $ARG_OC = 0 -a "$FIRST_CX" != "" ]; then
  BIN_NAME="`get_bin_name $FIRST_CX`"
  ARGS="$@ -o $BIN_NAME"
else
  ARGS="$@"
fi


if [ "$COMPILER" = "C" ] ; then
  CMD="$USERCC $USERCCFLAGS $ARGS $LOCALINC $LOCALLIB $IINC $LLIB $LLINKS"
  echo "$CMD"
  $CMD
elif [ "$COMPILER" = "C++" ] ; then
  CMD="$USERCPPC $USERCPPCFLAGS $ARGS $LOCALINC $LOCALLIB $IINC $LLIB $LLINKS"
  echo "$CMD"
  $CMD
elif [ "$COMPILER" = "FORTRAN" ] ; then
  CMD="$USERFC $USERFCFLAGS $ARGS $LOCALLIB $LLIB $LLINKS"
  echo "$CMD"
  $CMD
else
  echo "${BASENAME}: No input files specified."
fi

