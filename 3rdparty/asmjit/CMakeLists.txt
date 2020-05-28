cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

cmake_policy(PUSH)
cmake_policy(SET CMP0063 NEW) # Honor visibility properties.

include(CheckCXXCompilerFlag)

# Don't create a project if it was already created by another CMakeLists.txt.
# This allows one library to embed another library without making a collision.
if (NOT CMAKE_PROJECT_NAME OR "${CMAKE_PROJECT_NAME}" STREQUAL "asmjit")
  project(asmjit CXX)
endif()

# =============================================================================
# [AsmJit - Deprecated]
# =============================================================================

if (DEFINED ASMJIT_BUILD_EMBED)
  message(DEPRECATION "ASMJIT_BUILD_EMBED is deprecated, use ASMJIT_EMBED")
  set(ASMJIT_EMBED "${ASMJIT_BUILD_EMBED}")
endif()

if (DEFINED ASMJIT_BUILD_STATIC)
  message(DEPRECATION "ASMJIT_BUILD_STATIC is deprecated, use ASMJIT_STATIC")
  set(ASMJIT_STATIC "${ASMJIT_BUILD_STATIC}")
endif()

# =============================================================================
# [AsmJit - Configuration]
# =============================================================================

if (NOT DEFINED ASMJIT_EMBED)
  set(ASMJIT_EMBED FALSE)
endif()

if (NOT DEFINED ASMJIT_STATIC)
  set(ASMJIT_STATIC ${ASMJIT_EMBED})
endif()

if (NOT DEFINED ASMJIT_BUILD_ARM)
  set(ASMJIT_BUILD_ARM FALSE)
endif()

if (NOT DEFINED ASMJIT_BUILD_X86)
  set(ASMJIT_BUILD_X86 FALSE)
endif()

if (NOT DEFINED ASMJIT_TEST)
  set(ASMJIT_TEST FALSE)
endif()

if (NOT DEFINED ASMJIT_NO_NATVIS)
  set(ASMJIT_NO_NATVIS FALSE)
endif()

# EMBED implies STATIC.
if (ASMJIT_EMBED AND NOT ASMJIT_STATIC)
  set(ASMJIT_STATIC TRUE)
endif()

