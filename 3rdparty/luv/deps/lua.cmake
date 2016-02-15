# Modfied from luajit.cmake
# Added LUAJIT_ADD_EXECUTABLE Ryan Phillips <ryan at trolocsis.com>
# This CMakeLists.txt has been first taken from LuaDist
# Copyright (C) 2007-2011 LuaDist.
# Created by Peter Drahoš
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# Debugged and (now seriously) modified by Ronan Collobert, for Torch7

#project(Lua53 C)

SET(LUA_DIR ${CMAKE_CURRENT_LIST_DIR}/lua)

SET(CMAKE_REQUIRED_INCLUDES
  ${LUA_DIR}
  ${LUA_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
)

OPTION(WITH_AMALG "Build eveything in one shot (needs memory)" ON)

# Ugly warnings
IF(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

# Various includes
INCLUDE(CheckLibraryExists)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckTypeSize)

CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P)
IF(SIZEOF_VOID_P EQUAL 8)
  ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
ENDIF()

IF(NOT WIN32)
  FIND_LIBRARY(DL_LIBRARY "dl")
  IF(DL_LIBRARY)
    SET(CMAKE_REQUIRED_LIBRARIES ${DL_LIBRARY})
    LIST(APPEND LIBS ${DL_LIBRARY})
  ENDIF(DL_LIBRARY)
  CHECK_FUNCTION_EXISTS(dlopen LUA_USE_DLOPEN)
  IF(NOT LUA_USE_DLOPEN)
    MESSAGE(FATAL_ERROR "Cannot compile a useful lua.
Function dlopen() seems not to be supported on your platform.
Apparently you are not on a Windows platform as well.
So lua has no way to deal with shared libraries!")
  ENDIF(NOT LUA_USE_DLOPEN)
ENDIF(NOT WIN32)

check_library_exists(m sin "" LUA_USE_LIBM)
if ( LUA_USE_LIBM )
  list ( APPEND LIBS m )
endif ()

## SOURCES
SET(SRC_LUALIB
  ${LUA_DIR}/src/lbaselib.c
  ${LUA_DIR}/src/lcorolib.c
  ${LUA_DIR}/src/ldblib.c
  ${LUA_DIR}/src/liolib.c
  ${LUA_DIR}/src/lmathlib.c
  ${LUA_DIR}/src/loadlib.c
  ${LUA_DIR}/src/loslib.c
  ${LUA_DIR}/src/lstrlib.c
  ${LUA_DIR}/src/ltablib.c
  ${LUA_DIR}/src/lutf8lib.c)

SET(SRC_LUACORE
  ${LUA_DIR}/src/lauxlib.c
  ${LUA_DIR}/src/lapi.c
  ${LUA_DIR}/src/lcode.c
  ${LUA_DIR}/src/lctype.c
  ${LUA_DIR}/src/ldebug.c
  ${LUA_DIR}/src/ldo.c
  ${LUA_DIR}/src/ldump.c
  ${LUA_DIR}/src/lfunc.c
  ${LUA_DIR}/src/lgc.c
  ${LUA_DIR}/src/linit.c
  ${LUA_DIR}/src/llex.c
  ${LUA_DIR}/src/lmem.c
  ${LUA_DIR}/src/lobject.c
  ${LUA_DIR}/src/lopcodes.c
  ${LUA_DIR}/src/lparser.c
  ${LUA_DIR}/src/lstate.c
  ${LUA_DIR}/src/lstring.c
  ${LUA_DIR}/src/ltable.c
  ${LUA_DIR}/src/ltm.c
  ${LUA_DIR}/src/lundump.c
  ${LUA_DIR}/src/lvm.c
  ${LUA_DIR}/src/lzio.c
  ${SRC_LUALIB})

## GENERATE

IF(WITH_SHARED_LUA)
  IF(WITH_AMALG)
    add_library(lualib SHARED ${LUA_DIR}/../lua_one.c ${DEPS})
  ELSE()
    add_library(lualib SHARED ${SRC_LUACORE} ${DEPS} )
  ENDIF()
ELSE()
  IF(WITH_AMALG)
    add_library(lualib STATIC ${LUA_DIR}/../lua_one.c ${DEPS} )
  ELSE()
    add_library(lualib STATIC ${SRC_LUACORE} ${DEPS} )
  ENDIF()
  set_target_properties(lualib PROPERTIES
    PREFIX "lib" IMPORT_PREFIX "lib")
ENDIF()

target_link_libraries (lualib ${LIBS} )
set_target_properties (lualib PROPERTIES OUTPUT_NAME "lua53")

IF(WIN32)
  add_executable(lua ${LUA_DIR}/src/lua.c)
  target_link_libraries(lua lualib)
ELSE()
  IF(WITH_AMALG)
    add_executable(lua ${LUA_DIR}/src/lua.c ${LUA_DIR}/lua_one.c ${DEPS})
  ELSE()
    add_executable(lua ${LUA_DIR}/src/lua.c ${SRC_LUACORE} ${DEPS})
  ENDIF()
  target_link_libraries(lua ${LIBS})
  SET_TARGET_PROPERTIES(lua PROPERTIES ENABLE_EXPORTS ON)
ENDIF(WIN32)

