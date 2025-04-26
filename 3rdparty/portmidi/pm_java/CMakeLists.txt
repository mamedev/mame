# pm_java/CMakeLists.txt -- builds pmjni and pmdefaults program

find_package(Java)
message(STATUS "Java_JAVA_EXECUTABLE is " ${Java_JAVA_EXECUTABLE})

if(BUILD_PMDEFAULTS)
  # Since pmdefaults and pmdefaults.bat are in the source tree and look
  # for libpmjni in the same tree, out-of-source build will not create
  # a working pmdefaults (without more work):
  if(NOT (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR}))
    message(WARNING "PmDefaults only works when run in the pm_java/pmdefaults directory after an in-source build. This is an out-of-source build, so [probably] libpmjni will be built out-of-source, pmdefaults.jar will be built in-source, and the pmdefaults[.bat] script remains in-source. Rebuild in-source to use PmDefaults or adjust the file locations and scripts to suit yourself.")
  endif()

  set(JPORTMIDICLASS JPortMidi.class JPortMidiException.class
                     JPortMidiApi.class)
  set(PMDEFAULTSCLASS PmDefaultsFrame.class PmDefaults.class)
  prepend_path(JPORTMIDIPATH jportmidi/ ${JPORTMIDICLASS})
  prepend_path(PMDEFAULTSPATH pmdefaults/ ${PMDEFAULTSCLASS})
  set(PMDEFAULTS_ALL ${JPORTMIDIPATH} ${PMDEFAULTSPATH})

  project(pmdefaults NONE)
  include(UseJava)
  add_jar(pmdefaults
          SOURCES pmdefaults/PmDefaults.java pmdefaults/PmDefaultsFrame.java
                  jportmidi/JPortMidi.java jportmidi/JPortMidiApi.java
                  jportmidi/JPortMidiException.java
          RESOURCES NAMESPACE "." pmdefaults/portmusic_logo.png
          MANIFEST pmdefaults/manifest.txt
          OUTPUT_DIR pmdefaults)
  if(WIN32)
    set(JAVAVM_LIB ${JAVA_INCLUDE_PATH}/../jre/lib/i386/client/libjvm.so)
  endif()
  add_dependencies(pmdefaults pmjni)
endif(BUILD_PMDEFAULTS)

# Build pmjni
# this CMakeLists.txt is only loaded if BUILD_JAVA_NATIVE_INTERFACE
# This jni library includes portmidi sources to give just
# one library for JPortMidi users to manage rather than two.
if(UNIX)
  include(FindJNI)
  # message(STATUS "JAVA_JVM_LIB_PATH is " ${JAVA_JVM_LIB_PATH})
  # message(STATUS "JAVA_INCLUDE_PATH is " ${JAVA_INCLUDE_PATH})
  # note: should use JAVA_JVM_LIB_PATH, but it is not set properly
  # note: user might need to set JAVA_INCLUDE_PATH manually
  #
  # this will probably break on BSD and other Unix systems; the fix
  # depends on whether FindJNI can find Java or not. If yes, then
  # we should try to rely on automatically set JAVA_INCLUDE_PATH and
  # JAVA_INCLUDE_PATH2; if no, then we need to make both JAVA_INCLUDE_PATH
  # and JAVA_INCLUDE_PATH2 set by user (will need clear documentation
  # because JAVA_INCLUDE_PATH2 is pretty obscure)
  set(JAVA_INCLUDE_PATH  ${JAVA_INCLUDE_PATH-UNKNOWN}
      CACHE STRING "where to find Java SDK include directory")
  # libjvm.so is found relative to JAVA_INCLUDE_PATH:
  if (HAIKU)
    set(JAVA_INCLUDE_PATHS ${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH}/haiku)
  else()
    set(JAVA_INCLUDE_PATHS ${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH}/linux)
  endif()
elseif(WIN32)
  include(FindJNI)
  # note: should use JAVA_JVM_LIB_PATH, but it is not set properly
  set(JAVAVM_LIB ${JAVA_INCLUDE_PATH}/../lib/jvm.lib)

  set(JAVA_INCLUDE_PATHS ${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})
  # message(STATUS "JAVA_INCLUDE_PATHS: " ${JAVA_INCLUDE_PATHS})
  # message(STATUS "JAVAVM_LIB: " ${JAVAVM_LIB})
endif()

add_library(pmjni SHARED pmjni/pmjni.c)
target_sources(pmjni PRIVATE ${PM_LIB_PUBLIC_SRC} ${PM_LIB_PRIVATE_SRC})
message(STATUS "Java paths ${JAVA_INCLUDE_PATHS}")
# message(STATUS "Java pmjni src: pmjni/pmjni.c ${PM_LIB_SHARED_SRC} "
#                "${PM_LIB_PRIVATE_SRC}")
target_include_directories(pmjni PUBLIC ${JAVA_INCLUDE_PATHS})
target_link_libraries(pmjni ${PM_NEEDED_LIBS})
set_target_properties(pmjni PROPERTIES
                            VERSION ${LIBRARY_VERSION}
                            SOVERSION ${LIBRARY_SOVERSION}
                            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
                            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
                            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
                            EXECUTABLE_EXTENSION "jnilib"
                            MACOSX_RPATH ON)

