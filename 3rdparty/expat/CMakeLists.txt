# This file is copyrighted under the BSD-license for buildsystem files of KDE
# copyright 2010, Patrick Spendrin <ps_ml@gmx.de>

cmake_minimum_required(VERSION 3.1.3)

# This allows controlling documented build time switches
# when Expat is pulled in using the add_subdirectory function, e.g.
#
#   set(EXPAT_BUILD_DOCS OFF)
#   set(EXPAT_BUILD_TOOLS OFF)
#   add_subdirectory(${expat_SOURCE_DIR}/expat ${expat_BINARY_DIR})
#
# would disable compilation of the xmlwf CLI and its man page.
# Without activating behaviour NEW for policy CMP0077 here,
# a user with -Wdev enabled would see warning
#
#   Policy CMP0077 is not set: option() honors normal variables.  Run "cmake
#   --help-policy CMP0077" for policy details.  Use the cmake_policy command to
#   set the policy and suppress this warning.
#
#   For compatibility with older versions of CMake, option is clearing the
#   normal variable 'EXPAT_BUILD_DOCS'.
#
# and effectively not be able to adjust option EXPAT_BUILD_DOCS.
#
# For more details please see:
# - https://cmake.org/cmake/help/latest/policy/CMP0077.html
# - https://github.com/libexpat/libexpat/pull/419
#
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

project(expat
    VERSION
        2.2.10
    LANGUAGES
        C
)

set(PACKAGE_BUGREPORT "expat-bugs@libexpat.org")
set(PACKAGE_NAME "expat")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE_NAME}")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

#
# Detect use by means of add_subdirectory
#
get_directory_property(_EXPAT_PARENT_DIRECTORY PARENT_DIRECTORY)

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

#
# Configuration
#
option(EXPAT_BUILD_TOOLS "build the xmlwf tool for expat library" ${_EXPAT_BUILD_TOOLS_DEFAULT})
option(EXPAT_BUILD_EXAMPLES "build the examples for expat library" ON)
option(EXPAT_BUILD_TESTS "build the tests for expat library" ON)
option(EXPAT_SHARED_LIBS "build a shared expat library" ON)
option(EXPAT_BUILD_DOCS "build man page for xmlwf" ${_EXPAT_BUILD_DOCS_DEFAULT})
option(EXPAT_BUILD_FUZZERS "build fuzzers for the expat library" OFF)
option(EXPAT_BUILD_PKGCONFIG "build pkg-config file" ${_EXPAT_BUILD_PKGCONFIG_DEFAULT})
option(EXPAT_OSSFUZZ_BUILD "build fuzzers via ossfuzz for the expat library" OFF)
if(UNIX OR _EXPAT_HELP)
    option(EXPAT_WITH_LIBBSD "utilize libbsd (for arc4random_buf)" OFF)
endif()
option(EXPAT_ENABLE_INSTALL "install expat files in cmake install target" ON)
set(EXPAT_CONTEXT_BYTES 1024 CACHE STRING "Define to specify how much context to retain around the current parse point")
mark_as_advanced(EXPAT_CONTEXT_BYTES)
option(EXPAT_DTD "Define to make parameter entity parsing functionality available" ON)
mark_as_advanced(EXPAT_DTD)
option(EXPAT_NS "Define to make XML Namespaces functionality available" ON)
mark_as_advanced(EXPAT_NS)
option(EXPAT_WARNINGS_AS_ERRORS "Treat all compiler warnings as errors" OFF)
if(UNIX OR _EXPAT_HELP)
    option(EXPAT_DEV_URANDOM "Define to include code reading entropy from `/dev/urandom'." ON)
    set(EXPAT_WITH_GETRANDOM "AUTO" CACHE STRING
            "Make use of getrandom function (ON|OFF|AUTO) [default=AUTO]")
    set(EXPAT_WITH_SYS_GETRANDOM "AUTO" CACHE STRING
            "Make use of syscall SYS_getrandom (ON|OFF|AUTO) [default=AUTO]")
    mark_as_advanced(EXPAT_DEV_URANDOM)
endif()
set(EXPAT_CHAR_TYPE "char" CACHE STRING "Character type to use (char|ushort|wchar_t) [default=char]")
option(EXPAT_ATTR_INFO "Define to allow retrieving the byte offsets for attribute names and values" OFF)
mark_as_advanced(EXPAT_ATTR_INFO)
option(EXPAT_LARGE_SIZE "Make XML_GetCurrent* functions return <(unsigned) long long> rather than <(unsigned) long>" OFF)
mark_as_advanced(EXPAT_LARGE_SIZE)
option(EXPAT_MIN_SIZE "Get a smaller (but slower) parser (in particular avoid multiple copies of the tokenizer)" OFF)
mark_as_advanced(EXPAT_MIN_SIZE)
if(MSVC OR _EXPAT_HELP)
    set(EXPAT_MSVC_STATIC_CRT OFF CACHE BOOL "Use /MT flag (static CRT) when compiling in MSVC")
endif()

