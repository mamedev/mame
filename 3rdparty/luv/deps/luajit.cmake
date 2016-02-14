# Added LUAJIT_ADD_EXECUTABLE Ryan Phillips <ryan at trolocsis.com>
# This CMakeLists.txt has been first taken from LuaDist
# Copyright (C) 2007-2011 LuaDist.
# Created by Peter Drahoš
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# Debugged and (now seriously) modified by Ronan Collobert, for Torch7

#project(LuaJIT C ASM)

SET(LUAJIT_DIR ${CMAKE_CURRENT_LIST_DIR}/luajit)

SET(CMAKE_REQUIRED_INCLUDES
  ${LUAJIT_DIR}
  ${LUAJIT_DIR}/src
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

# LuaJIT specific
option(LUAJIT_DISABLE_FFI "Disable FFI." OFF)
option(LUAJIT_ENABLE_LUA52COMPAT "Enable Lua 5.2 compatibility." ON)
option(LUAJIT_DISABLE_JIT "Disable JIT." OFF)
option(LUAJIT_CPU_SSE2 "Use SSE2 instead of x87 instructions." ON)
option(LUAJIT_CPU_NOCMOV "Disable NOCMOV." OFF)
MARK_AS_ADVANCED(LUAJIT_DISABLE_FFI LUAJIT_ENABLE_LUA52COMPAT LUAJIT_DISABLE_JIT LUAJIT_CPU_SSE2 LUAJIT_CPU_NOCMOV)

IF(LUAJIT_DISABLE_FFI)
  ADD_DEFINITIONS(-DLUAJIT_DISABLE_FFI)
ENDIF()

IF(LUAJIT_ENABLE_LUA52COMPAT)
  ADD_DEFINITIONS(-DLUAJIT_ENABLE_LUA52COMPAT)
ENDIF()

IF(LUAJIT_DISABLE_JIT)
  ADD_DEFINITIONS(-DLUAJIT_DISABLE_JIT)
ENDIF()

IF(LUAJIT_CPU_SSE2)
  ADD_DEFINITIONS(-DLUAJIT_CPU_SSE2)
ENDIF()

IF(LUAJIT_CPU_NOCMOV)
  ADD_DEFINITIONS(-DLUAJIT_CPU_NOCMOV)
ENDIF()
######


CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P)
IF(SIZEOF_VOID_P EQUAL 8)
  ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
ENDIF()

if ( WIN32 AND NOT CYGWIN )
  add_definitions ( -DLUAJIT_OS=LUAJIT_OS_WINDOWS)
  set ( LJVM_MODE coffasm )
elseif ( APPLE )
  set ( CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}" )
  set ( LJVM_MODE machasm )
else ()
  set ( LJVM_MODE elfasm )
