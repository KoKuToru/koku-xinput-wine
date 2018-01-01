#!/bin/bash

set -e

SRC="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"

#DIR=$SRC/build/i686-w64-mingw32
DIR=$SRC/build/x86_64-w64-mingw32

mkdir -p $SRC/build

pushd $SRC/build &> /dev/null
cmake ..
make -j10
popd &> /dev/null

pushd $DIR &> /dev/null
export KOKU_XINPUT_DEBUG=1
export LD_PRELOAD="$SRC/build/koku-xinput-wine.so $SRC/build/koku-xinput-wine64.so"
wine xitest.exe
wine ditest.exe
popd &> /dev/null