set(ASMJIT_DIR        "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "Location of 'asmjit'")
set(ASMJIT_TEST       ${ASMJIT_TEST}              CACHE BOOL "Build 'asmjit' test applications")
set(ASMJIT_EMBED      ${ASMJIT_EMBED}             CACHE BOOL "Embed 'asmjit' library (no targets)")
set(ASMJIT_STATIC     ${ASMJIT_STATIC}            CACHE BOOL "Build 'asmjit' library as static")
set(ASMJIT_SANITIZE   ${ASMJIT_SANITIZE}          CACHE BOOL "Build with C/C++ sanitizers enabled")
set(ASMJIT_BUILD_X86  ${ASMJIT_BUILD_X86}         CACHE BOOL "Build X86 backends (X86 and X86_64)")
set(ASMJIT_BUILD_ARM  ${ASMJIT_BUILD_ARM}         CACHE BOOL "Build ARM backends")

# =============================================================================
# [AsmJit - Project]
# =============================================================================

set(ASMJIT_INCLUDE_DIRS "${ASMJIT_DIR}/src")     # Include directory is the same as source dir.
set(ASMJIT_DEPS "")                              # AsmJit dependencies (libraries) for the linker.
set(ASMJIT_LIBS "")                              # Dependencies of libs/apps that want to use AsmJit.
set(ASMJIT_CFLAGS "")                            # Public compiler flags.
set(ASMJIT_PRIVATE_CFLAGS "")                    # Private compiler flags independent of build type.
set(ASMJIT_PRIVATE_CFLAGS_DBG "")                # Private compiler flags used by debug builds.
set(ASMJIT_PRIVATE_CFLAGS_REL "")                # Private compiler flags used by release builds.
set(ASMJIT_SANITIZE_CFLAGS "")                   # Compiler flags required by currently enabled sanitizers.
set(ASMJIT_SANITIZE_LFLAGS "")                   # Linker flags required by currently enabled sanitizers.

# =============================================================================
# [AsmJit - Utilities]
# =============================================================================

function(asmjit_detect_cflags out)
  set(out_array ${${out}})
  foreach(flag ${ARGN})
    string(REGEX REPLACE "[+]" "x" flag_signature "${flag}")
    string(REGEX REPLACE "[-=:;/.\]" "_" flag_signature "${flag_signature}")
    check_cxx_compiler_flag(${flag} "__CxxFlag_${flag_signature}")
    if (${__CxxFlag_${flag_signature}})
      list(APPEND out_array "${flag}")
    endif()
  endforeach()
  set(${out} "${out_array}" PARENT_SCOPE)
endfunction()

# Support for various sanitizers provided by C/C++ compilers.
function(asmjit_detect_sanitizers out)
  set(_out_array ${${out}})
  set(_flags "")

  foreach(_arg ${ARGN})
    string(REPLACE "," ";" _arg "${_arg}")
    list(APPEND _flags ${_arg})
  endforeach()

  foreach(_flag ${_flags})
    if (NOT "${_flag}" MATCHES "^-fsanitize=")
      SET(_flag "-fsanitize=${_flag}")
    endif()

    # Sanitizers also require link flags, see CMAKE_REQUIRED_FLAGS.
    set(CMAKE_REQUIRED_FLAGS "${_flag}")
    asmjit_detect_cflags(_out_array ${_flag})
    unset(CMAKE_REQUIRED_FLAGS)
  endforeach()

  set(${out} "${_out_array}" PARENT_SCOPE)
endfunction()

function(asmjit_add_target target target_type)
  set(single_val "")
  set(multi_val SOURCES LIBRARIES CFLAGS CFLAGS_DBG CFLAGS_REL)
  cmake_parse_arguments("X" "" "${single_val}" "${multi_val}" ${ARGN})

  if ("${target_type}" MATCHES "^(EXECUTABLE|TEST)$")
    add_executable(${target} ${X_SOURCES})
  else()
    add_library(${target} ${target_type} ${X_SOURCES})
  endif()

  target_link_libraries(${target} PRIVATE ${X_LIBRARIES})

  # target_link_options was added in cmake v3.13, don't use it for now...
  foreach(link_flag ${ASMJIT_SANITIZE_LFLAGS})
    set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " ${link_flag}")
  endforeach()

  if (${CMAKE_VERSION} VERSION_LESS "3.8.0")
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
  else()
    target_compile_features(${target} PUBLIC cxx_std_11)
  endif()
  set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS NO)
  set_property(TARGET ${target} PROPERTY CXX_VISIBILITY_PRESET hidden)
  target_compile_options(${target} PRIVATE ${X_CFLAGS} ${ASMJIT_SANITIZE_CFLAGS} $<$<CONFIG:Debug>:${X_CFLAGS_DBG}> $<$<NOT:$<CONFIG:Debug>>:${X_CFLAGS_REL}>)

  if ("${target_type}" STREQUAL "TEST")
    add_test(NAME ${target} COMMAND ${target})
  endif()
endfunction()

# =============================================================================
# [AsmJit - Compiler Support]
# =============================================================================

set(ASMJIT_INCLUDE_DIRS "${ASMJIT_DIR}/src")     # Include directory is the same as source dir.
set(ASMJIT_DEPS "")                              # AsmJit dependencies (libraries) for the linker.
set(ASMJIT_LIBS "")                              # Dependencies of libs/apps that want to use AsmJit.
set(ASMJIT_CFLAGS "")                            # Public compiler flags.
set(ASMJIT_PRIVATE_CFLAGS "")                    # Private compiler flags independent of build type.
set(ASMJIT_PRIVATE_CFLAGS_DBG "")                # Private compiler flags used by debug builds.
set(ASMJIT_PRIVATE_CFLAGS_REL "")                # Private compiler flags used by release builds.
set(ASMJIT_SANITIZE_CFLAGS "")                   # Compiler flags required by currently enabled sanitizers.
set(ASMJIT_SANITIZE_LFLAGS "")                   # Linker flags required by currently enabled sanitizers.

# TODO: Backward compatibility.
set(ASMJIT_INCLUDE_DIR "${ASMJIT_INCLUDE_DIRS}")