#
# Environment checks
#
if(EXPAT_WITH_LIBBSD)
    find_library(LIB_BSD NAMES bsd)
    if(NOT LIB_BSD)
        message(SEND_ERROR "EXPAT_WITH_LIBBSD option is enabled, but libbsd was not found")
    else()
        set(HAVE_LIBBSD TRUE)
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
_expat_copy_bool_int(EXPAT_LARGE_SIZE       XML_LARGE_SIZE)
_expat_copy_bool_int(EXPAT_MIN_SIZE         XML_MIN_SIZE)
_expat_copy_bool_int(EXPAT_NS               XML_NS)
if(NOT WIN32)
    _expat_copy_bool_int(EXPAT_DEV_URANDOM  XML_DEV_URANDOM)
endif()
set(XML_CONTEXT_BYTES ${EXPAT_CONTEXT_BYTES})

macro(expat_install)
    if(EXPAT_ENABLE_INSTALL)
        install(${ARGN})
    endif()
endmacro()

configure_file(expat_config.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/expat_config.h")
add_definitions(-DHAVE_EXPAT_CONFIG_H)
expat_install(FILES "${CMAKE_CURRENT_BINARY_DIR}/expat_config.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


set(EXTRA_COMPILE_FLAGS)
if(FLAG_NO_STRICT_ALIASING)
    set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -fno-strict-aliasing")
endif()
if(FLAG_VISIBILITY)
  add_definitions(-DXML_ENABLE_VISIBILITY=1)
  set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -fvisibility=hidden")
endif()
if (EXPAT_WARNINGS_AS_ERRORS)
    if(MSVC)
        add_definitions(/WX)
    else()
        set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -Werror")
    endif()
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}")

if (MSVC)
    if (EXPAT_MSVC_STATIC_CRT)
        message("-- Using static CRT ${EXPAT_MSVC_STATIC_CRT}")
        foreach(flag_var
                CMAKE_CXX_FLAGS_DEBUG
                CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL
                CMAKE_CXX_FLAGS_RELWITHDEBINFO
                CMAKE_C_FLAGS_DEBUG
                CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL
                CMAKE_C_FLAGS_RELWITHDEBINFO
                )
            string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endforeach()
    endif()
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -wd4996)
endif()
if(WIN32)
    if(_EXPAT_UNICODE_WCHAR_T)
        set(_POSTFIX_WIDE "w")
    endif()

    if(MSVC AND NOT EXPAT_SHARED_LIBS)
        if(EXPAT_MSVC_STATIC_CRT)
            set(_POSTFIX_CRT "MT")
        else()
            set(_POSTFIX_CRT "MD")
        endif()
    endif()

    foreach(postfix_var
            CMAKE_DEBUG_POSTFIX
            CMAKE_RELEASE_POSTFIX
            CMAKE_MINSIZEREL_POSTFIX
            CMAKE_RELWITHDEBINFO_POSTFIX
            )
        if(postfix_var STREQUAL "CMAKE_DEBUG_POSTFIX")
            set(_POSTFIX_DEBUG "d")
        else()
            set(_POSTFIX_DEBUG "")
        endif()

        set(${postfix_var} "${_POSTFIX_WIDE}${_POSTFIX_DEBUG}${_POSTFIX_CRT}" CACHE STRING "Windows binary postfix, e.g. libexpat<postfix=[w][d][MD|MT]>.lib")
    endforeach()
endif()

#
# C library
#
set(expat_SRCS
    lib/xmlparse.c
    lib/xmlrole.c
    lib/xmltok.c
# NOTE: ISO C forbids an empty translation unit
#   lib/xmltok_impl.c
#   lib/xmltok_ns.c
)

if(EXPAT_SHARED_LIBS)
    set(_SHARED SHARED)
    if(MSVC)
        set(expat_SRCS ${expat_SRCS} lib/libexpat.def)
    endif()
else()
    set(_SHARED STATIC)
endif()

# Avoid colliding with Expat.dll of Perl's XML::Parser::Expat
if(WIN32 AND NOT MINGW)
    set(_EXPAT_OUTPUT_NAME libexpat)  # CMAKE_*_POSTFIX applies, see above
else()
    if(_EXPAT_UNICODE)
        set(_EXPAT_OUTPUT_NAME expatw)
    else()
        set(_EXPAT_OUTPUT_NAME expat)
    endif()
endif()

add_library(expat ${_SHARED} ${expat_SRCS})
if(EXPAT_WITH_LIBBSD)
    target_link_libraries(expat ${LIB_BSD})
endif()

set(LIBCURRENT 7)    # sync
set(LIBREVISION 12)  # with
set(LIBAGE 6)        # configure.ac!
math(EXPR LIBCURRENT_MINUS_AGE "${LIBCURRENT} - ${LIBAGE}")

set_property(TARGET expat PROPERTY OUTPUT_NAME "${_EXPAT_OUTPUT_NAME}")
if(NOT WIN32)
    set_property(TARGET expat PROPERTY VERSION ${LIBCURRENT_MINUS_AGE}.${LIBAGE}.${LIBREVISION})
    set_property(TARGET expat PROPERTY SOVERSION ${LIBCURRENT_MINUS_AGE})
    set_property(TARGET expat PROPERTY NO_SONAME ${NO_SONAME})
