# CMake Toolchain file for cross-compiling PortAudio to i686-w64-mingw32
# Inspired from: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/cross_compiling/Mingw
# Example usage: $ cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/i686-w64-mingw32.cmake .
# i686-w64-mingw32 needs to be installed for this to work. On Debian-based
# distributions the package is typically named `mingw-w64`.

SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
