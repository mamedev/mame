// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/decoder/physical_astc_block.h"
#include "src/base/uint128.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using astc_codec::PhysicalASTCBlock;
using astc_codec::ColorEndpointMode;
using astc_codec::base::UInt128;

namespace {

static const PhysicalASTCBlock kErrorBlock(UInt128(0));

// Test to make sure that each of the constructors work and that
// they produce the same block encodings, since the ASTC blocks
// are little-endian
TEST(PhysicalASTCBlockTest, TestConstructors) {
  // Little-endian reading of bytes
  PhysicalASTCBlock blk1(0x0000000001FE000173ULL);
  PhysicalASTCBlock blk2(
      std::string("\x73\x01\x00\xFE\x01\x00\x00\x00\x00"
                  "\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
  EXPECT_EQ(blk1.GetBlockBits(), blk2.GetBlockBits());
}

// Test to see if we properly decode the maximum value that a weight
// can take in an ASTC block based on the block mode encoding. We test
// against a valid case and various error cases
TEST(PhysicalASTCBlockTest, TestWeightRange) {
  PhysicalASTCBlock blk1(0x0000000001FE000173ULL);
  auto weight_range = blk1.WeightRange();
  ASSERT_TRUE(weight_range);
  EXPECT_EQ(weight_range.value(), 7);

  // If we flip the high bit then we should have a range of 31,
  // although then we have too many bits and this should error.
  PhysicalASTCBlock blk2(0x0000000001FE000373ULL);
  EXPECT_FALSE(blk2.WeightRange());

  // One bit per weight -- range of 1
  PhysicalASTCBlock non_shared_cem(0x4000000000800D44ULL);
  weight_range = non_shared_cem.WeightRange();
  ASSERT_TRUE(weight_range);
  EXPECT_EQ(weight_range.value(), 1);

  // Error blocks have no weight range
  EXPECT_FALSE(kErrorBlock.WeightRange());
}

// Test to see if we properly decode the weight grid width and height
// in an ASTC block based on the block mode encoding. We test against
// a valid case and various error cases
TEST(PhysicalASTCBlockTest, TestWeightDims) {
  PhysicalASTCBlock blk1(0x0000000001FE000173ULL);
  auto weight_dims = blk1.WeightGridDims();
  EXPECT_TRUE(weight_dims);
  EXPECT_EQ(weight_dims.value()[0], 6);
  EXPECT_EQ(weight_dims.value()[1], 5);

  // If we flip the high bit then we should have a range of 31,
  // although then we have too many bits for the weight grid
  // and this should error.
  PhysicalASTCBlock blk2(0x0000000001FE000373ULL);
  EXPECT_FALSE(blk2.WeightGridDims());
  EXPECT_EQ(blk2.IsIllegalEncoding().value(),
            "Too many bits required for weight grid");

  // Dual plane block with 3x5 weight dims
  PhysicalASTCBlock blk3(0x0000000001FE0005FFULL);
  weight_dims = blk3.WeightGridDims();
  ASSERT_TRUE(weight_dims);
  EXPECT_EQ(weight_dims->at(0), 3);
  EXPECT_EQ(weight_dims->at(1), 5);

  // Error blocks shouldn't have any weight dims
  EXPECT_FALSE(kErrorBlock.WeightGridDims());

  PhysicalASTCBlock non_shared_cem(0x4000000000800D44ULL);
  weight_dims = non_shared_cem.WeightGridDims();
  ASSERT_TRUE(weight_dims);
  EXPECT_EQ(weight_dims->at(0), 8);
  EXPECT_EQ(weight_dims->at(1), 8);
}

// Test to see whether or not the presence of a dual-plane bit
// is decoded properly. Error encodings are tested to *not* return
// that they have dual planes.
TEST(PhysicalASTCBlockTest, TestDualPlane) {
  PhysicalASTCBlock blk1(0x0000000001FE000173ULL);
  EXPECT_FALSE(blk1.IsDualPlane());
  EXPECT_FALSE(kErrorBlock.IsDualPlane());

  // If we flip the dual plane bit, we will have too many bits
  // for the weight grid and this should error
  PhysicalASTCBlock blk2(0x0000000001FE000573ULL);
  EXPECT_FALSE(blk2.IsDualPlane());
  EXPECT_FALSE(blk2.WeightGridDims());
  EXPECT_EQ(blk2.IsIllegalEncoding().value(),
            "Too many bits required for weight grid");

  // A dual plane with 3x5 weight grid should be supported
  PhysicalASTCBlock blk3(0x0000000001FE0005FFULL);
  EXPECT_TRUE(blk3.IsDualPlane());

  // If we use the wrong block mode, then a valid block
  // shouldn't have any dual plane
  PhysicalASTCBlock blk4(0x0000000001FE000108ULL);
  EXPECT_FALSE(blk4.IsDualPlane());
  EXPECT_FALSE(blk4.IsIllegalEncoding());
}

// Make sure that we properly calculate the number of bits used to encode
// the weight grid. Given error encodings or void extent blocks, this number
// should be zero
TEST(PhysicalASTCBlockTest, TestNumWeightBits) {
  // 6x5 single-plane weight grid with 3-bit weights
  // should have 90 bits for the weights.
  PhysicalASTCBlock blk1(0x0000000001FE000173ULL);
  EXPECT_EQ(90, blk1.NumWeightBits());

  // Error block has no weight bits
  EXPECT_FALSE(kErrorBlock.NumWeightBits());

  // Void extent blocks have no weight bits
  EXPECT_FALSE(PhysicalASTCBlock(0xFFF8003FFE000DFCULL).NumWeightBits());

  // If we flip the dual plane bit, we will have too many bits
  // for the weight grid and this should error and return no bits
  PhysicalASTCBlock blk2(0x0000000001FE000573ULL);
  EXPECT_FALSE(blk2.NumWeightBits());

  // 3x5 dual-plane weight grid with 3-bit weights
  // should have 90 bits for the weights.
  PhysicalASTCBlock blk3(0x0000000001FE0005FFULL);
  EXPECT_EQ(90, blk3.NumWeightBits());
}

// Test to make sure that our weight bits start where we expect them to.
// In other words, make sure that the calculation based on the block mode for
// where the weight bits start is accurate.
TEST(PhysicalASTCBlockTest, TestStartWeightBit) {
  EXPECT_EQ(PhysicalASTCBlock(0x4000000000800D44ULL).WeightStartBit(), 64);

  // Error blocks have no weight start bit
  EXPECT_FALSE(kErrorBlock.WeightStartBit());

  // Void extent blocks have no weight start bit
  EXPECT_FALSE(PhysicalASTCBlock(0xFFF8003FFE000DFCULL).WeightStartBit());
}

// Test to make sure that we catch various different reasons for error encoding
// of ASTC blocks, but also that certain encodings aren't errors.
TEST(PhysicalASTCBlockTest, TestErrorBlocks) {
  // Various valid block modes
  EXPECT_FALSE(PhysicalASTCBlock(0x0000000001FE000173ULL).IsIllegalEncoding());
  EXPECT_FALSE(PhysicalASTCBlock(0x0000000001FE0005FFULL).IsIllegalEncoding());
  EXPECT_FALSE(PhysicalASTCBlock(0x0000000001FE000108ULL).IsIllegalEncoding());

  // This is an error because it uses an invalid block mode
  EXPECT_EQ(kErrorBlock.IsIllegalEncoding().value(), "Reserved block mode");

  // This is an error because we have too many weight bits
  PhysicalASTCBlock err_blk(0x0000000001FE000573ULL);
  EXPECT_EQ(err_blk.IsIllegalEncoding().value(),
            "Too many bits required for weight grid");

  // This is an error because we have too many weights
  PhysicalASTCBlock err_blk2 = PhysicalASTCBlock(0x0000000001FE0005A8ULL);
  EXPECT_EQ(err_blk2.IsIllegalEncoding().value(), "Too many weights specified");

  PhysicalASTCBlock err_blk3 = PhysicalASTCBlock(0x0000000001FE000588ULL);
  EXPECT_EQ(err_blk3.IsIllegalEncoding().value(), "Too many weights specified");

  // This is an error because we have too few weights
  PhysicalASTCBlock err_blk4 = PhysicalASTCBlock(0x0000000001FE00002ULL);
  EXPECT_EQ(err_blk4.IsIllegalEncoding().value(),
            "Too few bits required for weight grid");

  // Four partitions, dual plane -- should be error
  // 2x2 weight grid, 3 bits per weight
  PhysicalASTCBlock dual_plane_four_parts(0x000000000000001D1FULL);
  EXPECT_FALSE(dual_plane_four_parts.NumPartitions());
  EXPECT_EQ(dual_plane_four_parts.IsIllegalEncoding().value(),
            "Both four partitions and dual plane specified");
}

// Test to make sure that we properly identify and can manipulate void-extent
// blocks. These are ASTC blocks that only define a single color for the entire
// block.
TEST(PhysicalASTCBlockTest, TestVoidExtentBlocks) {
  // Various valid block modes that aren't void extent blocks
  EXPECT_FALSE(PhysicalASTCBlock(0x0000000001FE000173ULL).IsVoidExtent());
  EXPECT_FALSE(PhysicalASTCBlock(0x0000000001FE0005FFULL).IsVoidExtent());
  EXPECT_FALSE(PhysicalASTCBlock(0x0000000001FE000108ULL).IsVoidExtent());

  // Error block is not a void extent block
  EXPECT_FALSE(kErrorBlock.IsVoidExtent());

  // Void extent block is void extent block...
  UInt128 void_extent_encoding(0, 0xFFF8003FFE000DFCULL);
  EXPECT_FALSE(PhysicalASTCBlock(void_extent_encoding).IsIllegalEncoding());
  EXPECT_TRUE(PhysicalASTCBlock(void_extent_encoding).IsVoidExtent());

  // If we modify the high 64 bits it shouldn't change anything
  void_extent_encoding |= UInt128(0xdeadbeefdeadbeef, 0);
  EXPECT_FALSE(PhysicalASTCBlock(void_extent_encoding).IsIllegalEncoding());
  EXPECT_TRUE(PhysicalASTCBlock(void_extent_encoding).IsVoidExtent());
}

TEST(PhysicalASTCBlockTest, TestVoidExtentCoordinates) {
  // The void extent block should have texture coordinates from 0-8191
  auto coords = PhysicalASTCBlock(0xFFF8003FFE000DFCULL).VoidExtentCoords();
  EXPECT_EQ(coords->at(0), 0);
  EXPECT_EQ(coords->at(1), 8191);
  EXPECT_EQ(coords->at(2), 0);
  EXPECT_EQ(coords->at(3), 8191);

  // If we set the coords to all 1's then it's still a void extent
  // block, but there aren't any void extent coords.
  EXPECT_FALSE(PhysicalASTCBlock(0xFFFFFFFFFFFFFDFCULL).IsIllegalEncoding());
  EXPECT_TRUE(PhysicalASTCBlock(0xFFFFFFFFFFFFFDFCULL).IsVoidExtent());
  EXPECT_FALSE(PhysicalASTCBlock(0xFFFFFFFFFFFFFDFCULL).VoidExtentCoords());

  // If we set the void extent coords to something where the coords are
  // >= each other, then the encoding is illegal.
  EXPECT_TRUE(PhysicalASTCBlock(0x0008004002001DFCULL).IsIllegalEncoding());
  EXPECT_TRUE(PhysicalASTCBlock(0x0007FFC001FFFDFCULL).IsIllegalEncoding());
}

// Test to see if we can properly identify the number of partitions in a block
// In particular -- we need to make sure we properly identify single and
// multi-partition blocks, but also that void extent and error blocks don't
// return valid numbers of partitions
TEST(PhysicalASTCBlockTest, TestNumPartitions) {
  // Various valid block modes, but all single partition
  EXPECT_EQ(PhysicalASTCBlock(0x0000000001FE000173ULL).NumPartitions(), 1);
  EXPECT_EQ(PhysicalASTCBlock(0x0000000001FE0005FFULL).NumPartitions(), 1);
  EXPECT_EQ(PhysicalASTCBlock(0x0000000001FE000108ULL).NumPartitions(), 1);

  // Two to four partitions don't have enough bits for color.
  EXPECT_FALSE(PhysicalASTCBlock(0x000000000000000973ULL).NumPartitions());
  EXPECT_FALSE(PhysicalASTCBlock(0x000000000000001173ULL).NumPartitions());
  EXPECT_FALSE(PhysicalASTCBlock(0x000000000000001973ULL).NumPartitions());

  // Test against having more than one partition
  PhysicalASTCBlock non_shared_cem(0x4000000000800D44ULL);
  EXPECT_EQ(non_shared_cem.NumPartitions(), 2);
}

// Test the color endpoint modes specified for how the endpoints are encoded.
// In particular, test that shared color endpoint modes work for multi-partition
// blocks and that non-shared color endpoint modes also work.
TEST(PhysicalASTCBlockTest, TestColorEndpointModes) {
  // Four partitions -- one shared CEM
  const auto blk1 = PhysicalASTCBlock(0x000000000000001961ULL);
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(blk1.GetEndpointMode(i), ColorEndpointMode::kLDRLumaDirect);
  }

  // Void extent blocks have no endpoint modes
  EXPECT_FALSE(PhysicalASTCBlock(0xFFF8003FFE000DFCULL).GetEndpointMode(0));

  // Test out of range partitions
  EXPECT_FALSE(PhysicalASTCBlock(0x0000000001FE000173ULL).GetEndpointMode(1));
  EXPECT_FALSE(PhysicalASTCBlock(0x0000000001FE000173ULL).GetEndpointMode(-1));
  EXPECT_FALSE(PhysicalASTCBlock(0x0000000001FE000173ULL).GetEndpointMode(100));

  // Error blocks have no endpoint modes
  EXPECT_FALSE(kErrorBlock.GetEndpointMode(0));

  // Test non-shared CEMs
  PhysicalASTCBlock non_shared_cem(0x4000000000800D44ULL);
  EXPECT_EQ(non_shared_cem.GetEndpointMode(0),
            ColorEndpointMode::kLDRLumaDirect);
  EXPECT_EQ(non_shared_cem.GetEndpointMode(1),
            ColorEndpointMode::kLDRLumaBaseOffset);
}

// Make sure that if we have more than one partition then we have proper
// partition IDs (these determine which pixels correspond to which partition)
TEST(PhysicalASTCBlockTest, TestPartitionID) {
  // Valid partitions
  EXPECT_EQ(PhysicalASTCBlock(0x4000000000FFED44ULL).PartitionID(), 0x3FF);
  EXPECT_EQ(PhysicalASTCBlock(0x4000000000AAAD44ULL).PartitionID(), 0x155);

  // Error blocks have no partition IDs
  EXPECT_FALSE(kErrorBlock.PartitionID());

  // Void extent blocks have no endpoint modes
  EXPECT_FALSE(PhysicalASTCBlock(0xFFF8003FFE000DFCULL).PartitionID());
}

// Make sure that we're properly attributing the number of bits associated with
// the encoded color values.
TEST(PhysicalASTCBlockTest, TestNumColorBits) {
  // If we're using a direct luma channel, then the number of color bits is 16
  EXPECT_EQ(PhysicalASTCBlock(0x0000000001FE000173ULL).NumColorValues(), 2);
  EXPECT_EQ(PhysicalASTCBlock(0x0000000001FE000173ULL).NumColorBits(), 16);

  // Error blocks have nothing
  EXPECT_FALSE(kErrorBlock.NumColorValues());
  EXPECT_FALSE(kErrorBlock.NumColorBits());

  // Void extent blocks have four color values and 64 bits of color
  EXPECT_EQ(PhysicalASTCBlock(0xFFF8003FFE000DFCULL).NumColorValues(), 4);
  EXPECT_EQ(PhysicalASTCBlock(0xFFF8003FFE000DFCULL).NumColorBits(), 64);
}

// Make sure that we're properly decoding the range of values that each of the
// encoded color values can take
TEST(PhysicalASTCBlockTest, TestColorValuesRange) {
  // If we're using a direct luma channel, then we use two color values up to
  // a full byte each.
  EXPECT_EQ(PhysicalASTCBlock(0x0000000001FE000173ULL).ColorValuesRange(), 255);

  // Error blocks have nothing
  EXPECT_FALSE(kErrorBlock.ColorValuesRange());

  // Void extent blocks have four color values and 64 bits of color, so the
  // color range for each is sixteen bits.
  EXPECT_EQ(PhysicalASTCBlock(0xFFF8003FFE000DFCULL).ColorValuesRange(),
            (1 << 16) - 1);
}

// Test that we know where the color data starts. This is different mostly
// depending on whether or not the block is single-partition or void extent.
TEST(PhysicalASTCBlockTest, TestColorStartBits) {
  // Void extent blocks start at bit 64
  EXPECT_EQ(PhysicalASTCBlock(0xFFF8003FFE000DFCULL).ColorStartBit(), 64);

  // Error blocks don't start anywhere
  EXPECT_FALSE(kErrorBlock.ColorStartBit());

  // Single partition blocks start at bit 17
  EXPECT_EQ(PhysicalASTCBlock(0x0000000001FE000173ULL).ColorStartBit(), 17);
  EXPECT_EQ(PhysicalASTCBlock(0x0000000001FE0005FFULL).ColorStartBit(), 17);
  EXPECT_EQ(PhysicalASTCBlock(0x0000000001FE000108ULL).ColorStartBit(), 17);

  // Multi-partition blocks start at bit 29
  EXPECT_EQ(PhysicalASTCBlock(0x4000000000FFED44ULL).ColorStartBit(), 29);
  EXPECT_EQ(PhysicalASTCBlock(0x4000000000AAAD44ULL).ColorStartBit(), 29);
}

}  // namespace
