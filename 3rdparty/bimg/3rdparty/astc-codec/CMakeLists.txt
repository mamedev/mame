# Copyright 2018 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
cmake_minimum_required(VERSION 3.1.0)
project(astc-codec)

option(OPTION_ASTC_TESTS "Build all the unit tests." ON)

# TODO add support for the fuzzer, it has some additional dependencies we are not
# yet bringing in.
option(OPTION_BUILD_FUZZER "Build the fuzzer tests." OFF)

set (CMAKE_CXX_STANDARD 11)
if(OPTION_ASTC_TESTS)
  enable_testing()

  # No need to build gmock if an external project defines it.
  if(NOT TARGET gmock_main)
    # We use the approach suggested by https://crascit.com/2015/07/25/cmake-gtest/ to download gtest.
    include(ExternalProject)
    # Download and unpack googletest at configure time
    configure_file(GoogleTest-CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
                    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
    execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")

    # Prevent GoogleTest from overriding our compiler/linker options when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This adds the following targets: gtest, gtest_main, gmock and gmock_main
    add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src" "${CMAKE_BINARY_DIR}/googletest-build")
  endif()
endif()

add_subdirectory(src/base)
add_subdirectory(src/decoder)
