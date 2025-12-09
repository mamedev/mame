#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2010      Patrick Spendrin <ps_ml@gmx.de>
# Copyright (c) 2012      Karl Waclawek <karl@waclawek.net>
# Copyright (c) 2016-2025 Sebastian Pipping <sebastian@pipping.org>
# Copyright (c) 2016      Sergei Nikulov <sergey.nikulov@gmail.com>
# Copyright (c) 2016      Björn Lindahl <bjorn.lindahl@foi.se>
# Copyright (c) 2016      Tobias Taschner <github@tc84.de>
# Copyright (c) 2016      Ben Boeckel <ben.boeckel@kitware.com>
# Copyright (c) 2017-2022 Rhodri James <rhodri@wildebeest.org.uk>
# Copyright (c) 2017      Rolf Eike Beer <eike@sf-mail.de>
# Copyright (c) 2017      Stephen Groat <stephen@groat.us>
# Copyright (c) 2017      Franek Korta <fkorta@gmail.com>
# Copyright (c) 2018      pedro-vicente <pedro.vicente@space-research.org>
# Copyright (c) 2018      Frank Rast <frank.rast@gefeg.com>
# Copyright (c) 2018      userwithuid <userwithuid@gmail.com>
# Copyright (c) 2018      Yury Gribov <tetra2005@gmail.com>
# Copyright (c) 2019      Kishore Kunche <kishore.kunche@intel.com>
# Copyright (c) 2019      xantares <xantares09@hotmail.com>
# Copyright (c) 2019      Mohammed Khajapasha <mohammed.khajapasha@intel.com>
# Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
# Copyright (c) 2019      Bhargava Shastry <bhargava.shastry@ethereum.org>
# Copyright (c) 2020      Maciej Sroczyński <macieksroczynski@poczta.fm>
# Copyright (c) 2020      Gulliver <gulliver@traumkristalle.net>
# Copyright (c) 2020      Thomas Beutlich <tc@tbeu.de>
# Copyright (c) 2021      Alex Richardson <Alexander.Richardson@cl.cam.ac.uk>
# Copyright (c) 2022      Johnny Jazeix <jazeix@gmail.com>
# Copyright (c) 2022      Mark Brand <markbrand@google.com>
# Copyright (c) 2022      David Faure <david.faure@kdab.com>
# Unlike most of Expat,
# this file is copyrighted under the BSD-license for buildsystem files of KDE.

cmake_minimum_required(VERSION 3.13.0)

project(expat
    VERSION
        2.7.1
    LANGUAGES
        C
)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)  # i.e. -std=c99 rather than default -std=gnu99

set(PACKAGE_BUGREPORT "https://github.com/libexpat/libexpat/issues")
set(PACKAGE_NAME "expat")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE_NAME}")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

#
# Configuration defaults
#
if(WINCE)
    set(_EXPAT_BUILD_TOOLS_DEFAULT OFF)
else()
    set(_EXPAT_BUILD_TOOLS_DEFAULT ON)
endif()
if(MSVC OR NOT _EXPAT_BUILD_TOOLS_DEFAULT)
    set(_EXPAT_BUILD_DOCS_DEFAULT OFF)
else()
    find_program(DOCBOOK_TO_MAN NAMES docbook2x-man db2x_docbook2man docbook2man docbook-to-man)
    if(DOCBOOK_TO_MAN)
        set(_EXPAT_BUILD_DOCS_DEFAULT ON)
    else()
        set(_EXPAT_BUILD_DOCS_DEFAULT OFF)
    endif()
endif()
if(MSVC)
    set(_EXPAT_BUILD_PKGCONFIG_DEFAULT OFF)
else()
    set(_EXPAT_BUILD_PKGCONFIG_DEFAULT ON)
endif()
if(DEFINED BUILD_SHARED_LIBS)
    set(_EXPAT_SHARED_LIBS_DEFAULT ${BUILD_SHARED_LIBS})
else()
    set(_EXPAT_SHARED_LIBS_DEFAULT ON)
endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE NoConfig)  # so that accessing EXPAT_*_POSTFIX will be waterproof
endif()
string(TOUPPER "${CMAKE_BUILD_TYPE}" _EXPAT_BUILD_TYPE_UPPER)

#
# Configuration
#

macro(expat_shy_set var default cache type desc)
    # Macro expat_shy_set came into life because:
    # - Expat was previously using an inconsistent mix of CMake's native set()
    #   and option() to define public build time options.
    # - option() is more friendly than set() with regard to configuring an
    #   external project that is pulled in by means of add_subdirectory() --
    #   see comments in issue #597 -- so we wanted to get away from set().
    # - option() auto-converts non-bool values to bool when writing to the CMake
    #   cache, so we needed something that supports non-bool better and hence
    #   wanted to get away from plain option(), too.
    #
    # As a result, this function serves as a hybrid between CMake's regular set()
    # and option(): from set() it takes support for non-bool types and the function
    # name and signature whereas from option() (with policy CMP0077 mode NEW) it
    # takes being shy when a value has previously been defined for that variable.
    #
    # So that resolves all need for set(.. FORCE) when pulling in Expat by means of
    # add_subdirectory().
    #
    if(NOT ${cache} STREQUAL "CACHE")
        message(SEND_ERROR "Macro usage is: expat_shy_set(var default CACHE type desc)")
    endif()

    if(DEFINED ${var})
        # NOTE: The idea is to (ideally) only add to the cache if
        #       there is no cache entry, yet.  "if(DEFINED CACHE{var})"
        #       requires CMake >=3.14.
        if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14" AND NOT DEFINED "CACHE{${var}}")
            set("${var}" "${${var}}" CACHE "${type}" "${desc}")
        endif()
    else()
        set("${var}" "${default}" CACHE "${type}" "${desc}")
    endif()
endmacro()

