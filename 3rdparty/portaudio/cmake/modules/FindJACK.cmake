#[=======================================================================[.rst:
FindJACK
--------

Finds the JACK Audio Connection Kit library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``JACK::jack``
  The JACK library

#]=======================================================================]

# Prefer finding the libraries from pkgconfig rather than find_library. This is
# required to build with PipeWire's reimplementation of the JACK library.
#
# This also enables using PortAudio with the jack2 port in vcpkg. That only
# builds JackWeakAPI (not the JACK server) which dynamically loads the real
# JACK library and forwards API calls to it. JackWeakAPI requires linking `dl`
# in addition to jack, as specified in the pkgconfig file in vcpkg.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(JACK jack)
else()
  find_library(JACK_LINK_LIBRARIES
    NAMES jack
    DOC "JACK library"
  )
  find_path(JACK_INCLUDEDIR
    NAMES jack/jack.h
    DOC "JACK header"
  )
endif()

find_package(Regex)
list(APPEND JACK_LINK_LIBRARIES Regex::regex)

if(NOT CMAKE_USE_PTHREADS_INIT)
    # This CMake find module is provided by the pthreads port in vcpkg.
    find_package(pthreads)
    list(APPEND JACK_LINK_LIBRARIES PThreads4W::PThreads4W)
endif()

if(CMAKE_USE_PTHREADS_INIT OR TARGET PThreads4W::PThreads4W)
  set(PTHREADS_AVAILABLE TRUE)
else()
  set(PTHREADS_AVAILABLE FALSE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  JACK
  DEFAULT_MSG
  JACK_LINK_LIBRARIES
  JACK_INCLUDEDIR
  PTHREADS_AVAILABLE
  Regex_FOUND
)

if(JACK_FOUND AND NOT TARGET JACK::jack)
  add_library(JACK::jack INTERFACE IMPORTED)
  target_link_libraries(JACK::jack INTERFACE "${JACK_LINK_LIBRARIES}" Regex::regex)
  target_include_directories(JACK::jack INTERFACE "${JACK_INCLUDEDIR}")
endif()
