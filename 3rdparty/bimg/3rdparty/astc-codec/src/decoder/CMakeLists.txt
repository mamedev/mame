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
add_library(footprint footprint.cc)
target_link_libraries(footprint base)

add_library(astc_utils
            astc_file.cc
            endpoint_codec.cc
            integer_sequence_codec.cc
            intermediate_astc_block.cc
            logical_astc_block.cc
            partition.cc
            physical_astc_block.cc
            quantization.cc
            weight_infill.cc)
target_link_libraries(astc_utils PRIVATE base footprint)
target_include_directories(astc_utils PRIVATE ../..)

add_library(astc-codec codec.cc)
target_link_libraries(astc-codec PRIVATE astc_utils)
target_include_directories(astc-codec PUBLIC ../../include)
target_include_directories(astc-codec PRIVATE ../..)

add_executable(astc_inspector_cli tools/astc_inspector_cli.cc)
target_include_directories(astc_inspector_cli PRIVATE ../..)
target_link_libraries(astc_inspector_cli PRIVATE astc_utils)

#
# Testing
#
if(OPTION_ASTC_TESTS)
  # Note that we will execute all the tests in the project directory.
  # We do this to ensure the unit tests can pick up the required test data

  # Create interface library exposing the root as an include directory
  add_library(codec_test_dependencies INTERFACE)
  target_include_directories(codec_test_dependencies INTERFACE ../..)

  add_executable(physical_astc_block_test test/physical_astc_block_test.cc)
  add_test(NAME physical_astc_block_test COMMAND physical_astc_block_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  target_link_libraries(physical_astc_block_test astc_utils codec_test_dependencies gmock_main)

  add_executable(partition_test test/partition_test.cc)
  add_test(NAME partition_test COMMAND partition_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  target_link_libraries(partition_test PRIVATE astc_utils codec_test_dependencies gmock_main)

  add_executable(integer_sequence_codec_test test/integer_sequence_codec_test.cc)
  target_link_libraries(integer_sequence_codec_test PRIVATE astc_utils codec_test_dependencies gmock_main)

  add_executable(intermediate_astc_block_test test/intermediate_astc_block_test.cc)
  add_test(NAME intermediate_astc_block_test COMMAND intermediate_astc_block_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  target_link_libraries(intermediate_astc_block_test PRIVATE astc_utils codec_test_dependencies gmock_main)

  add_executable(quantization_test test/quantization_test.cc)
  add_test(NAME quantization_test COMMAND quantization_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  target_link_libraries(quantization_test PRIVATE astc_utils codec_test_dependencies gmock_main)

  add_executable(weight_infill_test test/weight_infill_test.cc)
  add_test(NAME weight_infill_test COMMAND weight_infill_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  target_link_libraries(weight_infill_test PRIVATE astc_utils footprint codec_test_dependencies gmock_main)

  add_executable(endpoint_codec_test test/endpoint_codec_test.cc)
  add_test(NAME endpoint_codec_test COMMAND endpoint_codec_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  target_link_libraries(endpoint_codec_test PRIVATE astc_utils codec_test_dependencies gmock_main)

  add_executable(logical_astc_block_test test/logical_astc_block_test.cc)
  add_test(NAME logical_astc_block_test COMMAND logical_astc_block_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  target_link_libraries(logical_astc_block_test PRIVATE astc_utils codec_test_dependencies gmock_main)

  add_executable(codec_test test/codec_test.cc)
  add_test(NAME codec_test COMMAND codec_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

  target_link_libraries(codec_test PRIVATE astc-codec codec_test_dependencies gmock_main)

  add_executable(footprint_test test/footprint_test.cc)
  add_test(NAME footprint_test COMMAND footprint_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  target_link_libraries(footprint_test PRIVATE footprint codec_test_dependencies gmock_main)

  if(OPTION_BUILD_FUZZER)
    message(FATAL_ERROR "Not yet supported due to missing dependencies")
    add_executable(astc_fuzzer test/astc_fuzzer.cc codec_test_dependencies gmock_main)
    target_link_libraries(astc_fuzzer PRIVATE astc-codec honggfuzz benchmark)
  endif()
endif()
