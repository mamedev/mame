cmake_minimum_required (VERSION 2.8.12)

include (utils.cmake)

disallow_intree_builds()

project (utf8proc C)

# This is the ABI version number, which may differ from the
# API version number (defined in utf8proc.h).
# Be sure to also update these in Makefile and MANIFEST!
set(SO_MAJOR 2)
set(SO_MINOR 4)
set(SO_PATCH 1)

option(UTF8PROC_INSTALL "Enable installation of utf8proc" On)
option(UTF8PROC_ENABLE_TESTING "Enable testing of utf8proc" Off)

add_library (utf8proc
  utf8proc.c
  utf8proc.h
)

# expose header path, for when this is part of a larger cmake project
target_include_directories(utf8proc PUBLIC .)

if (BUILD_SHARED_LIBS)
  # Building shared library
else()
  # Building static library
  target_compile_definitions(utf8proc PUBLIC "UTF8PROC_STATIC")
  if (MSVC)
    set_target_properties(utf8proc PROPERTIES OUTPUT_NAME "utf8proc_static")
  endif()
endif()

target_compile_definitions(utf8proc PRIVATE "UTF8PROC_EXPORTS")

if (NOT MSVC)
  set_target_properties(
    utf8proc PROPERTIES
    COMPILE_FLAGS "-O2 -std=c99 -pedantic -Wall"
  )
endif ()

set_target_properties (utf8proc PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  VERSION "${SO_MAJOR}.${SO_MINOR}.${SO_PATCH}"
  SOVERSION ${SO_MAJOR}
)

if (UTF8PROC_INSTALL)
  install(TARGETS utf8proc
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

  install(
    FILES
      "${PROJECT_SOURCE_DIR}/utf8proc.h"
    DESTINATION include)
endif()

if(UTF8PROC_ENABLE_TESTING)
  enable_testing()
  file(MAKE_DIRECTORY data)
  set(UNICODE_VERSION 13.0.0)
  file(DOWNLOAD https://www.unicode.org/Public/${UNICODE_VERSION}/ucd/NormalizationTest.txt data/NormalizationTest.txt SHOW_PROGRESS)
  file(DOWNLOAD https://www.unicode.org/Public/${UNICODE_VERSION}/ucd/auxiliary/GraphemeBreakTest.txt data/GraphemeBreakTest.txt SHOW_PROGRESS)
  add_executable(case test/tests.h test/tests.c utf8proc.h test/case.c)
  target_link_libraries(case utf8proc)
  add_executable(custom test/tests.h test/tests.c utf8proc.h test/custom.c)
  target_link_libraries(custom utf8proc)
  add_executable(iterate test/tests.h test/tests.c utf8proc.h test/iterate.c)
  target_link_libraries(iterate utf8proc)
  add_executable(misc test/tests.h test/tests.c utf8proc.h test/misc.c)
  target_link_libraries(misc utf8proc)
  add_executable(printproperty test/tests.h test/tests.c utf8proc.h test/printproperty.c)
  target_link_libraries(printproperty utf8proc)
  add_executable(valid test/tests.h test/tests.c utf8proc.h test/valid.c)
  target_link_libraries(valid utf8proc)
  add_test(utf8proc.testcase case)
  add_test(utf8proc.testcustom custom)
  add_test(utf8proc.testiterate iterate)
  add_test(utf8proc.testmisc misc)
  add_test(utf8proc.testprintproperty printproperty)
  add_test(utf8proc.testvalid valid)

  if (NOT WIN32)
    # no wcwidth function on Windows
    add_executable(charwidth test/tests.h test/tests.c utf8proc.h test/charwidth.c)
    target_link_libraries(charwidth utf8proc)
    add_test(utf8proc.testcharwidth charwidth)
  endif()
  add_executable(graphemetest test/tests.h test/tests.c utf8proc.h test/graphemetest.c)
  target_link_libraries(graphemetest utf8proc)
  add_executable(normtest test/tests.h test/tests.c utf8proc.h test/normtest.c)
  target_link_libraries(normtest utf8proc)
  add_test(utf8proc.testgraphemetest graphemetest data/GraphemeBreakTest.txt)
  add_test(utf8proc.testnormtest normtest data/NormalizationTest.txt)
endif()