if (NOT ASMJIT_NO_CUSTOM_FLAGS)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    list(APPEND ASMJIT_PRIVATE_CFLAGS
      -MP                      # [+] Multi-Process Compilation.
      -GR-                     # [-] Runtime type information.
      -GF                      # [+] Eliminate duplicate strings.
      -Zc:inline               # [+] Remove unreferenced COMDAT.
      -Zc:strictStrings        # [+] Strict const qualification of string literals.
      -Zc:threadSafeInit-      # [-] Thread-safe statics.
      -W4)                     # [+] Warning level 4.

    list(APPEND ASMJIT_PRIVATE_CFLAGS_DBG
      -GS)                     # [+] Buffer security-check.

    list(APPEND ASMJIT_PRIVATE_CFLAGS_REL
      -GS-                     # [-] Buffer security-check.
      -O2                      # [+] Favor speed over size.
      -Oi)                     # [+] Generate intrinsic functions.
  elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang)$")
    list(APPEND ASMJIT_PRIVATE_CFLAGS -Wall -Wextra)
    list(APPEND ASMJIT_PRIVATE_CFLAGS -fno-math-errno)
    list(APPEND ASMJIT_PRIVATE_CFLAGS_REL -O2)

    asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS
      -fno-threadsafe-statics
      -fno-semantic-interposition)

    asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS_REL
      -fmerge-all-constants)
  endif()
endif()

# Support for sanitizers.
if (ASMJIT_SANITIZE)
  ASMJIT_detect_sanitizers(ASMJIT_SANITIZE_CFLAGS ${ASMJIT_SANITIZE})
  if (ASMJIT_SANITIZE_CFLAGS)
    message("-- Enabling sanitizers: '${ASMJIT_SANITIZE_CFLAGS}'")

    # Linker must receive the same flags as the compiler when it comes to sanitizers.
    set(ASMJIT_SANITIZE_LFLAGS ${ASMJIT_SANITIZE_CFLAGS})

    # Don't omit frame pointer if sanitizers are enabled.
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
      list(APPEND ASMJIT_SANITIZE_CFLAGS -Oy-)
    else()
      list(APPEND ASMJIT_SANITIZE_CFLAGS -fno-omit-frame-pointer -g)
    endif()

    list(APPEND ASMJIT_PRIVATE_CFLAGS ${ASMJIT_SANITIZE_CFLAGS})
    list(APPEND ASMJIT_PRIVATE_LFLAGS ${ASMJIT_SANITIZE_LFLAGS})
  endif()
endif()

if (NOT WIN32)
  list(APPEND ASMJIT_DEPS pthread)
endif()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  list(APPEND ASMJIT_DEPS rt)
endif()

set(ASMJIT_LIBS ${ASMJIT_DEPS})
if (NOT ASMJIT_EMBED)
  list(INSERT ASMJIT_LIBS 0 asmjit)
endif()

if (ASMJIT_EMBED)
  set(ASMJIT_TARGET_TYPE "EMBED")
elseif (ASMJIT_STATIC)
  set(ASMJIT_TARGET_TYPE "STATIC")
else()
  set(ASMJIT_TARGET_TYPE "SHARED")
endif()

foreach(build_option ASMJIT_STATIC
                     ASMJIT_BUILD_X86
                     #ASMJIT_BUILD_ARM
                     ASMJIT_BUILD_A64
                     ASMJIT_NO_JIT
                     ASMJIT_NO_LOGGING
                     ASMJIT_NO_BUILDER
                     ASMJIT_NO_COMPILER
                     ASMJIT_NO_TEXT
                     ASMJIT_NO_VALIDATION
                     ASMJIT_NO_INTROSPECTION)
  if (${build_option})
    List(APPEND ASMJIT_CFLAGS         "-D${build_option}")
    List(APPEND ASMJIT_PRIVATE_CFLAGS "-D${build_option}")
  endif()
endforeach()

# =============================================================================
# [AsmJit - Linker Support]
# =============================================================================

if (WIN32)
  if(CMAKE_LINKER MATCHES "link\\.exe" OR CMAKE_LINKER MATCHES "lld-link\\.exe")
    set(ASMJIT_LINKER_SUPPORTS_NATVIS TRUE)
  endif()
endif()

# =============================================================================
# [AsmJit - Source]
# =============================================================================

