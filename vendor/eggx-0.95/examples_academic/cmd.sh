#!/bin/sh

if [ -x $1 ]; then
  echo $@
  $@
else
  shift
  ARGS=""
  for i in $@ ; do
    if [ "`echo $i | grep '^[-][IL]'`" = "" ]; then
      ARGS="$ARGS $i"
    fi
  done
  echo egg $ARGS
  egg $ARGS
fi