expat_shy_set(EXPAT_BUILD_TOOLS ${_EXPAT_BUILD_TOOLS_DEFAULT} CACHE BOOL "Build the xmlwf tool for expat library")
expat_shy_set(EXPAT_BUILD_EXAMPLES ON CACHE BOOL "Build the examples for expat library")
expat_shy_set(EXPAT_BUILD_TESTS ON CACHE BOOL "Build the tests for expat library")
expat_shy_set(EXPAT_SHARED_LIBS ${_EXPAT_SHARED_LIBS_DEFAULT} CACHE BOOL "Build a shared expat library")
expat_shy_set(EXPAT_BUILD_DOCS ${_EXPAT_BUILD_DOCS_DEFAULT} CACHE BOOL "Build man page for xmlwf")
expat_shy_set(EXPAT_BUILD_FUZZERS OFF CACHE BOOL "Build fuzzers for the expat library")
expat_shy_set(EXPAT_BUILD_PKGCONFIG ${_EXPAT_BUILD_PKGCONFIG_DEFAULT} CACHE BOOL "Build pkg-config file")
expat_shy_set(EXPAT_OSSFUZZ_BUILD OFF CACHE BOOL "Build fuzzers via OSS-Fuzz for the expat library")
if(UNIX OR _EXPAT_HELP)
    expat_shy_set(EXPAT_WITH_LIBBSD OFF CACHE BOOL "Utilize libbsd (for arc4random_buf)")
endif()
expat_shy_set(EXPAT_ENABLE_INSTALL ON CACHE BOOL "Install expat files in cmake install target")
expat_shy_set(EXPAT_CONTEXT_BYTES 1024 CACHE STRING "Define to specify how much context to retain around the current parse point, 0 to disable")
mark_as_advanced(EXPAT_CONTEXT_BYTES)
expat_shy_set(EXPAT_DTD ON CACHE BOOL "Define to make parameter entity parsing functionality available")
mark_as_advanced(EXPAT_DTD)
expat_shy_set(EXPAT_GE ON CACHE BOOL "Define to make general entity parsing functionality available")
mark_as_advanced(EXPAT_GE)
expat_shy_set(EXPAT_NS ON CACHE BOOL "Define to make XML Namespaces functionality available")
mark_as_advanced(EXPAT_NS)
expat_shy_set(EXPAT_WARNINGS_AS_ERRORS OFF CACHE BOOL "Treat all compiler warnings as errors")
if(UNIX OR _EXPAT_HELP)
    expat_shy_set(EXPAT_DEV_URANDOM ON CACHE BOOL "Define to include code reading entropy from `/dev/urandom'.")
    expat_shy_set(EXPAT_WITH_GETRANDOM "AUTO" CACHE STRING "Make use of getrandom function (ON|OFF|AUTO) [default=AUTO]")
    expat_shy_set(EXPAT_WITH_SYS_GETRANDOM "AUTO" CACHE STRING "Make use of syscall SYS_getrandom (ON|OFF|AUTO) [default=AUTO]")
    mark_as_advanced(EXPAT_DEV_URANDOM)
endif()
expat_shy_set(EXPAT_CHAR_TYPE "char" CACHE STRING "Character type to use (char|ushort|wchar_t) [default=char]")
expat_shy_set(EXPAT_ATTR_INFO OFF CACHE BOOL "Define to allow retrieving the byte offsets for attribute names and values")
mark_as_advanced(EXPAT_ATTR_INFO)
expat_shy_set(EXPAT_LARGE_SIZE OFF CACHE BOOL "Make XML_GetCurrent* functions return <(unsigned) long long> rather than <(unsigned) long>")
mark_as_advanced(EXPAT_LARGE_SIZE)
expat_shy_set(EXPAT_MIN_SIZE OFF CACHE BOOL "Get a smaller (but slower) parser (in particular avoid multiple copies of the tokenizer)")
mark_as_advanced(EXPAT_MIN_SIZE)
if(MSVC OR _EXPAT_HELP)
    expat_shy_set(EXPAT_MSVC_STATIC_CRT OFF CACHE BOOL "Use /MT flag (static CRT) when compiling in MSVC")
endif()
if(NOT _EXPAT_HELP)
    expat_shy_set(_EXPAT_M32 OFF CACHE BOOL "(Unofficial!) Produce 32bit code with -m32")
    mark_as_advanced(_EXPAT_M32)
endif()

if(EXPAT_BUILD_TESTS OR EXPAT_BUILD_FUZZERS)
    # We have to call enable_language() before modifying any CMAKE_CXX_* variables
    enable_language(CXX)

    if (EXPAT_BUILD_FUZZERS)
        set(CMAKE_CXX_STANDARD 17)  # for std::string_view for Abseil for libprotobuf-mutator
    else()
        set(CMAKE_CXX_STANDARD 11)
    endif()
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)  # i.e. -std=c++11 rather than default -std=gnu++11
endif()

#
# Environment checks
#
if(EXPAT_DTD AND NOT EXPAT_GE)
    message(SEND_ERROR "Option EXPAT_DTD requires that EXPAT_GE is also enabled.")
    message(SEND_ERROR "Please either enable option EXPAT_GE (recommended) or disable EXPAT_DTD also.")
endif()

if(EXPAT_WITH_LIBBSD)
    find_library(LIB_BSD NAMES bsd)
    if(NOT LIB_BSD)
        message(SEND_ERROR "EXPAT_WITH_LIBBSD option is enabled, but libbsd was not found")
    else()
        set(HAVE_LIBBSD TRUE)
    endif()
endif()

