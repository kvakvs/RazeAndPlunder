#!/bin/bash

rm -rf cmake-mingw
mkdir cmake-mingw
cd cmake-mingw
cmake -DRNP_MSVC=0 ..