endif ()

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
MACRO(LJ_TEST_ARCH stuff)
  CHECK_C_SOURCE_COMPILES("
#undef ${stuff}
#include \"lj_arch.h\"
#if ${stuff}
int main() { return 0; }
#else
#error \"not defined\"
#endif
" ${stuff})
ENDMACRO()

MACRO(LJ_TEST_ARCH_VALUE stuff value)
  CHECK_C_SOURCE_COMPILES("
#undef ${stuff}
#include \"lj_arch.h\"
#if ${stuff} == ${value}
int main() { return 0; }
#else
#error \"not defined\"
#endif
" ${stuff}_${value})
ENDMACRO()


FOREACH(arch X64 X86 ARM PPC PPCSPE MIPS)
  LJ_TEST_ARCH(LJ_TARGET_${arch})
  if(LJ_TARGET_${arch})
    STRING(TOLOWER ${arch} TARGET_LJARCH)
    MESSAGE(STATUS "LuaJIT Target: ${TARGET_LJARCH}")
    BREAK()
  ENDIF()
ENDFOREACH()

IF(NOT TARGET_LJARCH)
  MESSAGE(FATAL_ERROR "architecture not supported")
ELSE()
  MESSAGE(STATUS "LuaJIT target ${TARGET_LJARCH}")
ENDIF()

FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/jit)
FILE(GLOB jit_files ${LUAJIT_DIR}/src/jit/*.lua)
FILE(COPY ${jit_files} DESTINATION ${CMAKE_BINARY_DIR}/jit)

SET(DASM_ARCH ${TARGET_LJARCH})
SET(DASM_FLAGS)
SET(TARGET_ARCH)
LIST(APPEND TARGET_ARCH "LUAJIT_TARGET=LUAJIT_ARCH_${TARGET_LJARCH}")
LJ_TEST_ARCH_VALUE(LJ_ARCH_BITS 64)
IF(LJ_ARCH_BITS_64)
  SET(DASM_FLAGS ${DASM_FLAGS} -D P64)
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_HASJIT 1)
IF(LJ_HASJIT_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D JIT)
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_HASFFI 1)
IF(LJ_HASFFI_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D FFI)
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_DUALNUM 1)
IF(LJ_DUALNUM_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D DUALNUM)
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_ARCH_HASFPU 1)
IF(LJ_ARCH_HASFPU_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D FPU)
  LIST(APPEND TARGET_ARCH "LJ_ARCH_HASFPU=1")
ELSE()
  LIST(APPEND TARGET_ARCH "LJ_ARCH_HASFPU=0")
ENDIF()
LJ_TEST_ARCH_VALUE(LJ_ABI_SOFTFP 1)
IF(NOT LJ_ABI_SOFTFP_1)
  SET(DASM_FLAGS ${DASM_FLAGS} -D HFABI)
  LIST(APPEND TARGET_ARCH "LJ_ABI_SOFTFP=0")
ELSE()
  LIST(APPEND TARGET_ARCH "LJ_ABI_SOFTFP=1")
ENDIF()
IF(WIN32)
  SET(DASM_FLAGS ${DASM_FLAGS} -LN -D WIN)
ENDIF()
IF(TARGET_LJARCH STREQUAL "x86")
  LJ_TEST_ARCH_VALUE(__SSE2__ 1)
  IF(__SSE2__1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D SSE)
  ENDIF()
ENDIF()
IF(TARGET_LJARCH STREQUAL "x64")
  SET(DASM_ARCH "x86")
ENDIF()
IF(TARGET_LJARCH STREQUAL "ppc")
  LJ_TEST_ARCH_VALUE(LJ_ARCH_SQRT 1)
  IF(NOT LJ_ARCH_SQRT_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D SQRT)
  ENDIF()
  LJ_TEST_ARCH_VALUE(LJ_ARCH_PPC64 1)
  IF(NOT LJ_ARCH_PPC64_1)
    SET(DASM_FLAGS ${DASM_FLAGS} -D GPR64)
  ENDIF()
ENDIF()

add_executable(minilua ${LUAJIT_DIR}/src/host/minilua.c)
SET_TARGET_PROPERTIES(minilua PROPERTIES COMPILE_DEFINITIONS "${TARGET_ARCH}")
CHECK_LIBRARY_EXISTS(m sin "" MINILUA_USE_LIBM)
if(MINILUA_USE_LIBM)
  TARGET_LINK_LIBRARIES(minilua m)
endif()

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h
  COMMAND minilua ${LUAJIT_DIR}/dynasm/dynasm.lua ${DASM_FLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h ${LUAJIT_DIR}/src/vm_${DASM_ARCH}.dasc
  DEPENDS ${LUAJIT_DIR}/dynasm/dynasm.lua minilua
)

SET(SRC_LJLIB
  ${LUAJIT_DIR}/src/lib_base.c
  ${LUAJIT_DIR}/src/lib_math.c
  ${LUAJIT_DIR}/src/lib_bit.c
  ${LUAJIT_DIR}/src/lib_string.c
  ${LUAJIT_DIR}/src/lib_table.c
  ${LUAJIT_DIR}/src/lib_io.c
  ${LUAJIT_DIR}/src/lib_os.c
  ${LUAJIT_DIR}/src/lib_package.c
  ${LUAJIT_DIR}/src/lib_debug.c
  ${LUAJIT_DIR}/src/lib_jit.c
  ${LUAJIT_DIR}/src/lib_ffi.c)

SET(SRC_LJCORE
  ${LUAJIT_DIR}/src/lj_gc.c
  ${LUAJIT_DIR}/src/lj_err.c
  ${LUAJIT_DIR}/src/lj_char.c
  ${LUAJIT_DIR}/src/lj_buf.c
  ${LUAJIT_DIR}/src/lj_profile.c
  ${LUAJIT_DIR}/src/lj_strfmt.c
  ${LUAJIT_DIR}/src/lj_bc.c
  ${LUAJIT_DIR}/src/lj_obj.c
  ${LUAJIT_DIR}/src/lj_str.c
  ${LUAJIT_DIR}/src/lj_tab.c
  ${LUAJIT_DIR}/src/lj_func.c
  ${LUAJIT_DIR}/src/lj_udata.c
  ${LUAJIT_DIR}/src/lj_meta.c
  ${LUAJIT_DIR}/src/lj_debug.c
  ${LUAJIT_DIR}/src/lj_state.c
  ${LUAJIT_DIR}/src/lj_dispatch.c
  ${LUAJIT_DIR}/src/lj_vmevent.c
  ${LUAJIT_DIR}/src/lj_vmmath.c
  ${LUAJIT_DIR}/src/lj_strscan.c
  ${LUAJIT_DIR}/src/lj_api.c
  ${LUAJIT_DIR}/src/lj_lex.c
  ${LUAJIT_DIR}/src/lj_parse.c
  ${LUAJIT_DIR}/src/lj_bcread.c
  ${LUAJIT_DIR}/src/lj_bcwrite.c
  ${LUAJIT_DIR}/src/lj_load.c
  ${LUAJIT_DIR}/src/lj_ir.c
  ${LUAJIT_DIR}/src/lj_opt_mem.c
  ${LUAJIT_DIR}/src/lj_opt_fold.c
  ${LUAJIT_DIR}/src/lj_opt_narrow.c
  ${LUAJIT_DIR}/src/lj_opt_dce.c
  ${LUAJIT_DIR}/src/lj_opt_loop.c
  ${LUAJIT_DIR}/src/lj_opt_split.c
  ${LUAJIT_DIR}/src/lj_opt_sink.c
  ${LUAJIT_DIR}/src/lj_mcode.c
  ${LUAJIT_DIR}/src/lj_snap.c
  ${LUAJIT_DIR}/src/lj_record.c
  ${LUAJIT_DIR}/src/lj_crecord.c
  ${LUAJIT_DIR}/src/lj_ffrecord.c
  ${LUAJIT_DIR}/src/lj_asm.c
  ${LUAJIT_DIR}/src/lj_trace.c
  ${LUAJIT_DIR}/src/lj_gdbjit.c
  ${LUAJIT_DIR}/src/lj_ctype.c
  ${LUAJIT_DIR}/src/lj_cdata.c
  ${LUAJIT_DIR}/src/lj_cconv.c
  ${LUAJIT_DIR}/src/lj_ccall.c
  ${LUAJIT_DIR}/src/lj_ccallback.c
  ${LUAJIT_DIR}/src/lj_carith.c
  ${LUAJIT_DIR}/src/lj_clib.c
  ${LUAJIT_DIR}/src/lj_cparse.c
  ${LUAJIT_DIR}/src/lj_lib.c
  ${LUAJIT_DIR}/src/lj_alloc.c
  ${LUAJIT_DIR}/src/lj_vmmath.c
  ${LUAJIT_DIR}/src/lib_aux.c
  ${LUAJIT_DIR}/src/lib_init.c
  ${SRC_LJLIB})

SET(SRC_BUILDVM
  ${LUAJIT_DIR}/src/host/buildvm.c
  ${LUAJIT_DIR}/src/host/buildvm_asm.c
  ${LUAJIT_DIR}/src/host/buildvm_peobj.c
  ${LUAJIT_DIR}/src/host/buildvm_lib.c
  ${LUAJIT_DIR}/src/host/buildvm_fold.c
  ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h)

## GENERATE
ADD_EXECUTABLE(buildvm ${SRC_BUILDVM})
SET_TARGET_PROPERTIES(buildvm PROPERTIES COMPILE_DEFINITIONS "${TARGET_ARCH}")

macro(add_buildvm_target _target _mode)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_target}
    COMMAND buildvm ARGS -m ${_mode} -o ${CMAKE_CURRENT_BINARY_DIR}/${_target} ${ARGN}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS buildvm ${ARGN}
  )
endmacro(add_buildvm_target)

if (MSVC)
  add_buildvm_target ( lj_vm.obj peobj )
  set (LJ_VM_SRC ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.obj)
else ()
  add_buildvm_target ( lj_vm.S ${LJVM_MODE} )
  set (LJ_VM_SRC ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.S)
endif ()
add_buildvm_target ( lj_ffdef.h   ffdef   ${SRC_LJLIB} )
add_buildvm_target ( lj_bcdef.h  bcdef  ${SRC_LJLIB} )
add_buildvm_target ( lj_folddef.h folddef ${LUAJIT_DIR}/src/lj_opt_fold.c )
add_buildvm_target ( lj_recdef.h  recdef  ${SRC_LJLIB} )
add_buildvm_target ( lj_libdef.h  libdef  ${SRC_LJLIB} )
add_buildvm_target ( vmdef.lua  vmdef  ${SRC_LJLIB} )

SET(DEPS
  ${LJ_VM_SRC}
  ${CMAKE_CURRENT_BINARY_DIR}/lj_ffdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_bcdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_libdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_recdef.h
  ${CMAKE_CURRENT_BINARY_DIR}/lj_folddef.h
  ${CMAKE_CURRENT_BINARY_DIR}/vmdef.lua
  )

## COMPILE
include_directories(
  ${LUAJIT_DIR}/dynasm
  ${LUAJIT_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
)

IF(WITH_SHARED_LUA)
    IF(WITH_AMALG)
	add_library(luajit-5.1 SHARED ${LUAJIT_DIR}/src/ljamalg.c ${DEPS} )
    ELSE()
	add_library(luajit-5.1 SHARED ${SRC_LJCORE} ${DEPS} )
    ENDIF()
    SET_TARGET_PROPERTIES(luajit-5.1 PROPERTIES OUTPUT_NAME "lua51")
ELSE()
    IF(WITH_AMALG)
	add_library(luajit-5.1 STATIC ${LUAJIT_DIR}/src/ljamalg.c ${DEPS} )
    ELSE()
	add_library(luajit-5.1 STATIC ${SRC_LJCORE} ${DEPS} )
    ENDIF()
    SET_TARGET_PROPERTIES(luajit-5.1 PROPERTIES
	PREFIX "lib" IMPORT_PREFIX "lib" OUTPUT_NAME "luajit")
ENDIF()

target_link_libraries (luajit-5.1 ${LIBS} )

IF(WIN32)
  add_executable(luajit ${LUAJIT_DIR}/src/luajit.c)
  target_link_libraries(luajit luajit-5.1)
ELSE()
  IF(WITH_AMALG)
    add_executable(luajit ${LUAJIT_DIR}/src/luajit.c ${LUAJIT_DIR}/src/ljamalg.c ${DEPS})
  ELSE()
    add_executable(luajit ${LUAJIT_DIR}/src/luajit.c ${SRC_LJCORE} ${DEPS})
  ENDIF()
  target_link_libraries(luajit ${LIBS})
  SET_TARGET_PROPERTIES(luajit PROPERTIES ENABLE_EXPORTS ON)
ENDIF()

MACRO(LUAJIT_add_custom_commands luajit_target)
  SET(target_srcs "")
  FOREACH(file ${ARGN})
    IF(${file} MATCHES ".*\\.lua$")
      set(file "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
      set(source_file ${file})
      string(LENGTH ${CMAKE_SOURCE_DIR} _luajit_source_dir_length)
      string(LENGTH ${file} _luajit_file_length)
      math(EXPR _begin "${_luajit_source_dir_length} + 1")
      math(EXPR _stripped_file_length "${_luajit_file_length} - ${_luajit_source_dir_length} - 1")
      string(SUBSTRING ${file} ${_begin} ${_stripped_file_length} stripped_file)

      set(generated_file "${CMAKE_BINARY_DIR}/jitted_tmp/${stripped_file}_${luajit_target}_generated${CMAKE_C_OUTPUT_EXTENSION}")

      add_custom_command(
        OUTPUT ${generated_file}
        MAIN_DEPENDENCY ${source_file}
        DEPENDS luajit
        COMMAND luajit
        ARGS -bg
          ${source_file}
          ${generated_file}
        COMMENT "Building Luajitted ${source_file}: ${generated_file}"
      )

      get_filename_component(basedir ${generated_file} PATH)
      file(MAKE_DIRECTORY ${basedir})

      set(target_srcs ${target_srcs} ${generated_file})
      set_source_files_properties(
        ${generated_file}
        properties
        external_object true # this is an object file
        generated true        # to say that "it is OK that the obj-files do not exist before build time"
      )
    ELSE()
      set(target_srcs ${target_srcs} ${file})
    ENDIF(${file} MATCHES ".*\\.lua$")
  ENDFOREACH(file)
ENDMACRO()

MACRO(LUAJIT_ADD_EXECUTABLE luajit_target)
  LUAJIT_add_custom_commands(${luajit_target} ${ARGN})
  add_executable(${luajit_target} ${target_srcs})
ENDMACRO(LUAJIT_ADD_EXECUTABLE luajit_target)
