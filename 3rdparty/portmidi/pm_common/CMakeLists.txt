# pm_common/CMakeLists.txt -- how to build portmidi library

# creates the portmidi library
# exports PM_NEEDED_LIBS to parent. It seems that PM_NEEDED_LIBS for
#   Linux should include Thread::Thread and ALSA::ALSA, but these
#   are not visible in other CMake files, even though the portmidi
#   target is. Therefore, Thread::Thread is replaced by
#   CMAKE_THREAD_LIBS_INIT and ALSA::ALSA is replaced by ALSA_LIBRARIES.
#   Is there a better way to do this? Maybe this whole file should be
#   at the parent level.

# Support alternative name for static libraries to avoid confusion.
# (In particular, Xcode has automatically converted portmidi.a to
# portmidi.dylib without warning, so using portmidi-static.a eliminates
# this possibility, but default for all libs is "portmidi"):
set(PM_STATIC_LIB_NAME "portmidi" CACHE STRING 
    "For static builds, the PortMidi library name, e.g. portmidi-static.
     Default is portmidi")
set(PM_ACTUAL_LIB_NAME "portmidi" PARENT_SCOPE)
if(NOT BUILD_SHARED_LIBS)
  set(PM_ACTUAL_LIB_NAME ${PM_STATIC_LIB_NAME} PARENT_SCOPE)
endif()

# set the build directory for libportmidi.a to be in portmidi, not in 
#    portmidi/pm_common. Must be done here BEFORE add_library below.
if(APPLE OR WIN32)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
  # set the build directory for .dylib libraries
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
endif(APPLE OR WIN32)

# we need full paths to sources because they are shared with other targets
# (in particular pmjni). Set PMDIR to the top-level portmidi directory:
get_filename_component(PMDIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set(PM_LIB_PUBLIC_SRC ${PMDIR}/pm_common/portmidi.c
                      ${PMDIR}/pm_common/pmutil.c
                      ${PMDIR}/porttime/porttime.c)
add_library(portmidi ${PM_LIB_PUBLIC_SRC})

# MSVCRT_DLL is "DLL" for shared runtime library, and "" for static:
set_target_properties(portmidi PROPERTIES
                      VERSION ${LIBRARY_VERSION}
                      SOVERSION ${LIBRARY_SOVERSION}
                      OUTPUT_NAME "${PM_ACTUAL_LIB_NAME}"
                      MSVC_RUNTIME_LIBRARY 
                      "MultiThreaded$<$<CONFIG:Debug>:Debug>${MSVCRT_DLL}"
                      WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
target_include_directories(portmidi PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


option(PM_CHECK_ERRORS
"Insert a check for error return values at the end of each PortMidi function.
If an error is encountered, a text message is printed using printf(), the user
is asked to type ENTER, and then exit(-1) is called to clean up and terminate
the program.

You should not use PM_CHECK_ERRORS if printf() does not work (e.g. this is not
a console application under Windows, or there is no visible console on some
other OS), and you should not use PM_CHECK_ERRORS if you intend to recover
from errors rather than abruptly terminate the program." OFF)
if(PM_CHECK_ERRORS)
  target_compile_definitions(portmidi PRIVATE PM_CHECK_ERRORS)
endif(PM_CHECK_ERRORS)

macro(prepend_path RESULT PATH)
  set(${RESULT})
  foreach(FILE ${ARGN})
    list(APPEND ${RESULT} "${PATH}${FILE}")
  endforeach(FILE)
endmacro(prepend_path)

# UNIX needs pthread library
if(NOT WIN32)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
endif()

# Check for sndio
if(USE_SNDIO)
  include (FindPackageHandleStandardArgs)
  find_path(SNDIO_INCLUDE_DIRS NAMES sndio.h)
  find_library(SNDIO_LIBRARY sndio)
  find_package_handle_standard_args(Sndio
      REQUIRED_VARS SNDIO_LIBRARY SNDIO_INCLUDE_DIRS)
endif(USE_SNDIO)

# first include the appropriate system-dependent file:
if(SNDIO_FOUND AND USE_SNDIO)
  set(PM_LIB_PRIVATE_SRC
      ${PMDIR}/porttime/ptlinux.c
      ${PMDIR}/pm_sndio/pmsndio.c)
  set(PM_NEEDED_LIBS Threads::Threads ${SNDIO_LIBRARY} PARENT_SCOPE)
  target_link_libraries(portmidi PRIVATE Threads::Threads ${SNDIO_LIBRARY})
  target_include_directories(portmidi PRIVATE ${SNDIO_INCLUDE_DIRS})
elseif(UNIX AND APPLE)
  set(Threads::Threads "" PARENT_SCOPE)
  set(PM_LIB_PRIVATE_SRC 
      ${PMDIR}/porttime/ptmacosx_mach.c
      ${PMDIR}/pm_mac/pmmac.c
      ${PMDIR}/pm_mac/pmmacosxcm.c)
  set(PM_NEEDED_LIBS 
      ${CMAKE_THREAD_LIBS_INIT}
      -Wl,-framework,CoreAudio
      -Wl,-framework,CoreFoundation
      -Wl,-framework,CoreMIDI
      -Wl,-framework,CoreServices
      PARENT_SCOPE)
  target_link_libraries(portmidi PRIVATE
      Threads::Threads
      -Wl,-framework,CoreAudio
      -Wl,-framework,CoreFoundation
      -Wl,-framework,CoreMIDI
      -Wl,-framework,CoreServices
  )
  # set to CMake default; is this right?:
  set_target_properties(portmidi PROPERTIES MACOSX_RPATH ON)
elseif(HAIKU)
  set(PM_LIB_PRIVATE_SRC
      ${PMDIR}/porttime/pthaiku.cpp
      ${PMDIR}/pm_haiku/pmhaiku.cpp)
  set(PM_NEEDED_LIBS be midi midi2 PARENT_SCOPE)
  target_link_libraries(portmidi PRIVATE be midi midi2)
elseif(UNIX)
  target_compile_definitions(portmidi PRIVATE ${LINUX_FLAGS})
  set(PM_LIB_PRIVATE_SRC
      ${PMDIR}/porttime/ptlinux.c
      ${PMDIR}/pm_linux/pmlinux.c
      ${PMDIR}/pm_linux/pmlinuxnull.c)
  if(${LINUX_DEFINES} MATCHES ".*PMALSA.*")
    # Note that ALSA is not required if PMNULL is defined -- PortMidi will then
    # compile without ALSA and report no MIDI devices. Later, PMSNDIO or PMJACK
    # might be additional options.
    find_package(ALSA REQUIRED)
    list(APPEND PM_LIB_PRIVATE_SRC ${PMDIR}/pm_linux/pmlinuxalsa.c)
    set(PM_NEEDED_LIBS ${CMAKE_THREAD_LIBS_INIT} ${ALSA_LIBRARIES} PARENT_SCOPE)
    target_link_libraries(portmidi PRIVATE Threads::Threads ALSA::ALSA)
    set(PKGCONFIG_REQUIRES_PRIVATE "alsa" PARENT_SCOPE)
  else()
    message(WARNING "No PMALSA, so PortMidi will not use ALSA, "
                    "and will not find or open MIDI devices.")
    set(PM_NEEDED_LIBS ${CMAKE_THREAD_LIBS_INIT} PARENT_SCOPE)
    target_link_libraries(portmidi PRIVATE Threads::Threads)
  endif()
elseif(WIN32)
  set(PM_LIB_PRIVATE_SRC
      ${PMDIR}/porttime/ptwinmm.c
      ${PMDIR}/pm_win/pmwin.c
      ${PMDIR}/pm_win/pmwinmm.c)
    set(PM_NEEDED_LIBS winmm PARENT_SCOPE)
    target_link_libraries(portmidi PRIVATE winmm)
#  if(NOT BUILD_SHARED_LIBS AND PM_USE_STATIC_RUNTIME)
    # /MDd is multithread debug DLL, /MTd is multithread debug
    # /MD is multithread DLL, /MT is multithread. Change to static:
#    include(../pm_win/static.cmake)
#  endif()
else()
  message(FATAL_ERROR "Operating system not supported.")
endif()

set(PM_LIB_PUBLIC_SRC ${PM_LIB_PUBLIC_SRC} PARENT_SCOPE) # export to parent
set(PM_LIB_PRIVATE_SRC ${PM_LIB_PRIVATE_SRC} PARENT_SCOPE) # export to parent

target_sources(portmidi PRIVATE ${PM_LIB_PRIVATE_SRC})