set(ASMJIT_SRC_LIST
  asmjit/asmjit.h

  asmjit/core.h
  asmjit/core/api-build_p.h
  asmjit/core/api-config.h
  asmjit/core/arch.cpp
  asmjit/core/arch.h
  asmjit/core/assembler.cpp
  asmjit/core/assembler.h
  asmjit/core/builder.cpp
  asmjit/core/builder.h
  asmjit/core/callconv.cpp
  asmjit/core/callconv.h
  asmjit/core/codebufferwriter_p.h
  asmjit/core/codeholder.cpp
  asmjit/core/codeholder.h
  asmjit/core/compiler.cpp
  asmjit/core/compiler.h
  asmjit/core/constpool.cpp
  asmjit/core/constpool.h
  asmjit/core/cpuinfo.cpp
  asmjit/core/cpuinfo.h
  asmjit/core/datatypes.h
  asmjit/core/emitter.cpp
  asmjit/core/emitter.h
  asmjit/core/features.h
  asmjit/core/func.cpp
  asmjit/core/func.h
  asmjit/core/globals.cpp
  asmjit/core/globals.h
  asmjit/core/inst.cpp
  asmjit/core/inst.h
  asmjit/core/jitallocator.cpp
  asmjit/core/jitallocator.h
  asmjit/core/jitruntime.cpp
  asmjit/core/jitruntime.h
  asmjit/core/logging.cpp
  asmjit/core/logging.h
  asmjit/core/misc_p.h
  asmjit/core/operand.cpp
  asmjit/core/operand.h
  asmjit/core/osutils.cpp
  asmjit/core/osutils.h
  asmjit/core/raassignment_p.h
  asmjit/core/rabuilders_p.h
  asmjit/core/radefs_p.h
  asmjit/core/ralocal.cpp
  asmjit/core/ralocal_p.h
  asmjit/core/rapass.cpp
  asmjit/core/rapass_p.h
  asmjit/core/rastack.cpp
  asmjit/core/rastack_p.h
  asmjit/core/string.cpp
  asmjit/core/string.h
  asmjit/core/support.cpp
  asmjit/core/support.h
  asmjit/core/target.cpp
  asmjit/core/target.h
  asmjit/core/type.cpp
  asmjit/core/type.h
  asmjit/core/virtmem.cpp
  asmjit/core/virtmem.h
  asmjit/core/zone.cpp
  asmjit/core/zone.h
  asmjit/core/zonehash.cpp
  asmjit/core/zonehash.h
  asmjit/core/zonelist.cpp
  asmjit/core/zonelist.h
  asmjit/core/zonestack.cpp
  asmjit/core/zonestack.h
  asmjit/core/zonestring.h
  asmjit/core/zonetree.cpp
  asmjit/core/zonetree.h
  asmjit/core/zonevector.cpp
  asmjit/core/zonevector.h

  asmjit/x86.h
  asmjit/x86/x86assembler.cpp
  asmjit/x86/x86assembler.h
  asmjit/x86/x86builder.cpp
  asmjit/x86/x86builder.h
  asmjit/x86/x86callconv.cpp
  asmjit/x86/x86callconv_p.h
  asmjit/x86/x86compiler.cpp
  asmjit/x86/x86compiler.h
  asmjit/x86/x86emitter.h
  asmjit/x86/x86features.cpp
  asmjit/x86/x86features.h
  asmjit/x86/x86globals.h
  asmjit/x86/x86internal.cpp
  asmjit/x86/x86internal_p.h
  asmjit/x86/x86instdb.cpp
  asmjit/x86/x86instdb.h
  asmjit/x86/x86instdb_p.h
  asmjit/x86/x86instapi.cpp
  asmjit/x86/x86instapi_p.h
  asmjit/x86/x86logging.cpp
  asmjit/x86/x86logging_p.h
  asmjit/x86/x86operand.cpp
  asmjit/x86/x86operand.h
  asmjit/x86/x86rapass.cpp
  asmjit/x86/x86rapass_p.h
)

if (MSVC AND NOT ASMJIT_NO_NATVIS)
  list(APPEND ASMJIT_SRC_LIST asmjit.natvis)
endif()

set(ASMJIT_SRC "")
foreach(src_file ${ASMJIT_SRC_LIST})
  set(src_file "${ASMJIT_DIR}/src/${src_file}")
  list(APPEND ASMJIT_SRC ${src_file})

  if ("${src_file}" MATCHES "\\.natvis")
    if (ASMJIT_LINKER_SUPPORTS_NATVIS)
      list(APPEND ASMJIT_PRIVATE_LFLAGS "-natvis:${src_file}")
    endif()
  endif()
