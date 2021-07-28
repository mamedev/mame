# - Try to find PortMidi
# Once done, this will define
#
#  PortMidi_FOUND - system has PortMidi
#  PortMidi_INCLUDE_DIRS - the PortMidi include directories
#  PortMidi_LIBRARIES - link these to use PortMidi
#  PortMidi_VERSION - detected version of PortMidi
#
# See documentation on how to write CMake scripts at
# http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries

find_library(PORTMIDI_LIBRARY portmidi
  HINTS
  $ENV{PORTMIDI_DIR}
)

find_path(PORTMIDI_INCLUDE_DIR portmidi.h
  HINTS
  $ENV{PORTMIDI_DIR}
)

find_path( PORTTIME_INCLUDE_DIR porttime.h
  HINTS
  $ENV{PORTMIDI_DIR}
)

set( PORTMIDI_LIBRARIES ${PORTMIDI_LIBRARY} )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args( PortMidi REQUIRED_VARS PORTMIDI_LIBRARIES PORTMIDI_INCLUDE_DIR PORTTIME_INCLUDE_DIR )

mark_as_advanced( PORTMIDI_LIBRARY )
