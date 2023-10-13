cmake_minimum_required(VERSION 3.1.0)
project(PortAudio VERSION 19.8)

#
# General PortAudio stuff
#

option(PA_BUILD_SHARED_LIBS "Build dynamic library" ${BUILD_SHARED_LIBS})
option(PA_BUILD_TESTS "Include test projects" OFF)
option(PA_BUILD_EXAMPLES "Include example projects" OFF)

if(PA_BUILD_SHARED_LIBS)
  set(LIBRARY_BUILD_TYPE SHARED)
else()
  set(LIBRARY_BUILD_TYPE STATIC)
endif()

option(PA_WARNINGS_ARE_ERRORS "Turn compiler warnings into errors" OFF)
if(PA_WARNINGS_ARE_ERRORS)
    if(MSVC)
        add_compile_options(/WX
            # "Grandfathered" warnings that existed before we started enforcement.
            # Do *NOT* add warnings to this list. Instead, fix your code so that it doesn't produce the warning.
            # TODO: fix the offending code so that we don't have to exclude specific warnings anymore.
            /wd4244 # W2 conversion possible loss of data
            /wd4267 # W3 conversion possible loss of data
            /wd4996 # W3 unsafe/deprecated
        )
    else()
        add_compile_options(-Werror
            # "Grandfathered" warnings that existed before we started enforcement.
            # Do *NOT* add warnings to this list. Instead, fix your code so that it doesn't produce the warning.
            # TODO: fix the offending code so that we don't have to exclude specific warnings anymore.
            -Wno-error=deprecated-declarations  # https://github.com/PortAudio/portaudio/issues/213 https://github.com/PortAudio/portaudio/issues/641
            -Wno-error=stringop-overflow
        )
        if (CMAKE_C_COMPILER_ID MATCHES "Clang")
             # Don't fail on older clang versions that don't recognize the latest warnings in the list above.
             # Note that unrecognized warning options are not a fatal error on GCC, and in fact, GCC will choke on this option. Hence the conditional.
            add_compile_options(-Wno-error=unknown-warning-option)
        endif()
    endif()
endif()

add_library(PortAudio
  ${LIBRARY_BUILD_TYPE}
  src/common/pa_allocation.c
  src/common/pa_allocation.h
  src/common/pa_converters.c
  src/common/pa_converters.h
  src/common/pa_cpuload.c
  src/common/pa_cpuload.h
  src/common/pa_debugprint.c
  src/common/pa_debugprint.h
  src/common/pa_dither.c
  src/common/pa_dither.h
  src/common/pa_endianness.h
  src/common/pa_front.c
  src/common/pa_hostapi.h
  src/common/pa_memorybarrier.h
  src/common/pa_process.c
  src/common/pa_process.h
  src/common/pa_ringbuffer.c
  src/common/pa_ringbuffer.h
  src/common/pa_stream.c
  src/common/pa_stream.h
  src/common/pa_trace.c
  src/common/pa_trace.h
  src/common/pa_types.h
  src/common/pa_util.h
)

include(GNUInstallDirs)

