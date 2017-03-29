@echo off

rmdir /s /q cmake-vs-debug
mkdir cmake-vs-debug
cd cmake-vs-debug
cmake -G "Visual Studio 14 2015" -DRNP_MSVC=1 -DCMAKE_BUILD_TYPE=Debug .. || echo ====== CMAKE FINISHED WITH ERROR ====== && exit /b
cd ..

rmdir /s /q cmake-vs-release
mkdir cmake-vs-release
cd cmake-vs-release
cmake -G "Visual Studio 14 2015" -DRNP_MSVC=1 -DCMAKE_BUILD_TYPE=Release .. || echo ====== CMAKE FINISHED WITH ERROR ====== && exit /b
