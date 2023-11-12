cmake_minimum_required(VERSION 3.9)

find_package(Doxygen)

if (NOT DOXYGEN_FOUND)
    return()
endif()

option(BUILD_DOXYGEN "Enable API documentation building via Doxygen" ON)

if (NOT BUILD_DOXYGEN)
    return()
endif()

set(top_srcdir "${PROJECT_SOURCE_DIR}")
configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)


add_custom_target(FLAC-doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/api/"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}/api")
