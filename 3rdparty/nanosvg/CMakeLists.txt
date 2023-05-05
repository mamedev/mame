cmake_minimum_required(VERSION 3.10)

project(NanoSVG C)

# CMake needs *.c files to do something useful
configure_file(src/nanosvg.h ${CMAKE_CURRENT_BINARY_DIR}/nanosvg.c)
configure_file(src/nanosvgrast.h ${CMAKE_CURRENT_BINARY_DIR}/nanosvgrast.c)

add_library(nanosvg ${CMAKE_CURRENT_BINARY_DIR}/nanosvg.c)

find_library(MATH_LIBRARY m) # Business as usual
if(MATH_LIBRARY)
    target_link_libraries(nanosvg PUBLIC ${MATH_LIBRARY})
endif()

target_include_directories(nanosvg PUBLIC $<INSTALL_INTERFACE:include/nanosvg>)
target_compile_definitions(nanosvg PRIVATE NANOSVG_IMPLEMENTATION)

# Same for nanosvgrast
add_library(nanosvgrast ${CMAKE_CURRENT_BINARY_DIR}/nanosvgrast.c)
target_link_libraries(nanosvgrast PUBLIC nanosvg)
target_include_directories(nanosvgrast PRIVATE src)
target_compile_definitions(nanosvgrast PRIVATE NANOSVGRAST_IMPLEMENTATION)

# Installation and export:

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION 1.0
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS nanosvg nanosvgrast
        EXPORT ${PROJECT_NAME}Targets
)

export(EXPORT ${PROJECT_NAME}Targets 
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake" 
       NAMESPACE ${PROJECT_NAME}::
)

set(ConfigPackageLocation lib/cmake/${PROJECT_NAME})

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${ConfigPackageLocation}
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(
    FILES
      src/nanosvg.h
      src/nanosvgrast.h
    DESTINATION
      include/nanosvg
  )

install(EXPORT ${PROJECT_NAME}Targets
  FILE
    ${PROJECT_NAME}Targets.cmake
  NAMESPACE
    ${PROJECT_NAME}::
  DESTINATION
    ${ConfigPackageLocation}
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
)