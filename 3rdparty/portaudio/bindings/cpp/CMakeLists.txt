cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

project(PortAudioCpp VERSION 19.8 LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Todo (multi-generator): Add support for multiple generators like: - {Debug,
# Release} x {Static, Dynamic} x {MT, MD (Windows only)}

# ##############################################################################
# sources and headers
# ##############################################################################

set(portaudiocpp-sources
    source/portaudiocpp/BlockingStream.cxx
    source/portaudiocpp/CFunCallbackStream.cxx
    source/portaudiocpp/CallbackInterface.cxx
    source/portaudiocpp/CallbackStream.cxx
    source/portaudiocpp/CppFunCallbackStream.cxx
    source/portaudiocpp/Device.cxx
    source/portaudiocpp/DirectionSpecificStreamParameters.cxx
    source/portaudiocpp/Exception.cxx
    source/portaudiocpp/HostApi.cxx
    source/portaudiocpp/InterfaceCallbackStream.cxx
    source/portaudiocpp/MemFunCallbackStream.cxx
    source/portaudiocpp/Stream.cxx
    source/portaudiocpp/StreamParameters.cxx
    source/portaudiocpp/System.cxx
    source/portaudiocpp/SystemDeviceIterator.cxx
    source/portaudiocpp/SystemHostApiIterator.cxx)

# since we don't GLOBing this variable must be kept up to date otherwise user
# installations are broken.
set(portaudiocpp-header-files
    include/portaudiocpp/AutoSystem.hxx
    include/portaudiocpp/BlockingStream.hxx
    include/portaudiocpp/CFunCallbackStream.hxx
    include/portaudiocpp/CallbackInterface.hxx
    include/portaudiocpp/CallbackStream.hxx
    include/portaudiocpp/CppFunCallbackStream.hxx
    include/portaudiocpp/Device.hxx
    include/portaudiocpp/DirectionSpecificStreamParameters.hxx
    include/portaudiocpp/Exception.hxx
    include/portaudiocpp/HostApi.hxx
    include/portaudiocpp/InterfaceCallbackStream.hxx
    include/portaudiocpp/MemFunCallbackStream.hxx
    include/portaudiocpp/PortAudioCpp.hxx
    include/portaudiocpp/SampleDataFormat.hxx
    include/portaudiocpp/Stream.hxx
    include/portaudiocpp/StreamParameters.hxx
    include/portaudiocpp/System.hxx
    include/portaudiocpp/SystemDeviceIterator.hxx
    include/portaudiocpp/SystemHostApiIterator.hxx)

if(WIN32)
  find_package(ASIO MODULE)
  if(ASIO_FOUND)
    list(APPEND portaudiocpp-sources source/portaudiocpp/AsioDeviceAdapter.cxx)
    list(APPEND portaudiocpp-header-files
        include/portaudiocpp/AsioDeviceAdapter.hxx)
  endif()
endif()

# ##############################################################################
# portaudiocpp-targets
# ##############################################################################

add_library(portaudiocpp ${portaudiocpp-sources})
add_library(PortAudio::portaudiocpp ALIAS portaudiocpp) # For subdirectory build

find_package(PortAudio MODULE REQUIRED)

target_link_libraries(portaudiocpp PUBLIC PortAudio::portaudio)
target_include_directories(
  portaudiocpp PUBLIC $<INSTALL_INTERFACE:include>
                      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
set_target_properties(portaudiocpp PROPERTIES SOVERSION 2)
# Todo (modernize): update the code at least to c++14
# target_compile_features(portaudiocpp PUBLIC cxx_std_14)

# ## Export ###
include(GNUInstallDirs)

install(
  TARGETS portaudiocpp
  EXPORT PortAudioCppTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/portaudiocpp)

install(FILES ${portaudiocpp-header-files}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/portaudiocpp)

install(
  EXPORT PortAudioCppTargets
  FILE PortAudioCppTargets.cmake
  NAMESPACE PortAudio::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PortAudio)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PortAudioCppConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/PortAudioCppConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PortAudio)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/PortAudioCppConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PortAudioCppConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/PortAudioCppConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PortAudio)

#use relative path, since CMAKE can't reconfigure on install with different prefix path 
set(PC_PREFIX "\${pcfiledir}/../..")  
configure_file(cmake/portaudiocpp.pc.in "${CMAKE_CURRENT_BINARY_DIR}/portaudiocpp.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/portaudiocpp.pc"
        CONFIGURATIONS Release RelWithDebInfo
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
