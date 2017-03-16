# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX i686-w64-mingw32)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER    /usr/bin/${TOOLCHAIN_PREFIX}-gcc-win32)
set(CMAKE_CXX_COMPILER  /usr/bin/${TOOLCHAIN_PREFIX}-g++-win32)
#set(CMAKE_LINKER        /usr/bin/${TOOLCHAIN_PREFIX}-ld)

# here is the target environment located
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