if(MSVC)
    # For the three types of MSVC version values, please see:
    # - https://cmake.org/cmake/help/latest/variable/MSVC_VERSION.html
    # - https://sourceforge.net/p/predef/wiki/Compilers/
    # - https://en.wikipedia.org/wiki/Microsoft_Visual_Studio#History
    set(_EXPAT_MSVC_REQUIRED_INT 1800)  # i.e. 12.0/2013/1800; see PR #426
    set(_EXPAT_MSVC_SUPPORTED_INT 1920)
    set(_EXPAT_MSVC_SUPPORTED_DISPLAY "Visual Studio 16.0/2019/${_EXPAT_MSVC_SUPPORTED_INT}")

    if(MSVC_VERSION VERSION_LESS ${_EXPAT_MSVC_SUPPORTED_INT})
        if(MSVC_VERSION VERSION_LESS ${_EXPAT_MSVC_REQUIRED_INT})
            message(SEND_ERROR "MSVC_VERSION ${MSVC_VERSION} is TOO OLD to compile Expat without errors.")
            message(SEND_ERROR "Please use officially supported ${_EXPAT_MSVC_SUPPORTED_DISPLAY} or later.  Thank you!")
        else()
            message(WARNING "MSVC_VERSION ${MSVC_VERSION} is NOT OFFICIALLY SUPPORTED by Expat.")
            message(WARNING "Please use ${_EXPAT_MSVC_SUPPORTED_DISPLAY} or later.  Thank you!")
        endif()
    endif()
endif()

macro(_expat_copy_bool_int source_ref dest_ref)
    if(${source_ref})
        set(${dest_ref} 1)
    else()
        set(${dest_ref} 0)
    endif()
endmacro()

if(EXPAT_LARGE_SIZE)
    add_definitions(-DXML_LARGE_SIZE)
endif()

if(EXPAT_MIN_SIZE)
    add_definitions(-DXML_MIN_SIZE)
endif()

if(EXPAT_CHAR_TYPE STREQUAL "char")
    set(_EXPAT_UNICODE OFF)
    set(_EXPAT_UNICODE_WCHAR_T OFF)
elseif(EXPAT_CHAR_TYPE STREQUAL "ushort")
    set(_EXPAT_UNICODE ON)
    set(_EXPAT_UNICODE_WCHAR_T OFF)
    if(EXPAT_BUILD_EXAMPLES)
        message(SEND_ERROR "Examples can not be built with option -DEXPAT_CHAR_TYPE=ushort. Please pass -DEXPAT_CHAR_TYPE=(char|wchar_t) or -DEXPAT_BUILD_EXAMPLES=OFF.")
    endif()
    if(EXPAT_BUILD_TESTS)
        message(SEND_ERROR "The testsuite can not be built with option -DEXPAT_CHAR_TYPE=ushort. Please pass -DEXPAT_CHAR_TYPE=(char|wchar_t) or -DEXPAT_BUILD_TESTS=OFF.")
    endif()
    if(EXPAT_BUILD_TOOLS)
        message(SEND_ERROR "The xmlwf tool can not be built with option -DEXPAT_CHAR_TYPE=ushort. Please pass -DEXPAT_CHAR_TYPE=(char|wchar_t) or -DEXPAT_BUILD_TOOLS=OFF.")
    endif()
elseif(EXPAT_CHAR_TYPE STREQUAL "wchar_t")
    set(_EXPAT_UNICODE ON)
    set(_EXPAT_UNICODE_WCHAR_T ON)
    if(NOT WIN32)
        string(FIND "${CMAKE_C_FLAGS}" "-fshort-wchar" _expat_short_wchar_found)
        if(${_expat_short_wchar_found} EQUAL "-1")
            message(SEND_ERROR "Configuration -DEXPAT_CHAR_TYPE=wchar_t requires -DCMAKE_{C,CXX}_FLAGS=-fshort-wchar (which was not found) and libc compiled with -fshort-wchar, too.")
        endif()
        if (EXPAT_BUILD_TOOLS)
            message(SEND_ERROR "The xmlwf tool can not be built with option -DEXPAT_CHAR_TYPE=wchar_t outside of Windows. Please pass -DEXPAT_CHAR_TYPE=char or -DEXPAT_BUILD_TOOLS=OFF.")
        endif()
    endif()
else()
    message(SEND_ERROR "Option -DEXPAT_CHAR_TYPE=(char|ushort|wchar_t) cannot be \"${EXPAT_CHAR_TYPE}\".")
endif()

if(_EXPAT_UNICODE)
    add_definitions(-DXML_UNICODE)              # for unsigned short
    if(_EXPAT_UNICODE_WCHAR_T)
        add_definitions(-DXML_UNICODE_WCHAR_T)  # for wchar_t
    endif()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/ConfigureChecks.cmake)

macro(evaluate_detection_results use_ref have_ref thing_lower thing_title)
    if(${use_ref} AND NOT (${use_ref} STREQUAL "AUTO") AND NOT ${have_ref})
        message(SEND_ERROR
                "Use of ${thing_lower} was enforced by ${use_ref}=ON but it could not be found.")
    elseif(NOT ${use_ref} AND ${have_ref})
        message("${thing_title} was found but it will not be used due to ${use_ref}=OFF.")
        set(${have_ref} 0)
    endif()
endmacro()

if(NOT WIN32)
    evaluate_detection_results(EXPAT_WITH_GETRANDOM HAVE_GETRANDOM "function getrandom" "Function getrandom")
    evaluate_detection_results(EXPAT_WITH_SYS_GETRANDOM HAVE_SYSCALL_GETRANDOM "syscall SYS_getrandom" "Syscall SYS_getrandom")
endif()

_expat_copy_bool_int(EXPAT_ATTR_INFO        XML_ATTR_INFO)
_expat_copy_bool_int(EXPAT_DTD              XML_DTD)
_expat_copy_bool_int(EXPAT_GE               XML_GE)
_expat_copy_bool_int(EXPAT_LARGE_SIZE       XML_LARGE_SIZE)
_expat_copy_bool_int(EXPAT_MIN_SIZE         XML_MIN_SIZE)
_expat_copy_bool_int(EXPAT_NS               XML_NS)
if(NOT WIN32)
    _expat_copy_bool_int(EXPAT_DEV_URANDOM  XML_DEV_URANDOM)
endif()
if(NOT EXPAT_CONTEXT_BYTES GREATER 0)  # in particular with -DEXPAT_CONTEXT_BYTES=OFF
    set(EXPAT_CONTEXT_BYTES 0)
endif()
set(XML_CONTEXT_BYTES ${EXPAT_CONTEXT_BYTES})

macro(expat_install)
    if(EXPAT_ENABLE_INSTALL)
        install(${ARGN})
    endif()
