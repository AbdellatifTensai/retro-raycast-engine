#!/bin/sh

CFLAGS="-Wall -Wextra -pedantic"
LIBS="x11"
LINKER_FLAGS="`pkg-config $LIBS --libs --static | sed -E 's/-l(\w*)/-l:lib\1.a/g'` -l:libm.a"

if [ $1 = "main" ]; then
    set -xe
    gcc $CFLAGS -O3 -g src/main.c -o main $LINKER_FLAGS

elif [ $1 = "debug" ]; then
    set -xe
    gcc $CFLAGS -DPROFILE -g src/main.c -o debug $LINKER_FLAGS

else
    echo "USAGE: build.sh <main|debug>"
    exit 1

fi
