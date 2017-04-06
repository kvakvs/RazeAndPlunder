#!/bin/sh

git clone https://github.com/bwapi/bwapi deps/bwapi --branch develop

BUILDDIR=cmake-build-debug

rm -rf $BUILDDIR
mkdir -p $BUILDDIR
cd $BUILDDIR
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake -DRNP_MSVC=0 ..
