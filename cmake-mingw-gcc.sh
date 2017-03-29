#!/bin/sh

BUILDDIR=./cmake-build-debug
rm -rf $BUILDDIR
mkdir $BUILDDIR
cd $BUILDDIR

cmake -DRNP_MSVC=0 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake ..
