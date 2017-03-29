#!/bin/sh

BUILDDIR=cmake-build-debug

mkdir -p $BUILDDIR
cd $BUILDDIR
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake -DRNP_MSVC=0 ..
