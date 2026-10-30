# portmidi
# Roger B. Dannenberg (and others)
# Sep 2009 - 2021

cmake_minimum_required(VERSION 3.21)
# (ALSA::ALSA new in 3.12 and used in pm_common/CMakeLists.txt)
# Some Java stuff failed on 3.17 but works with 3.20+

cmake_policy(SET CMP0091 NEW) # enables MSVC_RUNTIME_LIBRARY target property

# Previously, PortMidi versions were simply SVN commit version numbers.
# Versions are now in the form x.y.z
# Changed 1.0 to 2.0 because API is extended with virtual ports:
set(SOVERSION "2")
set(VERSION "2.0.7")

project(portmidi VERSION "${VERSION}"
                 DESCRIPTION "Cross-Platform MIDI IO")

set(LIBRARY_SOVERSION "${SOVERSION}")
set(LIBRARY_VERSION "${VERSION}")

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

option(PM_USE_STATIC_RUNTIME 
       "Use MSVC static runtime. Only applies when BUILD_SHARED_LIBS is OFF"
       ON)

option(USE_SNDIO "Use sndio" OFF)

# MSVCRT_DLL is used to construct the MSVC_RUNTIME_LIBRARY property
#  (see pm_common/CMakeLists.txt and pm_test/CMakeLists.txt)
if(PM_USE_STATIC_RUNTIME AND NOT BUILD_SHARED_LIBS)
  set(MSVCRT_DLL "")
else()
  set(MSVCRT_DLL "DLL")
endif()

# Always build with position-independent code (-fPIC)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9 CACHE STRING 
    "make for this OS version or higher")

# PM_ACTUAL_LIB_NAME is in this scope -- see pm_common/CMakeLists.txt
# PM_NEEDED_LIBS is in this scope -- see pm_common/CMakeLists.txt

include(GNUInstallDirs)

# Build Types
# credit: http://cliutils.gitlab.io/modern-cmake/chapters/features.html
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS 
      "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
               "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# where to put libraries? Everything goes here in this directory
# (or Debug or Release, depending on the OS)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(BUILD_JAVA_NATIVE_INTERFACE
       "build the Java PortMidi interface library" OFF)

# Defines are used in both portmidi (in pm_common/) and pmjni (in pm_java),
# so define them here to be inherited by both libraries.
#
# PortMidi software architecture supports multiple system API's to lower-
# level MIDI drivers, e.g. PMNULL (no drivers), Jack (but not supported yet),
# and sndio (BSD, not supported yet). Interfaces are selected by defining,
# e.g., PMALSA. (In principle, we should require PMCOREMIDI (for macOS)
# and PMWINMM (for windows), but these are assumed.
#
if(APPLE OR WIN32)
else(APPLE OR WIN32)
  set(LINUX_DEFINES "PMALSA" CACHE STRING "must define either PMALSA or PMNULL")
  add_compile_definitions(${LINUX_DEFINES})
endif(APPLE OR WIN32)

if(BUILD_JAVA_NATIVE_INTERFACE)
  message(WARNING
    "Java API and PmDefaults program updated 2021, but support has "
    "been discontinued. If you need/use this, let developers know.")
  set(PMJNI_IF_EXISTS "pmjni")  # used by INSTALL below
else(BUILD_JAVA_NATIVE_INTERFACE)
  set(PMJNI_IF_EXISTS "")       # used by INSTALL below
endif(BUILD_JAVA_NATIVE_INTERFACE)


# Something like this might help if you need to build for a specific cpu type:
# set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING
#     "change to support other architectures" FORCE)

include_directories(pm_common porttime)
add_subdirectory(pm_common)

option(BUILD_PORTMIDI_TESTS
       "Build test programs, including midi monitor (mm)" OFF)
if(BUILD_PORTMIDI_TESTS)
    add_subdirectory(pm_test)
endif(BUILD_PORTMIDI_TESTS)

# See note above about Java support (probably) discontinued
if(BUILD_JAVA_NATIVE_INTERFACE)
  add_subdirectory(pm_java)
endif(BUILD_JAVA_NATIVE_INTERFACE)

# Install the libraries and headers (Linux and Mac OS X command line)
INSTALL(TARGETS portmidi ${PMJNI_IF_EXISTS}
  EXPORT PortMidiTargets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

INSTALL(FILES
  pm_common/portmidi.h
  pm_common/pmutil.h 
  porttime/porttime.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Documentation
option(BUILD_DOC "Build documentation" OFF)
if(BUILD_DOC)
  find_package(Doxygen)
  if (NOT DOXYGEN_FOUND)
    message(ERROR "Doxygen is needed to build the documentation")
  endif()

  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  add_custom_target(doc ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )
endif (BUILD_DOC)

# pkgconfig - generate pc file
# See https://cmake.org/cmake/help/latest/command/configure_file.html
if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
    set(PKGCONFIG_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
else()
    set(PKGCONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
    set(PKGCONFIG_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
else()
    set(PKGCONFIG_LIBDIR "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packaging/portmidi.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/packaging/portmidi.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packaging/portmidi.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# CMake config
set(PORTMIDI_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/PortMidi")
install(
  EXPORT PortMidiTargets
  FILE PortMidiTargets.cmake
  NAMESPACE PortMidi::
  DESTINATION "${PORTMIDI_INSTALL_CMAKEDIR}"
)
include(CMakePackageConfigHelpers)
configure_package_config_file(packaging/PortMidiConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/PortMidiConfig.cmake"
  INSTALL_DESTINATION "${PORTMIDI_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/PortMidiConfigVersion.cmake"
  VERSION "${CMAKE_PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/packaging/PortMidiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/packaging/PortMidiConfigVersion.cmake"
  DESTINATION "${PORTMIDI_INSTALL_CMAKEDIR}"
)




# Finding out what CMake is doing is really hard, e.g. COMPILE_FLAGS
# does not include COMPILE_OPTIONS or COMPILE_DEFINTIONS. Thus, the
# following report is probably not complete...
MESSAGE(STATUS "PortMidi Library name: " ${PM_ACTUAL_LIB_NAME})
MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
MESSAGE(STATUS "Build shared library: " ${BUILD_SHARED_LIBS})
MESSAGE(STATUS "Compiler flags: " ${CMAKE_CXX_COMPILE_FLAGS})
get_directory_property(prop COMPILE_DEFINITIONS)
MESSAGE(STATUS "Compile definitions: " ${prop})
get_directory_property(prop COMPILE_OPTIONS)
MESSAGE(STATUS "Compile options: " ${prop})
MESSAGE(STATUS "Compiler cxx debug flags: " ${CMAKE_CXX_FLAGS_DEBUG})
MESSAGE(STATUS "Compiler cxx release flags: " ${CMAKE_CXX_FLAGS_RELEASE})
MESSAGE(STATUS "Compiler cxx min size flags: " ${CMAKE_CXX_FLAGS_MINSIZEREL})
MESSAGE(STATUS "Compiler cxx flags: " ${CMAKE_CXX_FLAGS})

