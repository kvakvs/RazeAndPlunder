#
# Getting started: see https://github.com/tpoechtrager/wclang
# Use your home ~/wclang as install prefix, otherwise modify the paths to wclang below
#
# Or you can install MinGW32 yourself and uncomment those gcc-win32 lines below
#

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# Should be 32-bit compiler, because parts of the source are 32-bit
set(TOOLCHAIN_PREFIX i686-w64-mingw32)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER    /usr/bin/${TOOLCHAIN_PREFIX}-gcc-win32)
set(CMAKE_CXX_COMPILER  /usr/bin/${TOOLCHAIN_PREFIX}-g++-win32)
#set(CMAKE_C_COMPILER    $ENV{HOME}/wclang/bin/w64-clang)
#set(CMAKE_CXX_COMPILER  $ENV{HOME}/wclang/bin/w64-clang++)

# here is the target environment located
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