endmacro()

configure_file(expat_config.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/expat_config.h")
expat_install(FILES "${CMAKE_CURRENT_BINARY_DIR}/expat_config.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


set(EXTRA_COMPILE_FLAGS)
if(FLAG_NO_STRICT_ALIASING)
    set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -fno-strict-aliasing")
endif()
if(FLAG_VISIBILITY)
  if(EXPAT_SHARED_LIBS)
     add_definitions(-DXML_ENABLE_VISIBILITY=1)
  endif()
  set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -fvisibility=hidden")
endif()
if(MINGW AND ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU"))
    # Without __USE_MINGW_ANSI_STDIO the compiler produces a false positive
    set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -Wno-pedantic-ms-format")
endif()
if (EXPAT_WARNINGS_AS_ERRORS)
    if(MSVC)
        add_definitions(/WX)
    else()
        set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -Werror")
        if(MINGW)
            # To avoid "error: unknown conversion type character ‘l’ in format [-Werror=format=]"
            set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -Wno-format")
        endif()
    endif()
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}")

if (MSVC)
    if (EXPAT_MSVC_STATIC_CRT)
        message("-- Using static CRT ${EXPAT_MSVC_STATIC_CRT}")
        foreach(flag_var
                CMAKE_CXX_FLAGS_${_EXPAT_BUILD_TYPE_UPPER}
                CMAKE_CXX_FLAGS_DEBUG
                CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL
                CMAKE_CXX_FLAGS_RELWITHDEBINFO
                CMAKE_C_FLAGS_${_EXPAT_BUILD_TYPE_UPPER}
                CMAKE_C_FLAGS_DEBUG
                CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL
                CMAKE_C_FLAGS_RELWITHDEBINFO
                )
            string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endforeach()
    endif()
endif()

if(_EXPAT_M32 AND NOT MSVC)
    foreach(flag_var
            CMAKE_CXX_FLAGS_${_EXPAT_BUILD_TYPE_UPPER}
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_${_EXPAT_BUILD_TYPE_UPPER}
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
            )
        set(${flag_var} "${${flag_var}} -m32")
    endforeach()
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -wd4996)
endif()

#
# C library
#
set(_EXPAT_C_SOURCES
    lib/xmlparse.c
    lib/xmlrole.c
    lib/xmltok.c
# NOTE: ISO C forbids an empty translation unit
#   lib/xmltok_impl.c
#   lib/xmltok_ns.c
)

if(EXPAT_SHARED_LIBS)
    set(_SHARED SHARED)
    if(WIN32)
        macro(_expat_def_file_toggle source_var target_var)
            if(${source_var})
                set(${target_var} " ")  # i.e. not commented out, a single space
            else()
                set(${target_var} ";")  # i.e. commented out
            endif()
        endmacro()

        if(EXPAT_DTD OR EXPAT_GE)
            set(_EXPAT_DTD_OR_GE TRUE)
        else()
            set(_EXPAT_DTD_OR_GE FALSE)
        endif()

        _expat_def_file_toggle(_EXPAT_DTD_OR_GE _EXPAT_COMMENT_DTD_OR_GE)
        _expat_def_file_toggle(EXPAT_ATTR_INFO _EXPAT_COMMENT_ATTR_INFO)

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/lib/libexpat.def.cmake" "${CMAKE_CURRENT_BINARY_DIR}/lib/libexpat.def")
        set(_EXPAT_EXTRA_SOURCES ${_EXPAT_EXTRA_SOURCES} "${CMAKE_CURRENT_BINARY_DIR}/lib/libexpat.def")

        # Add DLL version
        string(REPLACE "." "," _EXPAT_DLL_VERSION ${PROJECT_VERSION}.0)
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/win32/version.rc.cmake" "${CMAKE_CURRENT_BINARY_DIR}/win32/version.rc")
        set(_EXPAT_EXTRA_SOURCES ${_EXPAT_EXTRA_SOURCES} "${CMAKE_CURRENT_BINARY_DIR}/win32/version.rc")
    endif()
else()
    set(_SHARED STATIC)
endif()

add_library(expat ${_SHARED} ${_EXPAT_C_SOURCES} ${_EXPAT_EXTRA_SOURCES})
add_library(expat::expat ALIAS expat)
if(_EXPAT_LIBM_FOUND)
    target_link_libraries(expat m)
endif()
if(EXPAT_WITH_LIBBSD)
    target_link_libraries(expat ${LIB_BSD})
endif()

#
# Library filename postfix
#
if(_EXPAT_UNICODE)
    set(_POSTFIX_WIDE "w")
endif()

if(MSVC AND NOT EXPAT_SHARED_LIBS)
    if(EXPAT_MSVC_STATIC_CRT)
        set(_POSTFIX_CRT "MT")
    else()
        set(_POSTFIX_CRT "MD")
    endif()
endif()

foreach(build_type_upper
        ${_EXPAT_BUILD_TYPE_UPPER}
        DEBUG
        RELEASE
        MINSIZEREL
        RELWITHDEBINFO
        )
    if(WIN32 AND build_type_upper STREQUAL "DEBUG")
        set(_POSTFIX_DEBUG "d")
    else()
        set(_POSTFIX_DEBUG "")  # needs a reset because of being looped
    endif()

    expat_shy_set(EXPAT_${build_type_upper}_POSTFIX "${_POSTFIX_WIDE}${_POSTFIX_DEBUG}${_POSTFIX_CRT}" CACHE STRING "Library filename postfix for build type ${build_type_upper}; yields filenames libexpat<postfix=[w][d][MD|MT]>.(dll|dylib|lib|so)")
    mark_as_advanced(EXPAT_${build_type_upper}_POSTFIX)
    set_property(TARGET expat PROPERTY ${build_type_upper}_POSTFIX ${EXPAT_${build_type_upper}_POSTFIX})
endforeach()

set(LIBCURRENT 11)  # sync
set(LIBREVISION 2)  # with
set(LIBAGE 10)      # configure.ac!
math(EXPR LIBCURRENT_MINUS_AGE "${LIBCURRENT} - ${LIBAGE}")

if(NOT WIN32)
    set_property(TARGET expat PROPERTY VERSION ${LIBCURRENT_MINUS_AGE}.${LIBAGE}.${LIBREVISION})
    set_property(TARGET expat PROPERTY SOVERSION ${LIBCURRENT_MINUS_AGE})
    set_property(TARGET expat PROPERTY NO_SONAME ${NO_SONAME})

    if(APPLE)
        if(NOT CMAKE_VERSION VERSION_GREATER_EQUAL 3.17)
            message(FATAL_ERROR "Expat requires CMake >=3.17 on platform \"APPLE\".")
        endif()

        # NOTE: This intends to talk CMake into compatibility with GNU Libtool
        math(EXPR _EXPAT_MACHO_COMPATIBILITY_VERSION "${LIBCURRENT} + 1")
        set(_EXPAT_MACHO_CURRENT_VERSION "${_EXPAT_MACHO_COMPATIBILITY_VERSION}.${LIBREVISION}")
        set_property(TARGET expat PROPERTY MACHO_COMPATIBILITY_VERSION ${_EXPAT_MACHO_COMPATIBILITY_VERSION})
        set_property(TARGET expat PROPERTY MACHO_CURRENT_VERSION ${_EXPAT_MACHO_CURRENT_VERSION})
    endif()
endif()

if(MINGW AND EXPAT_SHARED_LIBS)
    set_target_properties(expat PROPERTIES SUFFIX "-${LIBCURRENT_MINUS_AGE}.dll")
endif()

if(WIN32 AND NOT MINGW)
    # NOTE: This avoids a name collision with Expat.dll of Perl's XML::Parser::Expat
    #       on Windows by resorting to filename libexpat.dll since Expat 1.95.3.
    #       Everything but MSVC is already adding prefix "lib", automatically.
    # NOTE: "set_property(TARGET expat PROPERTY PREFIX lib)" would only affect *.dll
    #       files but not *.lib files, so we have to rely on property OUTPUT_NAME, instead.
    #       Target property <CONFIG>_POSTFIX still applies.
    set(_EXPAT_OUTPUT_NAME libexpat)
    set_property(TARGET expat PROPERTY OUTPUT_NAME ${_EXPAT_OUTPUT_NAME})
else()
    set(_EXPAT_OUTPUT_NAME expat)
endif()

target_include_directories(expat
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(WIN32 AND EXPAT_SHARED_LIBS)
    target_compile_definitions(expat PRIVATE VER_FILEVERSION=${_EXPAT_DLL_VERSION})
endif()

expat_install(TARGETS expat EXPORT expat
                      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

expat_install(FILES lib/expat.h lib/expat_external.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#
# pkg-config file
#
if(EXPAT_BUILD_PKGCONFIG)
    if(CMAKE_INSTALL_LIBDIR MATCHES "^/")
        set(_expat_pkgconfig_libdir "${CMAKE_INSTALL_LIBDIR}")
    else()
        set(_expat_pkgconfig_libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
    endif()

    if(CMAKE_INSTALL_INCLUDEDIR MATCHES "^/")
        set(_expat_pkgconfig_includedir "${CMAKE_INSTALL_INCLUDEDIR}")
    else()
        set(_expat_pkgconfig_includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
    endif()

    set_target_properties(expat PROPERTIES
        pkgconfig_prefix "${CMAKE_INSTALL_PREFIX}"
        pkgconfig_exec_prefix "\${prefix}"
        pkgconfig_libdir "${_expat_pkgconfig_libdir}"
        pkgconfig_includedir "${_expat_pkgconfig_includedir}"
        pkgconfig_version "${PACKAGE_VERSION}")

    foreach(_build_type ${CMAKE_BUILD_TYPE} Debug Release RelWithDebInfo MinSizeRel)
        string(TOLOWER "${_build_type}" _build_type_lower)
        string(TOUPPER "${_build_type}" _build_type_upper)
        set_property(TARGET expat PROPERTY "pkgconfig_${_build_type_lower}_name" "expat${EXPAT_${_build_type_upper}_POSTFIX}")
        set_property(TARGET expat PROPERTY "pkgconfig_${_build_type_lower}_output_name" "${_EXPAT_OUTPUT_NAME}${EXPAT_${_build_type_upper}_POSTFIX}")
        if(_EXPAT_LIBM_FOUND)
            set_property(TARGET expat PROPERTY "pkgconfig_libm" "-lm")
        else()
            set_property(TARGET expat PROPERTY "pkgconfig_libm" "")
        endif()
    endforeach()

    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/expat.pc
        INPUT ${PROJECT_SOURCE_DIR}/expat.pc.cmake)

    expat_install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/expat.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

#
# C command line tool xmlwf
#
if(EXPAT_BUILD_TOOLS)
    set(xmlwf_SRCS
        xmlwf/codepage.c
        xmlwf/readfilemap.c
        xmlwf/xmlfile.c
        xmlwf/xmlwf.c
    )

    add_executable(xmlwf ${xmlwf_SRCS})
    set_property(TARGET xmlwf PROPERTY RUNTIME_OUTPUT_DIRECTORY xmlwf)
    target_link_libraries(xmlwf expat)
    if(_EXPAT_LIBM_FOUND)
        target_link_libraries(xmlwf m)
    endif()
    expat_install(TARGETS xmlwf DESTINATION ${CMAKE_INSTALL_BINDIR})

    if(MINGW AND _EXPAT_UNICODE_WCHAR_T)
        # https://gcc.gnu.org/onlinedocs/gcc/x86-Windows-Options.html
        target_link_options(xmlwf PRIVATE -municode)
    endif()

    if(EXPAT_BUILD_DOCS)
        file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/doc)
        add_custom_target(
            xmlwf-manpage
            COMMAND
                "${DOCBOOK_TO_MAN}" "${PROJECT_SOURCE_DIR}/doc/xmlwf.xml" && mv "XMLWF.1" "${PROJECT_BINARY_DIR}/doc/xmlwf.1"
            BYPRODUCTS
                doc/xmlwf.1)
        add_dependencies(expat xmlwf-manpage)
        expat_install(FILES "${PROJECT_BINARY_DIR}/doc/xmlwf.1" DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
    elseif(EXISTS ${PROJECT_SOURCE_DIR}/doc/xmlwf.1)
        expat_install(FILES "${PROJECT_SOURCE_DIR}/doc/xmlwf.1" DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
    endif()
endif()

#
# C code examples
#
if(EXPAT_BUILD_EXAMPLES)
    foreach(_target element_declarations elements outline)
        add_executable(${_target} examples/${_target}.c)
        set_property(TARGET ${_target} PROPERTY RUNTIME_OUTPUT_DIRECTORY examples)
        target_link_libraries(${_target} expat)
    endforeach()
endif()

#
# C/C++ test runners
#
if(EXPAT_BUILD_TESTS)
    ## these are unittests that can be run on any platform
    enable_testing()

    if(NOT MSVC)
        if(MINGW)
            set(host whatever-mingw32)  # for nothing but run.sh
        endif()
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh.in run.sh @ONLY)
    endif()

    function(expat_add_test _name _file)
        if(MSVC)
            add_test(NAME ${_name} COMMAND ${_file})
        else()
            add_test(NAME ${_name} COMMAND bash run.sh ${_file})
        endif()
    endfunction()

    set(_EXPAT_TEST_TARGETS runtests runtests_cxx)

    add_executable(runtests
        tests/acc_tests.c
        tests/alloc_tests.c
        tests/basic_tests.c
        tests/chardata.c
        tests/common.c
        tests/dummy.c
        tests/handlers.c
        tests/memcheck.c
        tests/minicheck.c
        tests/misc_tests.c
        tests/ns_tests.c
        tests/nsalloc_tests.c
        tests/runtests.c
        tests/structdata.c
        ${_EXPAT_C_SOURCES}
    )

    add_executable(runtests_cxx
        tests/acc_tests_cxx.cpp
        tests/alloc_tests_cxx.cpp
        tests/basic_tests_cxx.cpp
        tests/chardata_cxx.cpp
        tests/common_cxx.cpp
        tests/dummy_cxx.cpp
        tests/handlers_cxx.cpp
        tests/memcheck_cxx.cpp
        tests/minicheck_cxx.cpp
        tests/misc_tests_cxx.cpp
        tests/ns_tests_cxx.cpp
        tests/nsalloc_tests_cxx.cpp
        tests/runtests_cxx.cpp
        tests/structdata_cxx.cpp
        ${_EXPAT_C_SOURCES}
    )

    foreach(_target ${_EXPAT_TEST_TARGETS})
        target_compile_definitions(${_target} PRIVATE -DXML_TESTING)

        set_property(TARGET ${_target} PROPERTY RUNTIME_OUTPUT_DIRECTORY tests)
        expat_add_test(${_target} $<TARGET_FILE:${_target}>)

        if(_EXPAT_LIBM_FOUND)
            target_link_libraries(${_target} m)
        endif()

        if(EXPAT_WITH_LIBBSD)
            target_link_libraries(${_target} ${LIB_BSD})
        endif()
    endforeach()

    add_executable(benchmark tests/benchmark/benchmark.c)
    set_property(TARGET benchmark PROPERTY RUNTIME_OUTPUT_DIRECTORY tests/benchmark)
    target_link_libraries(benchmark expat)
endif()

#
# Fuzzers
#
if(EXPAT_BUILD_FUZZERS)
    if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        message(SEND_ERROR
            "Building fuzz targets without Clang (but ${CMAKE_C_COMPILER_ID}) "
            "is not supported. Please set "
            "-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++.")
    endif()

    string(FIND "${CMAKE_C_FLAGS}" "-fsanitize" sanitizer_present)
    if(${sanitizer_present} EQUAL "-1")
        message(WARNING
            "There was no sanitizer present when building the fuzz targets. "
            "This is likely in error - consider adding "
            "-DCMAKE_C_FLAGS='-fsanitize=<sanitizer>' and "
            "-DCMAKE_CXX_FLAGS='-fsanitize=<sanitizer>' and "
            "-DCMAKE_EXE_LINKER_FLAGS='-fsanitize=<sanitizer>' and "
            "-DCMAKE_MODULE_LINKER_FLAGS='-fsanitize=<sanitizer>' and "
            "-DCMAKE_SHARED_LINKER_FLAGS='-fsanitize=<sanitizer>' to your cmake "
            "execution.")
    endif()

    if(EXPAT_OSSFUZZ_BUILD AND NOT DEFINED ENV{LIB_FUZZING_ENGINE})
        message(SEND_ERROR
            "OSS-Fuzz builds require the environment variable "
            "LIB_FUZZING_ENGINE to be set. If you are seeing this "
            "warning, it points to a deeper problem in the OSS-Fuzz "
            "build setup.")
    endif()

    set(encoding_types UTF-16 UTF-8 ISO-8859-1 US-ASCII UTF-16BE UTF-16LE)
    set(fuzz_targets xml_parse_fuzzer xml_parsebuffer_fuzzer)

    add_library(fuzzpat STATIC ${_EXPAT_C_SOURCES})
    if(NOT EXPAT_OSSFUZZ_BUILD)
        target_compile_options(fuzzpat PRIVATE -fsanitize=fuzzer-no-link)
    endif()

    foreach(fuzz_target ${fuzz_targets})
        foreach(encoding_type ${encoding_types})
            set(target_name ${fuzz_target}_${encoding_type})
            add_executable(${target_name} fuzz/${fuzz_target}.c)
            target_link_libraries(${target_name} fuzzpat)
            target_compile_definitions(${target_name}
                PRIVATE ENCODING_FOR_FUZZING=${encoding_type})
            if(EXPAT_OSSFUZZ_BUILD)
                target_link_options(${target_name} PRIVATE $ENV{LIB_FUZZING_ENGINE})
                set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE "CXX")
            else()
                target_compile_options(${target_name} PRIVATE -fsanitize=fuzzer)
                target_link_options(${target_name} PRIVATE -fsanitize=fuzzer)
            endif()
            set_property(
                TARGET ${target_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY fuzz)
        endforeach()
    endforeach()

    find_package(Protobuf REQUIRED)

    # Only include libprotobuf-mutator here so we don't build it in non-fuzz
    # configurations.
    include(ExternalProject)

    set(ProtobufMutator_PREFIX libprotobuf-mutator)
    set(ProtobufMutator_PATH ${CMAKE_CURRENT_BINARY_DIR}/${ProtobufMutator_PREFIX}/src/${ProtobufMutator_PREFIX})
    set(ProtobufMutator_BUILD_PATH ${ProtobufMutator_PATH}-build)
    set(ProtobufMutator_INCLUDE_DIR ${ProtobufMutator_PATH})
    set(ProtobufMutator_LIBRARIES ${ProtobufMutator_BUILD_PATH}/src/libfuzzer/libprotobuf-mutator-libfuzzer.a ${ProtobufMutator_BUILD_PATH}/src/libprotobuf-mutator.a)

    ExternalProject_Add(
        ${ProtobufMutator_PREFIX}
        PREFIX ${ProtobufMutator_PREFIX}
        GIT_REPOSITORY https://github.com/google/libprotobuf-mutator.git
        GIT_TAG 57928f41ae52bb27666aa15b310130d086dac245  # v1.4-16-g57928f4
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        CMAKE_CACHE_ARGS
            -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
            -DLIB_PROTO_MUTATOR_EXAMPLES:BOOL=OFF
            -DLIB_PROTO_MUTATOR_TESTING:BOOL=OFF
        BUILD_BYPRODUCTS ${ProtobufMutator_LIBRARIES}
        UPDATE_COMMAND true
        INSTALL_COMMAND true)

    # Check for availability of protobuf compiler to avoid hard-to-understand
    # errors from make(1) down the line as seen with CMake 3.25.1 on Debian
    if(NOT Protobuf_PROTOC_EXECUTABLE)
        message(SEND_ERROR
            "The protobuf compiler (protoc) could not be found. "
            "Is it installed and working properly?")
    endif()

    protobuf_generate_cpp(XML_LPM_FUZZER_PROTO_SRCS
                          XML_LPM_FUZZER_PROTO_HDRS
                          fuzz/xml_lpm_fuzzer.proto)

    add_executable(xml_lpm_fuzzer
                   fuzz/xml_lpm_fuzzer.cpp
                   ${XML_LPM_FUZZER_PROTO_SRCS})
    target_include_directories(xml_lpm_fuzzer PUBLIC ${ProtobufMutator_INCLUDE_DIR})
    target_link_libraries(xml_lpm_fuzzer
                          fuzzpat
                          ${ProtobufMutator_LIBRARIES}
                          ${Protobuf_LIBRARIES})
    add_dependencies(xml_lpm_fuzzer ${ProtobufMutator_PREFIX})

    if(EXPAT_OSSFUZZ_BUILD)
        target_link_options(xml_lpm_fuzzer PRIVATE $ENV{LIB_FUZZING_ENGINE})
    else()
        target_compile_options(xml_lpm_fuzzer PRIVATE -fsanitize=fuzzer)
        target_link_options(xml_lpm_fuzzer PRIVATE -fsanitize=fuzzer)
    endif()
    set_property(TARGET xml_lpm_fuzzer PROPERTY RUNTIME_OUTPUT_DIRECTORY fuzz)
else()
    if(EXPAT_OSSFUZZ_BUILD)
        message(SEND_ERROR
                "Attempting to perform an OSS-Fuzz build without turning on the fuzzer build. "
                "This is likely in error - consider adding "
                "-DEXPAT_BUILD_FUZZERS=ON to your cmake execution.")
    endif()
endif()

#
# C/C++ config affecting multiple targets
#
if(WIN32)
    set(_EXPAT_STATIC_TARGETS ${_EXPAT_TEST_TARGETS})
    if(NOT EXPAT_SHARED_LIBS)
        list(APPEND _EXPAT_STATIC_TARGETS expat)
    endif()
    if(EXPAT_BUILD_FUZZERS)
        list(APPEND _EXPAT_STATIC_TARGETS fuzzpat)
    endif()

    foreach(_target ${_EXPAT_STATIC_TARGETS})
        target_compile_definitions(${_target} PUBLIC -DXML_STATIC)
    endforeach()
endif()

#
# Custom target "run-xmltest"
#
if(EXPAT_BUILD_TOOLS AND NOT MSVC)
    add_custom_target(
        xmlts-zip-downloaded
        COMMAND
            sh -c 'test -f xmlts.zip || wget --output-document=xmlts.zip https://www.w3.org/XML/Test/xmlts20080827.zip'
        BYPRODUCTS
            tests/xmlts.zip
        WORKING_DIRECTORY
            tests/)

    add_custom_target(
        xmlts-zip-extracted
        COMMAND
            sh -c 'test -d xmlconf || unzip -q xmlts.zip'
        BYPRODUCTS
            tests/xmlconf
        WORKING_DIRECTORY
            tests/)
    add_dependencies(xmlts-zip-extracted xmlts-zip-downloaded)

    add_custom_target(
        xmltest-sh-been-run
        COMMAND
            sh -c '${CMAKE_CURRENT_SOURCE_DIR}/tests/xmltest.sh "bash ${CMAKE_CURRENT_BINARY_DIR}/run.sh $<TARGET_FILE:xmlwf>" 2>&1 | tee tests/xmltest.log'
        BYPRODUCTS
            tests/xmltest.log)
    add_dependencies(xmltest-sh-been-run xmlts-zip-extracted xmlwf)

    add_custom_target(
        xmltest-log-fixed
        COMMAND
            ${CMAKE_CURRENT_SOURCE_DIR}/fix-xmltest-log.sh tests/xmltest.log
        DEPENDS
            tests/xmltest.log)
    add_dependencies(xmltest-log-fixed xmltest-sh-been-run)

    add_custom_target(
        xmltest-log-verified
        COMMAND
            diff -u ${CMAKE_CURRENT_SOURCE_DIR}/tests/xmltest.log.expected tests/xmltest.log)
    add_dependencies(xmltest-log-verified xmltest-log-fixed)

    add_custom_target(run-xmltest)
    add_dependencies(run-xmltest xmltest-log-verified)
endif()

#
# Documentation
#
configure_file(Changes changelog COPYONLY)
expat_install(
    FILES
        AUTHORS
        ${CMAKE_CURRENT_BINARY_DIR}/changelog
    DESTINATION
        ${CMAKE_INSTALL_DOCDIR})

#
# CMake files for find_package(expat [..] CONFIG [..])
#
configure_package_config_file(
        cmake/expat-config.cmake.in
        cmake/expat-config.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/expat-${PROJECT_VERSION}/
)
write_basic_package_version_file(
    cmake/expat-config-version.cmake
    COMPATIBILITY SameMajorVersion  # i.e. semver
)
export(
    TARGETS
        expat
    FILE
        cmake/expat-targets.cmake  # not going to be installed
)
expat_install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/expat-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/expat-config-version.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/expat-${PROJECT_VERSION}/
)
expat_install(
    EXPORT
        expat
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/expat-${PROJECT_VERSION}/
    NAMESPACE
        expat::
)

#
# CPack
#

# This effectively disables target "package_source".
# That is done due to CPack's unfortunate choice of an exclusion list
# rather than inclusion list.  An exclusion list does not protect against
# unwanted files ending up in the resulting archive in a way that's
# safe to run from an Expat developer's machine.
set(CPACK_SOURCE_GENERATOR '')

if(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()

include(CPack)

#
# Summary
#
if(EXPAT_CHAR_TYPE STREQUAL "char")
    set(_EXPAT_CHAR_TYPE_SUMMARY "char (UTF-8)")
elseif(EXPAT_CHAR_TYPE STREQUAL "ushort")
    set(_EXPAT_CHAR_TYPE_SUMMARY "ushort (unsigned short, UTF-16)")
elseif(EXPAT_CHAR_TYPE STREQUAL "wchar_t")
    if(WIN32)
        set(_EXPAT_CHAR_TYPE_SUMMARY "wchar_t (UTF-16)")
    else()
        set(_EXPAT_CHAR_TYPE_SUMMARY "wchar_t (UTF-32)  // not implemented")
    endif()
else()
    set(_EXPAT_CHAR_TYPE_SUMMARY "ERROR")
endif()
get_property(_EXPAT_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

message(STATUS "===========================================================================")
message(STATUS "")
message(STATUS "Configuration")
message(STATUS "  Generator .................. ${CMAKE_GENERATOR}")
if(_EXPAT_GENERATOR_IS_MULTI_CONFIG)
    message(STATUS "  Build types ................ ${CMAKE_CONFIGURATION_TYPES}")
else()
    message(STATUS "  Build type ................. ${CMAKE_BUILD_TYPE}")
endif()
message(STATUS "  Prefix ..................... ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Shared libraries ........... ${EXPAT_SHARED_LIBS}")
if(MSVC)
    message(STATUS "  Static CRT ................. ${EXPAT_MSVC_STATIC_CRT}")
endif()
message(STATUS "  Character type ............. ${_EXPAT_CHAR_TYPE_SUMMARY}")
if(NOT _EXPAT_GENERATOR_IS_MULTI_CONFIG)
    message(STATUS "  Library name postfix ....... ${EXPAT_${_EXPAT_BUILD_TYPE_UPPER}_POSTFIX}")
endif()
message(STATUS "")
message(STATUS "  Build documentation ........ ${EXPAT_BUILD_DOCS}")
message(STATUS "  Build examples ............. ${EXPAT_BUILD_EXAMPLES}")
message(STATUS "  Build fuzzers .............. ${EXPAT_BUILD_FUZZERS}")
message(STATUS "  Build tests ................ ${EXPAT_BUILD_TESTS}")
message(STATUS "  Build tools (xmlwf) ........ ${EXPAT_BUILD_TOOLS}")
message(STATUS "  Build pkg-config file ...... ${EXPAT_BUILD_PKGCONFIG}")
message(STATUS "  Install files .............. ${EXPAT_ENABLE_INSTALL}")
message(STATUS "")
message(STATUS "  Features")
message(STATUS "    // Advanced options, changes not advised")
message(STATUS "    Attributes info .......... ${EXPAT_ATTR_INFO}")
message(STATUS "    Context bytes ............ ${EXPAT_CONTEXT_BYTES}")
message(STATUS "    DTD support .............. ${EXPAT_DTD}")
message(STATUS "    General entities ......... ${EXPAT_GE}")
message(STATUS "    Large size ............... ${EXPAT_LARGE_SIZE}")
message(STATUS "    Minimum size ............. ${EXPAT_MIN_SIZE}")
message(STATUS "    Namespace support ........ ${EXPAT_NS}")
message(STATUS "")
message(STATUS "  Entropy sources")
if(WIN32)
    message(STATUS "    rand_s ................... ON")
else()
    message(STATUS "    getrandom ................ ${HAVE_GETRANDOM}")
    message(STATUS "    syscall SYS_getrandom .... ${HAVE_SYSCALL_GETRANDOM}")
    message(STATUS "    libbsd ................... ${EXPAT_WITH_LIBBSD}")
    message(STATUS "    /dev/random .............. ${EXPAT_DEV_URANDOM}")
endif()
message(STATUS "")
if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    message(STATUS "Continue with")
    message(STATUS "  make")
    if(EXPAT_BUILD_TESTS)
        message(STATUS "  make test")
    endif()
    if(EXPAT_ENABLE_INSTALL)
        message(STATUS "  sudo make install")
    endif()
    message(STATUS "")
endif()
message(STATUS "===========================================================================")
