#[=======================================================================[.rst:
FindOSS
--------

Finds the Open Sound System include directory. There is no library to link.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``OSS::oss``
  Target for the OSS header include directory. One of the following
  compile definitions is added to the target:
  HAVE_SYS_SOUNDCARD_H if the header is sys/soundcard.h
  HAVE_LINUX_SOUNDCARD_H if the header is linux/soundcard.h
  HAVE_MACHINE_SOUNDCARD_H if the header is machine/soundcard.h

#]=======================================================================]

find_path(OSS_INCLUDE_DIR
  NAMES sys/soundcard.h
  DOC "OSS include directory")
if(OSS_INCLUDE_DIR)
  set(OSS_DEFINITIONS HAVE_SYS_SOUNDCARD_H)
else()
  find_path(OSS_INCLUDE_DIR
    NAMES linux/soundcard.h
    DOC "OSS include directory")
  if(OSS_INCLUDE_DIR)
    set(OSS_DEFINITIONS HAVE_LINUX_SOUNDCARD_H)
  else()
    find_path(OSS_INCLUDE_DIR
      NAMES machine/soundcard.h
      DOC "OSS include directory")
    if(OSS_INCLUDE_DIR)
      set(OSS_DEFINITIONS HAVE_MACHINE_SOUNDCARD_H)
    endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OSS
  DEFAULT_MSG
  OSS_INCLUDE_DIR
  OSS_DEFINITIONS
)

if(OSS_INCLUDE_DIR AND OSS_DEFINITIONS)
  set(OSS_FOUND TRUE)
  if(NOT TARGET OSS::oss)
    add_library(OSS::oss INTERFACE IMPORTED)
    target_include_directories(OSS::oss INTERFACE "${OSS_INCLUDE_DIR}")
    target_compile_definitions(OSS::oss INTERFACE "${OSS_DEFINITIONS}")
  endif()
endif()
