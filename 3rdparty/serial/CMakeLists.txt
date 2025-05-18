cmake_minimum_required(VERSION 3.15)

project(serial CXX)

# Platform-specific source files
set(SERIAL_PLATFORM_SRCS)
if(WINDOWS)
    list(APPEND SERIAL_PLATFORM_SRCS
        src/impl/win.cc
        src/impl/list_ports/list_ports_win.cc
    )
elseif(UNIX)
    list(APPEND SERIAL_PLATFORM_SRCS
        src/impl/unix.cc
        src/impl/list_ports/list_ports_linux.cc
    )
    if(APPLE)
        list(APPEND SERIAL_PLATFORM_SRCS src/impl/list_ports/list_ports_osx.cc)
    endif()
endif()

# Core serial library sources
set(SERIAL_SRCS
    src/serial.cc
    ${SERIAL_PLATFORM_SRCS}
)

# Create static library
add_library(serial STATIC ${SERIAL_SRCS})

# Include directories
target_include_directories(serial PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set C++ standard
set_target_properties(serial PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

# Platform-specific libraries
if(WINDOWS)
    target_link_libraries(serial PRIVATE setupapi)
elseif(UNIX)
    if(APPLE)
        find_library(IOKIT_LIBRARY IOKit)
        find_library(FOUNDATION_LIBRARY Foundation)
        target_link_libraries(serial PRIVATE ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
    else()
        target_link_libraries(serial PRIVATE rt pthread)
    endif()
endif()