target_include_directories(PortAudio PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/common>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if(UNIX)
  target_compile_options(PortAudio PRIVATE -fPIC)
endif()

set(PORTAUDIO_PUBLIC_HEADERS include/portaudio.h)

find_package(Threads REQUIRED)
target_link_libraries(PortAudio PRIVATE Threads::Threads)

option(PA_ENABLE_DEBUG_OUTPUT "Enable debug output for Portaudio" OFF)
if(PA_ENABLE_DEBUG_OUTPUT)
  target_compile_definitions(PortAudio PRIVATE PA_ENABLE_DEBUG_OUTPUT)
endif()

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
  target_compile_definitions(PortAudio PRIVATE PA_BIG_ENDIAN)
else()
  target_compile_definitions(PortAudio PRIVATE PA_LITTLE_ENDIAN)
endif()

if(WIN32 AND MSVC AND PA_BUILD_SHARED_LIBS
  # Check if the user is building PortAudio stand-alone or as part of a larger
  # project. If this is part of a larger project (i.e. the CMakeLists.txt has
  # been imported by some other CMakeLists.txt), we don't want to override
  # that project's global settings.
  AND "${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}")
  option(PA_DLL_LINK_WITH_STATIC_RUNTIME
    "Link with static runtime libraries (minimizes runtime dependencies)" ON)
  if(PA_DLL_LINK_WITH_STATIC_RUNTIME)
    foreach(flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()
    endforeach()
  endif()
endif()

#
# Host APIs
#

option(PA_USE_SKELETON "Use skeleton host API" OFF)
if(PA_USE_SKELETON)
  target_sources(PortAudio PRIVATE src/hostapi/skeleton/pa_hostapi_skeleton.c)
  target_compile_definitions(PortAudio PRIVATE PA_USE_SKELETON=1)
endif()

include(CMakeDependentOption)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# JACK is most commonly used on Linux, but it is cross platform, so allow building it on any OS
# if the FindJACK.cmake module finds POSIX headers.
find_package(JACK)
cmake_dependent_option(PA_USE_JACK "Enable support for JACK Audio Connection Kit" ON JACK_FOUND OFF)
if(PA_USE_JACK)
  target_link_libraries(PortAudio PRIVATE JACK::jack)
  target_sources(PortAudio PRIVATE src/hostapi/jack/pa_jack.c)
  set(PORTAUDIO_PUBLIC_HEADERS "${PORTAUDIO_PUBLIC_HEADERS}" include/pa_jack.h)
  target_compile_definitions(PortAudio PUBLIC PA_USE_JACK=1)
  set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_JACK=1")
  set(PKGCONFIG_REQUIRES_PRIVATE "${PKGCONFIG_REQUIRES_PRIVATE} jack")

  # needed for PortAudioConfig.cmake so `find_package(PortAudio)` works in downstream projects
  install(FILES cmake/modules/FindRegex.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/portaudio/modules")
  install(FILES cmake/modules/FindJACK.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/portaudio/modules")
endif()

if(WIN32)
  target_sources(PortAudio PRIVATE
    src/os/win/pa_win_coinitialize.c
    src/os/win/pa_win_coinitialize.h
    src/os/win/pa_win_hostapis.c
    src/os/win/pa_win_util.c
    src/os/win/pa_win_util.h
    src/os/win/pa_win_version.c
    src/os/win/pa_win_version.h
    src/os/win/pa_win_waveformat.c
    src/os/win/pa_win_wdmks_utils.h
    src/os/win/pa_x86_plain_converters.h
  )
  target_include_directories(PortAudio PRIVATE src/os/win)
  set(PORTAUDIO_PUBLIC_HEADERS "${PORTAUDIO_PUBLIC_HEADERS}" include/pa_win_waveformat.h)
  target_link_libraries(PortAudio PRIVATE winmm)

  if(MSVC)
    target_sources(PortAudio PRIVATE src/os/win/pa_x86_plain_converters.c)
  else()
    target_compile_definitions(PortAudio PRIVATE _WIN32_WINNT=0x0501 WINVER=0x0501)
    set(DEF_EXCLUDE_X86_PLAIN_CONVERTERS ";")
  endif()

  target_compile_definitions(PortAudio PRIVATE _CRT_SECURE_NO_WARNINGS)

  option(PA_USE_ASIO "Enable support for ASIO" OFF)
  if(PA_USE_ASIO)
    find_package(ASIO)
    # Automatically download the ASIO SDK ZIP if it is not found. The ASIO SDK license
    # allows for downloading it from Steinberg and using it without charge, but it is
    # not allowed to be redistributed.
    #
    # The file(ARCHIVE_EXTRACT) command needed to extract the ZIP archive was added in
    # CMake 3.18, so do not bother downloading the ZIP archive for older CMake versions.
    # Instead, FindASIO.cmake directs the user to manually extract the ZIP file to
    # CMAKE_PREFIX_PATH or CMAKE_CURRENT_BINARY_DIR.
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.18 AND NOT TARGET ASIO::host)
      if(NOT ASIO_SDK_ZIP_PATH)
        set(ASIO_SDK_ZIP_PATH "${CMAKE_CURRENT_BINARY_DIR}/asiosdk.zip")
      endif()
      message(STATUS "Downloading ASIO SDK... ${ASIO_SDK_ZIP_PATH}")
      file(DOWNLOAD "https://www.steinberg.net/asiosdk"
        "${ASIO_SDK_ZIP_PATH}"
        STATUS ASIO_DOWNLOAD_STATUS
        SHOW_PROGRESS
      )
      if("${ASIO_DOWNLOAD_STATUS}" EQUAL 0)
        find_package(ASIO)
      else()
        list(GET "${ASIO_DOWNLOAD_STATUS}" 1 DOWNLOAD_ERROR)
        message(FATAL_ERROR "Error downloading ASIO SDK: ${DOWNLOAD_ERROR} "
          "Reconfigure CMake with -DPA_USE_ASIO=OFF to build without ASIO. "
          "Alternatively, download the ZIP from https://www.steinberg.net/asiosdk "
          "and put it in ${CMAKE_PREFIX_PATH} or ${CMAKE_CURRENT_BINARY_DIR}"
        )
      endif()
    endif()
  endif()
  if(PA_USE_ASIO AND TARGET ASIO::host)
    target_link_libraries(PortAudio PRIVATE "$<BUILD_INTERFACE:ASIO::host>")
    set(PORTAUDIO_PUBLIC_HEADERS "${PORTAUDIO_PUBLIC_HEADERS}" include/pa_asio.h)
    target_compile_definitions(PortAudio PUBLIC PA_USE_ASIO=1)
    set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_ASIO=1")
    target_sources(PortAudio PRIVATE
      src/hostapi/asio/pa_asio.cpp
      src/hostapi/asio/iasiothiscallresolver.cpp
      src/hostapi/asio/iasiothiscallresolver.h
    )
  else()
    set(DEF_EXCLUDE_ASIO_SYMBOLS ";")
  endif()

  option(PA_USE_DS "Enable support for DirectSound" ON)
  if(PA_USE_DS)
    target_sources(PortAudio PRIVATE
      src/hostapi/dsound/pa_win_ds.c
      src/hostapi/dsound/pa_win_ds_dynlink.c
      src/hostapi/dsound/pa_win_ds_dynlink.h
    )
    target_include_directories(PortAudio PRIVATE src/hostapi/dsound)
    set(PORTAUDIO_PUBLIC_HEADERS "${PORTAUDIO_PUBLIC_HEADERS}" include/pa_win_ds.h)
    target_compile_definitions(PortAudio PUBLIC PA_USE_DS=1)
    set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_DS=1")
    target_link_libraries(PortAudio PRIVATE dsound)
    if(NOT MINGW)
      target_compile_definitions(PortAudio PRIVATE PAWIN_USE_DIRECTSOUNDFULLDUPLEXCREATE)
    endif()
  endif()

  option(PA_USE_WMME "Enable support for WMME" ON)
  if(PA_USE_WMME)
    target_sources(PortAudio PRIVATE src/hostapi/wmme/pa_win_wmme.c)
    set(PORTAUDIO_PUBLIC_HEADERS "${PORTAUDIO_PUBLIC_HEADERS}" include/pa_win_wmme.h)
    target_compile_definitions(PortAudio PUBLIC PA_USE_WMME=1)
    set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_WMME=1")
    target_link_libraries(PortAudio PRIVATE ole32 uuid)
  else()
    set(DEF_EXCLUDE_WMME_SYMBOLS ";")
  endif()

  option(PA_USE_WASAPI "Enable support for WASAPI" ON)
  if(PA_USE_WASAPI)
    target_sources(PortAudio PRIVATE src/hostapi/wasapi/pa_win_wasapi.c)
    set(PORTAUDIO_PUBLIC_HEADERS "${PORTAUDIO_PUBLIC_HEADERS}" include/pa_win_wasapi.h)
    target_compile_definitions(PortAudio PUBLIC PA_USE_WASAPI=1)
    set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_WASAPI=1")
    target_link_libraries(PortAudio PRIVATE ole32 uuid)
  else()
    set(DEF_EXCLUDE_WASAPI_SYMBOLS ";")
  endif()

  option(PA_USE_WDMKS "Enable support for WDMKS" ON)
  if(PA_USE_WDMKS)
    target_sources(PortAudio PRIVATE
      src/os/win/pa_win_wdmks_utils.c
      src/hostapi/wdmks/pa_win_wdmks.c
    )
    set(PORTAUDIO_PUBLIC_HEADERS "${PORTAUDIO_PUBLIC_HEADERS}" include/pa_win_wdmks.h)
    target_compile_definitions(PortAudio PUBLIC PA_USE_WDMKS=1)
    set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_WDMKS=1")
    target_link_libraries(PortAudio PRIVATE setupapi ole32 uuid)
  endif()

  option(PA_USE_WDMKS_DEVICE_INFO "Use WDM/KS API for device info" ON)
  if(PA_USE_WDMKS_DEVICE_INFO)
    target_compile_definitions(PortAudio PRIVATE PAWIN_USE_WDMKS_DEVICE_INFO)
  endif()

  if(PA_BUILD_SHARED_LIBS)
    configure_file(cmake/portaudio.def.in "${CMAKE_CURRENT_BINARY_DIR}/portaudio.def" @ONLY)
    target_sources(PortAudio PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/portaudio.def")
  endif()
elseif(UNIX)
  target_sources(PortAudio PRIVATE
    src/os/unix/pa_unix_hostapis.c
    src/os/unix/pa_unix_util.c
    src/os/unix/pa_unix_util.h
  )
  target_include_directories(PortAudio PRIVATE src/os/unix)
  target_link_libraries(PortAudio PRIVATE m)
  set(PKGCONFIG_LDFLAGS_PRIVATE "${PKGCONFIG_LDFLAGS_PUBLIC} -lm -lpthread")
  set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -pthread")

  if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    target_sources(PortAudio PRIVATE
      src/hostapi/coreaudio/pa_mac_core.c
      src/hostapi/coreaudio/pa_mac_core_blocking.c
      src/hostapi/coreaudio/pa_mac_core_blocking.h
      src/hostapi/coreaudio/pa_mac_core_internal.h
      src/hostapi/coreaudio/pa_mac_core_utilities.c
      src/hostapi/coreaudio/pa_mac_core_utilities.h
    )
    target_include_directories(PortAudio PRIVATE src/hostapi/coreaudio)
    set(PORTAUDIO_PUBLIC_HEADERS "${PORTAUDIO_PUBLIC_HEADERS}" include/pa_mac_core.h)

    target_link_libraries(PortAudio 
      PRIVATE
        -Wl,-framework,CoreAudio
        -Wl,-framework,AudioToolbox
        -Wl,-framework,AudioUnit
        -Wl,-framework,CoreFoundation
        -Wl,-framework,CoreServices
    )
    target_compile_definitions(PortAudio PUBLIC PA_USE_COREAUDIO=1)
    set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_COREAUDIO=1")

    # Use C11 so that we can make use of atomic library and avoid deprecation errors.
    set_property(TARGET PortAudio PROPERTY C_STANDARD 11)

    set(PKGCONFIG_LDFLAGS_PRIVATE
      "${PKGCONFIG_LDFLAGS_PRIVATE} -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreFoundation -framework CoreServices")
  else()
    # Some BSDs have a reimplementation of alsalib, so do not explicitly check for Linux.
    find_package(ALSA)
    cmake_dependent_option(PA_USE_ALSA "Enable support for ALSA" ON ALSA_FOUND OFF)
    if(PA_USE_ALSA)
      target_link_libraries(PortAudio PRIVATE "${ALSA_LIBRARIES}")
      target_sources(PortAudio PRIVATE src/hostapi/alsa/pa_linux_alsa.c)
      set(PORTAUDIO_PUBLIC_HEADERS "${PORTAUDIO_PUBLIC_HEADERS}" include/pa_linux_alsa.h)
      target_compile_definitions(PortAudio PUBLIC PA_USE_ALSA=1)
      set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_ALSA=1")

      option(PA_ALSA_DYNAMIC "Enable dynamically loading libasound with dlopen using PaAlsa_SetLibraryPathName" OFF)
      if(PA_ALSA_DYNAMIC)
        target_compile_definitions(PortAudio PRIVATE PA_ALSA_DYNAMIC)
        target_link_libraries(PortAudio PRIVATE "${CMAKE_DL_LIBS}")
        set(PKGCONFIG_LDFLAGS_PRIVATE "${PKGCONFIG_LDFLAGS_PRIVATE} -l${CMAKE_DL_LIBS}")
      else()
        set(PKGCONFIG_REQUIRES_PRIVATE "${PKGCONFIG_REQUIRES_PRIVATE} alsa")
      endif()
    endif()

    # OSS is intentionally off by default to avoid confusing users of PortAudio
    # applications. OSS builds but there are no devices available on modern
    # Linux systems.
    find_package(OSS)
    cmake_dependent_option(PA_USE_OSS "Enable support for OSS" OFF "OSS_FOUND" OFF)
    if(PA_USE_OSS)
      target_sources(PortAudio PRIVATE src/hostapi/oss/pa_unix_oss.c)
      target_compile_definitions(PortAudio PUBLIC PA_USE_OSS=1)
      set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_OSS=1")
      target_link_libraries(PortAudio PRIVATE OSS::oss)
      # The FindOSS.cmake module does not need to be installed like the JACK modules because it
      # does not link any library; it only adds an include directory and compile definition.
    endif()

    check_include_file(sys/audioio.h HAVE_SYS_AUDIOIO_H)
    cmake_dependent_option(AUDIOIO "Enable support for Solaris/NetBSD audio" ON "HAVE_SYS_AUDIOIO_H" AUDIOIO)
    if(AUDIOIO AND HAVE_SYS_AUDIOIO_H)
      target_sources(PortAudio PRIVATE src/hostapi/audioio/pa_unix_audioio.c)
      target_compile_definitions(PortAudio PUBLIC PA_USE_AUDIOIO=1)
      set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_AUDIOIO=1")
    endif()

    find_package(PulseAudio)
    cmake_dependent_option(PA_USE_PULSEAUDIO "Enable support for PulseAudio general purpose sound server" ON PulseAudio_FOUND OFF)
    if(PA_USE_PULSEAUDIO)
      target_link_libraries(PortAudio PRIVATE PulseAudio::PulseAudio)
      target_sources(PortAudio PRIVATE
        src/hostapi/pulseaudio/pa_linux_pulseaudio_block.c
        src/hostapi/pulseaudio/pa_linux_pulseaudio.c
        src/hostapi/pulseaudio/pa_linux_pulseaudio_cb.c)

      target_compile_definitions(PortAudio PUBLIC PA_USE_PULSEAUDIO=1)
      set(PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS} -DPA_USE_PULSEAUDIO=1")
      set(PKGCONFIG_REQUIRES_PRIVATE "${PKGCONFIG_REQUIRES_PRIVATE} libpulse")

      # needed for PortAudioConfig.cmake so `find_package(PortAudio)` works in downstream projects
      install(FILES cmake/modules/FindPulseAudio.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/portaudio/modules")
    endif()

  endif()
endif()

# Add public headers to sources of PortAudio (used by some IDEs to list them in project tree)
source_group("Public Header Files" FILES ${PORTAUDIO_PUBLIC_HEADERS})
target_sources(PortAudio PRIVATE ${PORTAUDIO_PUBLIC_HEADERS})

#
# Installation
#

include(CMakePackageConfigHelpers)

if(NOT CMAKE_FRAMEWORK)
  install(FILES README.md DESTINATION "${CMAKE_INSTALL_DOCDIR}/portaudio")
  install(FILES LICENSE.txt DESTINATION "${CMAKE_INSTALL_DOCDIR}/portaudio")

  configure_file(cmake/portaudio-2.0.pc.in "${CMAKE_CURRENT_BINARY_DIR}/portaudio-2.0.pc" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/portaudio-2.0.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

  configure_package_config_file(cmake/PortAudioConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/portaudio/PortAudioConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/portaudio"
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/portaudio/PortAudioConfigVersion.cmake"
    VERSION "${PORTAUDIO_VERSION}"
    COMPATIBILITY SameMajorVersion
  )
  install(EXPORT PortAudio-targets NAMESPACE "PortAudio::" FILE "PortAudioTargets.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/portaudio")
  export(TARGETS PortAudio
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/portaudio/PortAudioTargets.cmake")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/portaudio/PortAudioConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/cmake/portaudio/PortAudioConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/portaudio")

  if(NOT TARGET uninstall)
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
      IMMEDIATE @ONLY)
    add_custom_target(uninstall
      COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
  endif()
endif()

set_target_properties(PortAudio PROPERTIES
  OUTPUT_NAME portaudio
  PUBLIC_HEADER "${PORTAUDIO_PUBLIC_HEADERS}"
  MACOSX_FRAMEWORK_IDENTIFIER com.portaudio
  FRAMEWORK_VERSION A
  WINDOWS_EXPORT_ALL_SYMBOLS TRUE
  VERSION ${PROJECT_VERSION}
  SOVERSION 2
)
install(TARGETS PortAudio
  EXPORT PortAudio-targets
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  FRAMEWORK DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

#
# Subdirectories
#

# Some of the tests and examples use private symbols which are not
# exposed by the .def file on Windows.
if(WIN32 AND PA_BUILD_SHARED_LIBS)
  set(LINK_PRIVATE_SYMBOLS OFF)
else()
  set(LINK_PRIVATE_SYMBOLS ON)
endif()

if(PA_BUILD_TESTS)
  macro(add_test appl_name)
    add_executable(${appl_name} "${appl_name}.c")
    target_link_libraries(${appl_name} PortAudio Threads::Threads)
    if(UNIX)
      target_link_libraries(${appl_name} m)
    endif()
    set_target_properties(${appl_name}
        PROPERTIES
        FOLDER "Test"
    )
  endmacro()

  subdirs(test)
  subdirs(qa)
endif()

if(PA_BUILD_EXAMPLES)
  subdirs(examples)
endif()
