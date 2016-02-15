# Locate libuv library
# This module defines
#  LIBUV_FOUND, if false, do not try to link to libuv
#  LIBUV_LIBRARIES
#  LIBUV_INCLUDE_DIR, where to find uv.h

FIND_PATH(LIBUV_INCLUDE_DIR NAMES uv.h)
FIND_LIBRARY(LIBUV_LIBRARIES NAMES uv libuv)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBUV DEFAULT_MSG LIBUV_LIBRARIES LIBUV_INCLUDE_DIR)
