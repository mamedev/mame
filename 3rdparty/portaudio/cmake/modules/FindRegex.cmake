#[=======================================================================[.rst:
FindRegex
--------

Finds an implementation of POSIX regular expressions. It first checks if the
standard regex.h POSIX header is available. If not, it looks for the TRE library.
MinGW does not come with regex.h, so TRE is useful with MinGW.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Regex::regex``
  The POSIX regular expression implementation

#]=======================================================================]

include(FindPackageHandleStandardArgs)
include(CheckIncludeFile)

set(CMAKE_REQUIRED_QUIET TRUE)
check_include_file(regex.h REGEX_H)
set(CMAKE_REQUIRED_QUIET FALSE)

if(REGEX_H)
  # No need to add any include directories or link any libraries;
  # simply provide a dummy target.
  if(NOT TARGET Regex::regex)
    add_library(Regex::regex INTERFACE IMPORTED)
  endif()

  # check_include_file sets the variable to "1" which looks odd in the output
  # of find_package_handle_standard_args, so show the user what was actually found.
  set(REGEX_H "POSIX regex.h")
  find_package_handle_standard_args(
    Regex
    DEFAULT_MSG
    REGEX_H
  )
else()
  # MinGW does not include regex.h but this can be supplied by the TRE library.
  find_path(TRE_REGEX_H NAMES tre/regex.h)
  if(TRE_REGEX_H)
    # The POSIX #include is simply <regex.h> but the tre regex.h is at <tre/regex.h>,
    # so add the directory containing tre's headers to the include path.
    set(TRE_INCLUDE_DIR "${TRE_REGEX_H}/tre")
  endif()
  find_library(TRE_LIBRARY NAMES tre)
  if(TRE_REGEX_H AND TRE_LIBRARY)
    message(STATUS "Found regex.h from TRE")
  else()
    message(STATUS "regex.h POSIX header NOT found and NOT available from TRE")
  endif()

  if(NOT TARGET Regex::regex)
    add_library(Regex::regex INTERFACE IMPORTED)
    target_include_directories(Regex::regex INTERFACE "${TRE_INCLUDE_DIR}")
    target_link_libraries(Regex::regex INTERFACE "${TRE_LIBRARY}")
  endif()

  find_package_handle_standard_args(
    Regex
    DEFAULT_MSG
    TRE_INCLUDE_DIR
    TRE_LIBRARY
  )
endif()
