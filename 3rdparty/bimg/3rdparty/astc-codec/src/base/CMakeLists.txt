# Copyright 2018 Google LLC
#
# Licensed under the Apache License Version 2.0 (the License); you may not use
# this file except in compliance with the License. You may obtain a copy of the
# License at
#
# https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing software distributed
# under the License is distributed on an AS IS BASIS WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND either express or implied. See the License for the
# specific language governing permissions and limitations under the License.
add_library(base INTERFACE)
target_include_directories(base INTERFACE ../..)

if(OPTION_ASTC_TESTS)
  add_executable(base_test
                 test/bit_stream_test.cpp
                 test/bottom_n_test.cpp
                 test/math_utils_test.cpp
                 test/optional_test.cpp
                 test/string_utils_test.cpp
                 test/type_traits_test.cpp
                 test/uint128_test.cpp)
  target_link_libraries(base_test base gmock_main)
  add_test(NAME base_test COMMAND base_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
