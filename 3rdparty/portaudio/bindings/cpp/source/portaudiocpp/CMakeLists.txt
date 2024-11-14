find_package(ASIO)
if(ASIO_FOUND)
    set(portaudio-cpp-sources-asio AsioDeviceAdapter.cxx PARENT_SCOPE)
else()
    set(portaudio-cpp-sources-asio PARENT_SCOPE)
endif()

set(portaudio-cpp-sources
    ${CMAKE_CURRENT_SOURCE_DIR}/BlockingStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/CFunCallbackStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/CallbackInterface.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/CallbackStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/CppFunCallbackStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/Device.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/DirectionSpecificStreamParameters.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/Exception.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/HostApi.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/InterfaceCallbackStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/MemFunCallbackStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/Stream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/StreamParameters.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/System.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/SystemDeviceIterator.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/SystemHostApiIterator.cxx
    PARENT_SCOPE
)
