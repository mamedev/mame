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

#include "src/decoder/intermediate_astc_block.h"
#include "src/decoder/test/image_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

namespace astc_codec {

namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::SizeIs;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

// Test to make sure that unpacking an error block returns false.
TEST(IntermediateASTCBlockTest, TestUnpackError) {
  const PhysicalASTCBlock kErrorBlock(base::UInt128(0));
  EXPECT_FALSE(UnpackVoidExtent(kErrorBlock));
  EXPECT_FALSE(UnpackIntermediateBlock(kErrorBlock));
}

// Test to make sure that if we don't populate our weight data in the
// intermediate block than the resulting color range should error due to the
// mismatch.
TEST(IntermediateASTCBlockTest, TestEndpointRangeErrorOnNotSettingWeights) {
  IntermediateBlockData data;
  data.weight_range = 15;
  for (auto& ep : data.endpoints) {
    ep.mode = ColorEndpointMode::kLDRRGBDirect;
  }
  data.weight_grid_dim_x = 6;
  data.weight_grid_dim_y = 6;
  EXPECT_EQ(-1, EndpointRangeForBlock(data));

  base::UInt128 dummy;
  auto err_str = Pack(data, &dummy);
  EXPECT_TRUE(err_str.hasValue());
  EXPECT_THAT(err_str.value(), HasSubstr("Incorrect number of weights"));
}

// Test to make sure that if we run out of bits, then we should say so.
TEST(IntermediateASTCBlockTest, TestEndpointRangeErrorOnNotEnoughBits) {
  IntermediateBlockData data;
  data.weight_range = 1;
  data.partition_id = 0;
  data.endpoints.resize(3);
  for (auto& ep : data.endpoints) {
    ep.mode = ColorEndpointMode::kLDRRGBDirect;
  }
  data.weight_grid_dim_x = 8;
  data.weight_grid_dim_y = 8;
  EXPECT_EQ(-2, EndpointRangeForBlock(data));

  // Resize the weights to get past the error that they do not match the grid
  // dimensions.
  data.weights.resize(64);

  base::UInt128 dummy;
  auto err_str = Pack(data, &dummy);
  EXPECT_TRUE(err_str.hasValue());
  EXPECT_THAT(err_str.value(), HasSubstr("illegal color range"));
}

// Test to make sure that as we increase the number of weights, we decrease the
// allowable range of colors
TEST(IntermediateASTCBlockTest, TestEndpointRangeForBlock) {
  IntermediateBlockData data;
  data.weight_range = 2;
  data.endpoints.resize(2);
  data.dual_plane_channel.clear();
  for (auto& ep : data.endpoints) {
    ep.mode = ColorEndpointMode::kLDRRGBDirect;
  }

  // Weight params control how many weights are present in a block
  struct WeightParams {
    int width;
    int height;

    // We should sort based on number of weights for these params
    int NumWeights() const { return width * height; }
    bool operator<(const WeightParams& other) const {
      return NumWeights() < other.NumWeights();
    }
  };

  std::vector<WeightParams> weight_params;
  for (int y = 2; y < 8; ++y) {
    for (int x = 2; x < 8; ++x) {
      weight_params.emplace_back(WeightParams{x, y});
    }
  }

  // Sort weights from fewest to largest such that the allowable color range
  // should be monotonically decreasing
  std::sort(weight_params.begin(), weight_params.end());

  // Keep track of the largest available color range and measure that it
  // decreases as we add more weights to our block
  int last_color_range = 255;
  for (const auto& params : weight_params) {
    data.weight_grid_dim_x = params.width;
    data.weight_grid_dim_y = params.height;

    const int color_range = EndpointRangeForBlock(data);
    EXPECT_LE(color_range, last_color_range);
    last_color_range = std::min(color_range, last_color_range);
  }

  // Make sure that we actually changed it at some point.
  EXPECT_LT(last_color_range, 255);
}

// Test to make sure that unpacking an legitimate ASTC block returns the encoded
// values that we expect.
TEST(IntermediateASTCBlockTest, TestUnpackNonVoidExtentBlock) {
  PhysicalASTCBlock blk(0x0000000001FE000173ULL);
  auto b = UnpackIntermediateBlock(blk);
  ASSERT_TRUE(b);

  const auto& data = b.value();
  EXPECT_EQ(data.weight_grid_dim_x, 6);
  EXPECT_EQ(data.weight_grid_dim_y, 5);
  EXPECT_EQ(data.weight_range, 7);

  EXPECT_FALSE(data.partition_id);
  EXPECT_FALSE(data.dual_plane_channel);

  ASSERT_EQ(data.weights.size(), 30);
  for (auto weight : data.weights) {
    EXPECT_EQ(weight, 0);
  }

  ASSERT_EQ(data.endpoints.size(), 1);
  for (const auto& ep_data : data.endpoints) {
    EXPECT_EQ(ep_data.mode, ColorEndpointMode::kLDRLumaDirect);
    ASSERT_EQ(ep_data.colors.size(), 2);
    EXPECT_EQ(ep_data.colors[0], 0);
    EXPECT_EQ(ep_data.colors[1], 255);
  }
}

// Make sure that we can pack blocks that aren't void extent blocks. (In other
// words, can we actually deal with intermediate ASTC data).
TEST(IntermediateASTCBlockTest, TestPackNonVoidExtentBlock) {
  IntermediateBlockData data;

  data.weight_grid_dim_x = 6;
  data.weight_grid_dim_y = 5;
  data.weight_range = 7;

  data.partition_id = {};
  data.dual_plane_channel = {};

  data.weights.resize(30);
  for (auto& weight : data.weights) {
    weight = 0;
  }

  data.endpoints.resize(1);
  for (auto& ep_data : data.endpoints) {
    ep_data.mode = ColorEndpointMode::kLDRLumaDirect;
    ep_data.colors.resize(2);
    ep_data.colors[0] = 0;
    ep_data.colors[1] = 255;
  }

  base::UInt128 packed;
  auto error_str = Pack(data, &packed);
  ASSERT_FALSE(error_str) << (error_str ? error_str.value() : std::string(""));
  EXPECT_EQ(packed, 0x0000000001FE000173ULL);
}

// Make sure that we can unpack void extent blocks
TEST(IntermediateASTCBlockTest, TestUnpackVoidExtentBlock) {
  PhysicalASTCBlock void_extent_block(0xFFFFFFFFFFFFFDFCULL);

  auto b = UnpackVoidExtent(void_extent_block);
  ASSERT_TRUE(b);

  const auto& data = b.value();
  EXPECT_EQ(data.r, 0);
  EXPECT_EQ(data.g, 0);
  EXPECT_EQ(data.b, 0);
  EXPECT_EQ(data.a, 0);
  for (const auto& coord : data.coords) {
    EXPECT_EQ(coord, (1 << 13) - 1);
  }

  base::UInt128 more_interesting(0xdeadbeefdeadbeefULL, 0xFFF8003FFE000DFCULL);
  b = UnpackVoidExtent(PhysicalASTCBlock(more_interesting));
  ASSERT_TRUE(b);

  const auto& other_data = b.value();
  EXPECT_EQ(other_data.r, 0xbeef);
  EXPECT_EQ(other_data.g, 0xdead);
  EXPECT_EQ(other_data.b, 0xbeef);
  EXPECT_EQ(other_data.a, 0xdead);
  EXPECT_EQ(other_data.coords[0], 0);
  EXPECT_EQ(other_data.coords[1], 8191);
  EXPECT_EQ(other_data.coords[2], 0);
  EXPECT_EQ(other_data.coords[3], 8191);
}

// Make sure that we can pack void extent blocks and void extent data.
TEST(IntermediateASTCBlockTest, TestPackVoidExtentBlock) {
  VoidExtentData data;
  data.r = 0;
  data.g = 0;
  data.b = 0;
  data.a = 0;
  for (auto& coord : data.coords) {
    coord = (1 << 13) - 1;
  }

  base::UInt128 packed;
  auto error_str = Pack(data, &packed);
  ASSERT_FALSE(error_str) << (error_str ? error_str.value() : std::string(""));
  EXPECT_EQ(packed, 0xFFFFFFFFFFFFFDFCULL);

  data.r = 0xbeef;
  data.g = 0xdead;
  data.b = 0xbeef;
  data.a = 0xdead;
  data.coords[0] = 0;
  data.coords[1] = 8191;
  data.coords[2] = 0;
  data.coords[3] = 8191;

  error_str = Pack(data, &packed);
  ASSERT_FALSE(error_str) << (error_str ? error_str.value() : std::string(""));
  EXPECT_EQ(packed,
            base::UInt128(0xdeadbeefdeadbeefULL, 0xFFF8003FFE000DFCULL));
}

// Make sure that the color endpoint mode is properly repacked. This test case
// was created as a bug during testing.
TEST(IntermediateASTCBlockTest, TestPackUnpackWithSameCEM) {
  base::UInt128 orig(0xe8e8eaea20000980ULL, 0x20000200cb73f045ULL);

  auto b = UnpackIntermediateBlock(PhysicalASTCBlock(orig));
  ASSERT_TRUE(b);

  base::UInt128 repacked;
  auto err_str = Pack(b.value(), &repacked);
  ASSERT_FALSE(err_str) << (err_str ? err_str.value() : std::string(""));

  EXPECT_EQ(repacked, orig);

  // Test case #2
  orig = base::UInt128(0x3300c30700cb01c5ULL, 0x0573907b8c0f6879ULL);
  b = UnpackIntermediateBlock(PhysicalASTCBlock(orig));
  ASSERT_TRUE(b);

  err_str = Pack(b.value(), &repacked);
  ASSERT_FALSE(err_str) << (err_str ? err_str.value() : std::string(""));
  EXPECT_EQ(repacked, orig);
}

// Test that we can encode/decode a block that uses a very large gap
// between weight and endpoint data.
TEST(IntermediateASTCBlockTest, TestPackingWithLargeGap) {
  // We can construct this block by doing the following:
  //   -- choose a block mode that only gives 24 weight bits
  //   -- choose the smallest endpoint mode: grayscale direct
  //   -- make sure there are no partitions
  const base::UInt128 orig(0xBEDEAD0000000000ULL, 0x0000000001FE032EULL);
  const auto b = UnpackIntermediateBlock(PhysicalASTCBlock(orig));
  ASSERT_TRUE(b);

  const auto& data = b.value();
  EXPECT_EQ(data.weight_grid_dim_x, 2);
  EXPECT_EQ(data.weight_grid_dim_y, 3);
  EXPECT_EQ(data.weight_range, 15);

  EXPECT_FALSE(data.partition_id);
  EXPECT_FALSE(data.dual_plane_channel);

  ASSERT_EQ(data.endpoints.size(), 1);
  EXPECT_EQ(data.endpoints.at(0).mode, ColorEndpointMode::kLDRLumaDirect);

  ASSERT_EQ(data.endpoints.at(0).colors.size(), 2);
  EXPECT_EQ(data.endpoints.at(0).colors.at(0), 255);
  EXPECT_EQ(data.endpoints.at(0).colors.at(1), 0);

  // Now encode it again
  base::UInt128 repacked;
  const auto err_str = Pack(b.value(), &repacked);
  EXPECT_EQ(orig, repacked) << (err_str ? err_str.value() : std::string(""));
}

// Take a block that is encoded using direct luma with full byte values and see
// if we properly set the endpoint range.
TEST(IntermediateASTCBlockTest, TestEndpointRange) {
  PhysicalASTCBlock blk(0x0000000001FE000173ULL);
  EXPECT_TRUE(blk.ColorValuesRange().hasValue());
  EXPECT_EQ(blk.ColorValuesRange().valueOr(0), 255);

  auto b = UnpackIntermediateBlock(blk);
  ASSERT_TRUE(b);

  const auto& data = b.value();
  ASSERT_THAT(data.endpoints, SizeIs(1));
  EXPECT_THAT(data.endpoints[0].mode, Eq(ColorEndpointMode::kLDRLumaDirect));
  EXPECT_THAT(data.endpoints[0].colors, ElementsAre(0, 255));
  EXPECT_TRUE(data.endpoint_range.hasValue());
  EXPECT_EQ(data.endpoint_range.valueOr(0), 255);
}

struct ImageTestParams {
  std::string image_name;
  int checkered_dim;
};

static void PrintTo(const ImageTestParams& params, std::ostream* os) {
  *os << "ImageTestParams(" << params.image_name << ")";
}

class IntermediateASTCBlockTest : public TestWithParam<ImageTestParams> { };

// Test whether or not a real-world ASTC implementation can be unpacked and
// then repacked into the same implementation. In conjunction with the other
// tests, we make sure that we can recreate ASTC blocks that we have previously
// unpacked.
TEST_P(IntermediateASTCBlockTest, TestPackUnpack) {
  const auto& params = GetParam();
  const int astc_dim = 8;
  const int img_dim = params.checkered_dim * astc_dim;
  const std::string astc = LoadASTCFile(params.image_name);

  // Make sure that unpacking and repacking all of the blocks works...
  const int kNumASTCBlocks = (img_dim / astc_dim) * (img_dim / astc_dim);
  for (int i = 0; i < kNumASTCBlocks; ++i) {
    base::UInt128 block_bits;
    memcpy(&block_bits, astc.data() + PhysicalASTCBlock::kSizeInBytes * i,
           PhysicalASTCBlock::kSizeInBytes);

    const PhysicalASTCBlock block(block_bits);

    base::UInt128 repacked;
    if (block.IsVoidExtent()) {
      auto b = UnpackVoidExtent(block);
      ASSERT_TRUE(b);

      auto err_str = Pack(b.value(), &repacked);
      ASSERT_FALSE(err_str) << (err_str ? err_str.value() : std::string(""));
    } else {
      auto b = UnpackIntermediateBlock(block);
      ASSERT_TRUE(b);

      // Check to see that we properly set the endpoint range when we decoded
      // the block.
      auto& block_data = b.value();
      EXPECT_EQ(block_data.endpoint_range, block.ColorValuesRange());

      // Reset the endpoint range here to see if we correctly reconstruct it
      // below
      block_data.endpoint_range = {};

      auto err_str = Pack(b.value(), &repacked);
      ASSERT_FALSE(err_str) << (err_str ? err_str.value() : std::string(""));
    }

    // You would expect the following line to be enough:
    //   EXPECT_EQ(repacked, block.GetBlockBits())
    // ... except that the ASTC encoder makes some interesting decisions
    // about how to encode the same logical bits. One example is that
    // sometimes if all partitions share an endpoint mode, the encoded
    // block will not use the shared CEM mode, and rather list each
    // partition's mode explicitly. For that reason, we just need to make as
    // close of an approximation as possible that we decode to the same
    // physical values.

    PhysicalASTCBlock pb(repacked);
    ASSERT_FALSE(pb.IsIllegalEncoding());

    base::UInt128 pb_color_mask =
        (base::UInt128(1) << pb.NumColorBits().value()) - 1;
    base::UInt128 pb_color_bits =
        pb.GetBlockBits() >> pb.ColorStartBit().value();
    pb_color_bits &= pb_color_mask;

    base::UInt128 b_color_mask =
        (base::UInt128(1) << pb.NumColorBits().value()) - 1;
    base::UInt128 b_color_bits =
        block.GetBlockBits() >> block.ColorStartBit().value();
    b_color_bits &= b_color_mask;

    EXPECT_EQ(pb_color_mask, b_color_mask);
    EXPECT_EQ(pb_color_bits, b_color_bits);

    EXPECT_EQ(pb.IsVoidExtent(), block.IsVoidExtent());
    EXPECT_EQ(pb.VoidExtentCoords(), block.VoidExtentCoords());

    EXPECT_EQ(pb.WeightGridDims(), block.WeightGridDims());
    EXPECT_EQ(pb.WeightRange(), block.WeightRange());
    EXPECT_EQ(pb.NumWeightBits(), block.NumWeightBits());
    EXPECT_EQ(pb.WeightStartBit(), block.WeightStartBit());

    EXPECT_EQ(pb.IsDualPlane(), block.IsDualPlane());
    EXPECT_EQ(pb.DualPlaneChannel(), block.DualPlaneChannel());

    EXPECT_EQ(pb.NumPartitions(), block.NumPartitions());
    EXPECT_EQ(pb.PartitionID(), block.PartitionID());

    EXPECT_EQ(pb.NumColorValues(), block.NumColorValues());
    EXPECT_EQ(pb.ColorValuesRange(), block.ColorValuesRange());

    for (int j = 0; j < pb.NumPartitions().valueOr(0); ++j) {
      EXPECT_EQ(pb.GetEndpointMode(j), block.GetEndpointMode(j));
    }
  }
}

std::vector<ImageTestParams> GetImageTestParams() {
  return {
    // image_name       checkered_dim
    { "checkered_4",    4  },
    { "checkered_5",    5  },
    { "checkered_6",    6  },
    { "checkered_7",    7  },
    { "checkered_8",    8  },
    { "checkered_9",    9  },
    { "checkered_10",   10 },
    { "checkered_11",   11 },
    { "checkered_12",   12 },
  };
}

INSTANTIATE_TEST_CASE_P(Checkered, IntermediateASTCBlockTest,
                        ValuesIn(GetImageTestParams()));

}  // namespace

}  // namespace astc_codec
