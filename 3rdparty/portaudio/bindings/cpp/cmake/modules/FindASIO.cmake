#[=======================================================================[.rst:
FindASIO
--------

Finds the ASIO SDK by searching for the SDK ZIP in CMAKE_PREFIX_PATH and
CMAKE_CURRENT_BINARY_DIR. Alternatively, you may manually specify the path of
the SDK ZIP with the ASIO_SDK_ZIP_PATH variable, which can be used for caching
in CI scripts.

If the ZIP is found, this module extracts it.
The ZIP extraction is skipped if the unzipped SDK is found.

This module provides an `ASIO::host` IMPORT library target for building host
applications which use ASIO drivers. If you want to build an ASIO driver, this
module may serve as a useful start but you will need to modify it.

#]=======================================================================]

if(NOT WIN32)
  message(WARNING "ASIO is only supported on Windows.")
  set(ASIO_FOUND OFF)
  return()
endif()

file(GLOB HEADER_FILE
  "${CMAKE_CURRENT_BINARY_DIR}/asiosdk*/common/asio.h"
  "${CMAKE_PREFIX_PATH}/asiosdk*/common/asio.h"
  # The old build systems before PortAudio 19.8 used to look for the ASIO SDK
  # in the same parent directory as the source code repository. This is no
  # longer advised or documented but kept for backwards compatibility.
  "${CMAKE_CURRENT_SOURCE_DIR}/../asiosdk*/common/asio.h"
)
if(NOT EXISTS "${HEADER_FILE}")
  # The file(ARCHIVE_EXTRACT) command was added in CMake 3.18, so if using an
  # older version of CMake, the user needs to extract it themselves.
  if(CMAKE_VERSION VERSION_LESS 3.18)
    message(STATUS "ASIO SDK NOT found. Download the ASIO SDK ZIP from "
      "https://www.steinberg.net/asiosdk and extract it to "
      "${CMAKE_PREFIX_PATH} or ${CMAKE_CURRENT_BINARY_DIR}"
    )
    return()
  endif()
  file(GLOB results
    "${ASIO_SDK_ZIP_PATH}"
    "${CMAKE_CURRENT_BINARY_DIR}/asiosdk*.zip"
    "${CMAKE_PREFIX_PATH}/asiosdk*.zip"
    "${CMAKE_CURRENT_SOURCE_DIR}/../asiosdk*.zip"
  )
  foreach(f ${results})
    if(EXISTS "${f}")
      message(STATUS "Extracting ASIO SDK ZIP archive: ${f}")
      file(ARCHIVE_EXTRACT INPUT "${f}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
  endforeach()
  file(GLOB HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/asiosdk*/common/asio.h")
endif()

get_filename_component(HEADER_DIR "${HEADER_FILE}" DIRECTORY)
get_filename_component(ASIO_ROOT "${HEADER_DIR}" DIRECTORY)

if(ASIO_ROOT)
  set(ASIO_FOUND TRUE)
  message(STATUS "Found ASIO SDK: ${ASIO_ROOT}")

  if(ASIO_FOUND AND NOT TARGET ASIO::host)
    add_library(ASIO::host INTERFACE IMPORTED)
    target_sources(ASIO::host INTERFACE
      "${ASIO_ROOT}/common/asio.cpp"
      "${ASIO_ROOT}/host/asiodrivers.cpp"
      "${ASIO_ROOT}/host/pc/asiolist.cpp"
    )
    target_include_directories(ASIO::host INTERFACE
      "${ASIO_ROOT}/common"
      "${ASIO_ROOT}/host"
      "${ASIO_ROOT}/host/pc"
    )
    target_link_libraries(ASIO::host INTERFACE ole32 uuid)
  endif()
else()
  message(STATUS "ASIO SDK NOT found")
endif()