endforeach()

if (NOT ${CMAKE_VERSION} VERSION_LESS "3.8.0")
  source_group(TREE "${ASMJIT_DIR}" FILES ${ASMJIT_SRC})
endif()

# =============================================================================
# [AsmJit - Summary]
# =============================================================================

message("** AsmJit Summary **")
message("   ASMJIT_DIR=${ASMJIT_DIR}")
message("   ASMJIT_TEST=${ASMJIT_TEST}")
message("   ASMJIT_TARGET_TYPE=${ASMJIT_TARGET_TYPE}")
message("   ASMJIT_DEPS=${ASMJIT_DEPS}")
message("   ASMJIT_LIBS=${ASMJIT_LIBS}")
message("   ASMJIT_CFLAGS=${ASMJIT_CFLAGS}")
message("   ASMJIT_PRIVATE_CFLAGS=${ASMJIT_PRIVATE_CFLAGS}")
message("   ASMJIT_PRIVATE_CFLAGS_DBG=${ASMJIT_PRIVATE_CFLAGS_DBG}")
message("   ASMJIT_PRIVATE_CFLAGS_REL=${ASMJIT_PRIVATE_CFLAGS_REL}")

# =============================================================================
# [AsmJit - Targets]
# =============================================================================

if (NOT ASMJIT_EMBED)
  # Add AsmJit target.
  asmjit_add_target(asmjit "${ASMJIT_TARGET_TYPE}"
                    SOURCES    ${ASMJIT_SRC}
                    LIBRARIES  ${ASMJIT_DEPS}
                    CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                    CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                    CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
  target_include_directories(asmjit BEFORE INTERFACE ${ASMJIT_INCLUDE_DIRS})
  target_compile_options(asmjit INTERFACE ${ASMJIT_CFLAGS})
  add_library(AsmJit::AsmJit ALIAS asmjit)

  # Add AsmJit install instructions (library and public headers).
  if (NOT ASMJIT_NO_INSTALL)
    install(TARGETS asmjit RUNTIME DESTINATION "bin"
                           LIBRARY DESTINATION "lib${LIB_SUFFIX}"
                           ARCHIVE DESTINATION "lib${LIB_SUFFIX}")
    foreach(_src_file ${ASMJIT_SRC_LIST})
      if ("${_src_file}" MATCHES "\\.h$" AND NOT "${_src_file}" MATCHES "_p\\.h$")
        get_filename_component(_src_dir ${_src_file} PATH)
        install(FILES "${ASMJIT_DIR}/src/${_src_file}" DESTINATION "include/${_src_dir}")
      endif()
    endforeach()
  endif()

  # Add AsmJit tests.
  if (ASMJIT_TEST)
    enable_testing()

    # Special target that always uses embedded AsmJit.
    asmjit_add_target(asmjit_test_unit TEST
                      SOURCES    ${ASMJIT_SRC}
                                 test/asmjit_test_unit.cpp
                                 test/broken.cpp
                                 test/broken.h
                      LIBRARIES  ${ASMJIT_DEPS}
                      CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                                 -DASMJIT_TEST
                                 -DASMJIT_STATIC
                      CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                      CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    target_include_directories(asmjit_test_unit BEFORE PRIVATE ${ASMJIT_INCLUDE_DIRS})

    foreach(_target asmjit_test_opcode
                    asmjit_test_x86_asm
                    asmjit_test_x86_sections)
      asmjit_add_target(${_target} TEST
                        SOURCES    test/${_target}.cpp
                        LIBRARIES  AsmJit::AsmJit
                        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endforeach()

    if (NOT (ASMJIT_NO_BUILDER OR ASMJIT_NO_COMPILER))
      asmjit_add_target(asmjit_test_x86_cc TEST
                        SOURCES    test/asmjit_test_x86_cc.cpp
                        LIBRARIES  AsmJit::AsmJit
                        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endif()

    foreach(_target asmjit_bench_x86)
      asmjit_add_target(${_target} EXECUTABLE
                        SOURCES    test/${_target}.cpp
                        LIBRARIES  AsmJit::AsmJit
                        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
                        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
                        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endforeach()
  endif()
endif()

cmake_policy(POP)