endif()

target_include_directories(expat
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(NOT EXPAT_SHARED_LIBS AND WIN32)
    target_compile_definitions(expat PUBLIC -DXML_STATIC)
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
    set(prefix ${CMAKE_INSTALL_PREFIX})
    set(exec_prefix "\${prefix}")
    set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
    set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
    configure_file(expat.pc.in ${CMAKE_CURRENT_BINARY_DIR}/expat.pc @ONLY)
    expat_install(FILES ${CMAKE_CURRENT_BINARY_DIR}/expat.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

#
# C command line tool xmlwf
#
if(EXPAT_BUILD_TOOLS)
    set(xmlwf_SRCS
        xmlwf/xmlwf.c
        xmlwf/xmlfile.c
        xmlwf/codepage.c
        xmlwf/readfilemap.c
    )

    add_executable(xmlwf ${xmlwf_SRCS})
    set_property(TARGET xmlwf PROPERTY RUNTIME_OUTPUT_DIRECTORY xmlwf)
    target_link_libraries(xmlwf expat)
    expat_install(TARGETS xmlwf DESTINATION ${CMAKE_INSTALL_BINDIR})

    if(MINGW AND _EXPAT_UNICODE_WCHAR_T)
        # https://gcc.gnu.org/onlinedocs/gcc/x86-Windows-Options.html
        set_target_properties(xmlwf PROPERTIES LINK_FLAGS -municode)
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
    add_executable(elements examples/elements.c)
    set_property(TARGET elements PROPERTY RUNTIME_OUTPUT_DIRECTORY examples)
    target_link_libraries(elements expat)

    add_executable(outline examples/outline.c)
    set_property(TARGET outline PROPERTY RUNTIME_OUTPUT_DIRECTORY examples)
    target_link_libraries(outline expat)
endif()

#
# C/C++ test runners
#
if(EXPAT_BUILD_TESTS)
    ## these are unittests that can be run on any platform
    enable_language(CXX)
    enable_testing()

    set(test_SRCS
        tests/chardata.c
        tests/memcheck.c
        tests/minicheck.c
        tests/structdata.c
    )

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

    add_executable(runtests tests/runtests.c ${test_SRCS})
    set_property(TARGET runtests PROPERTY RUNTIME_OUTPUT_DIRECTORY tests)
    target_link_libraries(runtests expat)
    expat_add_test(runtests $<TARGET_FILE:runtests>)

    add_executable(runtestspp tests/runtestspp.cpp ${test_SRCS})
    set_property(TARGET runtestspp PROPERTY RUNTIME_OUTPUT_DIRECTORY tests)
    target_link_libraries(runtestspp expat)
    expat_add_test(runtestspp $<TARGET_FILE:runtestspp>)
endif()

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
            "warning, it points to a deeper problem in the ossfuzz "
            "build setup.")
    endif()

    set(encoding_types UTF-16 UTF-8 ISO-8859-1 US-ASCII UTF-16BE UTF-16LE)
    set(fuzz_targets xml_parse_fuzzer xml_parsebuffer_fuzzer)

    add_library(fuzzpat STATIC ${expat_SRCS})
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
            if(NOT EXPAT_OSSFUZZ_BUILD)
                target_compile_options(${target_name} PRIVATE -fsanitize=fuzzer-no-link)
            endif()
            # NOTE: Avoiding target_link_options here only because it needs CMake >=3.13
            if(EXPAT_OSSFUZZ_BUILD)
                set_target_properties(${target_name} PROPERTIES LINK_FLAGS $ENV{LIB_FUZZING_ENGINE})
                set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE "CXX")
            else()
                set_target_properties(${target_name} PROPERTIES LINK_FLAGS -fsanitize=fuzzer)
            endif()
            set_property(
                TARGET ${target_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY fuzz)
        endforeach()
    endforeach()
else()
    if(EXPAT_OSSFUZZ_BUILD)
        message(SEND_ERROR
                "Attempting to perform an ossfuzz build without turning on the fuzzer build. "
                "This is likely in error - consider adding "
                "-DEXPAT_BUILD_FUZZERS=ON to your cmake execution.")
    endif()
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
string(TOUPPER "${CMAKE_BUILD_TYPE}" _EXPAT_BUILD_TYPE_UPPER)

message(STATUS "===========================================================================")
message(STATUS "")
message(STATUS "Configuration")
message(STATUS "  Prefix ..................... ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build type ................. ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared libraries ........... ${EXPAT_SHARED_LIBS}")
if(MSVC)
    message(STATUS "  Static CRT ................. ${EXPAT_MSVC_STATIC_CRT}")
endif()
message(STATUS "  Character type ............. ${_EXPAT_CHAR_TYPE_SUMMARY}")
if(WIN32)
    message(STATUS "  Binary postfix ............. ${CMAKE_${_EXPAT_BUILD_TYPE_UPPER}_POSTFIX}")
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
