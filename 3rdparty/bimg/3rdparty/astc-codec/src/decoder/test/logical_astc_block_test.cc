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

#include "src/decoder/logical_astc_block.h"
#include "src/decoder/test/image_utils.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <string>

namespace astc_codec {

namespace {

using ::testing::Eq;
using ::testing::ElementsAre;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

ImageBuffer LoadGoldenImageWithAlpha(std::string basename) {
  const std::string filename = std::string("src/decoder/testdata/") + basename + ".bmp";
  ImageBuffer result;
  LoadGoldenBmp(filename, &result);
  EXPECT_EQ(result.BytesPerPixel(), 4);
  return result;
}

ImageBuffer LoadGoldenImage(std::string basename) {
  const std::string filename = std::string("src/decoder/testdata/") + basename + ".bmp";
  ImageBuffer result;
  LoadGoldenBmp(filename, &result);
  EXPECT_EQ(result.BytesPerPixel(), 3);
  return result;
}

struct ImageTestParams {
  std::string image_name;
  bool has_alpha;
  Footprint footprint;
  int width;
  int height;
};

static void PrintTo(const ImageTestParams& params, std::ostream* os) {
    *os << "ImageTestParams(" << params.image_name << ", "
        << params.width << "x" << params.height << ", "
        << (params.has_alpha ? "RGBA" : "RGB") << ", "
        << "footprint " << params.footprint.Width() << "x"
        << params.footprint.Height() << ")";
}

class LogicalASTCBlockTest : public TestWithParam<ImageTestParams> { };

// Test to make sure that reading out color values from blocks is not
// terribly wrong. To do so, we compress an image and then decompress it
// using our logical blocks and the library. The difference between the
// decoded images should be minimal.
TEST_P(LogicalASTCBlockTest, ImageWithFootprint) {
  const auto& params = GetParam();
  const std::string astc = LoadASTCFile(params.image_name);

  ImageBuffer our_decoded_image;
  our_decoded_image.Allocate(params.width, params.height, params.has_alpha ? 4 : 3);

  const int block_width = params.footprint.Width();
  const int block_height = params.footprint.Height();

  base::UInt128 block;
  for (int i = 0; i < astc.size(); i += 16) {
    const int block_index = i / 16;
    const int blocks_wide =
        (params.width + block_width - 1) / block_width;
    const int block_x = block_index % blocks_wide;
    const int block_y = block_index / blocks_wide;
    memcpy(&block, astc.data() + i, sizeof(block));

    PhysicalASTCBlock physical_block(block);
    if (physical_block.IsVoidExtent()) {
      auto ve = UnpackVoidExtent(physical_block);
      ASSERT_TRUE(ve) << "ASTC encoder produced invalid block!";
    } else {
      auto ib = UnpackIntermediateBlock(physical_block);
      ASSERT_TRUE(ib) << "ASTC encoder produced invalid block!";
    }

    // Make sure that the library doesn't produce incorrect ASTC blocks.
    // This is covered in more depth in other tests in
    // intermediate_astc_block_test and physical_astc_block_test
    auto lb = UnpackLogicalBlock(params.footprint, physical_block);
    ASSERT_TRUE(lb) << "ASTC encoder produced invalid block!";

    LogicalASTCBlock logical_block = lb.value();
    const size_t color_size = params.has_alpha ? 4 : 3;

    for (int y = 0; y < block_height; ++y) {
      for (int x = 0; x < block_width; ++x) {
        const int px = block_width * block_x + x;
        const int py = block_height * block_y + y;

        // Skip out of bounds.
        if (px >= params.width || py >= params.height) {
          continue;
        }

        uint8_t* pixel = our_decoded_image(px, py);
        const RgbaColor decoded_color = logical_block.ColorAt(x, y);
        ASSERT_LE(color_size, decoded_color.size());

        for (int c = 0; c < color_size; ++c) {
          // All of the pixels should also be 8-bit values.
          ASSERT_GE(decoded_color[c], 0);
          ASSERT_LT(decoded_color[c], 256);
          pixel[c] = decoded_color[c];
        }
      }
    }
  }

  // Check that the decoded image is *very* similar to the library decoding
  // of an ASTC texture. They may not be exact due to differences in how we
  // convert a 16-bit float to an 8-bit integer.
  ImageBuffer decoded_image = params.has_alpha ? LoadGoldenImageWithAlpha(params.image_name) : LoadGoldenImage(params.image_name);
  CompareSumOfSquaredDifferences(decoded_image, our_decoded_image, 1.0);
}

// Test to make sure that a simple gradient image can be compressed and decoded
// by our logical block representation. This should work with every footprint.
std::vector<ImageTestParams> GetSyntheticImageTestParams() {
  return {
    // image_name         alpha    astc footprint          width  height
    { "footprint_4x4",    false,   Footprint::Get4x4(),    32,    32 },
    { "footprint_5x4",    false,   Footprint::Get5x4(),    32,    32 },
    { "footprint_5x5",    false,   Footprint::Get5x5(),    32,    32 },
    { "footprint_6x5",    false,   Footprint::Get6x5(),    32,    32 },
    { "footprint_6x6",    false,   Footprint::Get6x6(),    32,    32 },
    { "footprint_8x5",    false,   Footprint::Get8x5(),    32,    32 },
    { "footprint_8x6",    false,   Footprint::Get8x6(),    32,    32 },
    { "footprint_10x5",   false,   Footprint::Get10x5(),   32,    32 },
    { "footprint_10x6",   false,   Footprint::Get10x6(),   32,    32 },
    { "footprint_8x8",    false,   Footprint::Get8x8(),    32,    32 },
    { "footprint_10x8",   false,   Footprint::Get10x8(),   32,    32 },
    { "footprint_10x10",  false,   Footprint::Get10x10(),  32,    32 },
    { "footprint_12x10",  false,   Footprint::Get12x10(),  32,    32 },
    { "footprint_12x12",  false,   Footprint::Get12x12(),  32,    32 },
  };
}

INSTANTIATE_TEST_CASE_P(Synthetic, LogicalASTCBlockTest,
                        ValuesIn(GetSyntheticImageTestParams()));

// Test to make sure that reading out color values from blocks in a real-world
// image isn't terribly wrong, either.
std::vector<ImageTestParams> GetRealWorldImageTestParams() {
  return {
    // image_name   alpha  astc footprint         width   height
    { "rgb_4x4",    false, Footprint::Get4x4(),   224,    288 },
    { "rgb_6x6",    false, Footprint::Get6x6(),   224,    288 },
    { "rgb_8x8",    false, Footprint::Get8x8(),   224,    288 },
    { "rgb_12x12",  false, Footprint::Get12x12(), 224,    288 },
    { "rgb_5x4",    false, Footprint::Get5x4(),   224,    288 }
  };
}

INSTANTIATE_TEST_CASE_P(RealWorld, LogicalASTCBlockTest,
                        ValuesIn(GetRealWorldImageTestParams()));

// Test to make sure that reading out color values from blocks in a real-world
// image isn't terribly wrong, either.
std::vector<ImageTestParams> GetTransparentImageTestParams() {
  return {
    // image_name          alpha  astc footprint       width    height
    { "atlas_small_4x4",   true,  Footprint::Get4x4(), 256,     256 },
    { "atlas_small_5x5",   true,  Footprint::Get5x5(), 256,     256 },
    { "atlas_small_6x6",   true,  Footprint::Get6x6(), 256,     256 },
    { "atlas_small_8x8",   true,  Footprint::Get8x8(), 256,     256 },
  };
}

INSTANTIATE_TEST_CASE_P(Transparent, LogicalASTCBlockTest,
                        ValuesIn(GetTransparentImageTestParams()));

// Test to make sure that if we set our endpoints then it's reflected in our
// color selection
TEST(LogicalASTCBlockTest, SetEndpoints) {
  LogicalASTCBlock logical_block(Footprint::Get8x8());

  // Setup a weight checkerboard
  for (int j = 0; j < 8; ++j) {
    for (int i = 0; i < 8; ++i) {
      if (((i ^ j) & 1) == 1) {
        logical_block.SetWeightAt(i, j, 0);
      } else {
        logical_block.SetWeightAt(i, j, 64);
      }
    }
  }

  // Now set the colors to something ridiculous
  logical_block.SetEndpoints({{ 123, 45, 67, 89 }}, {{ 101, 121, 31, 41 }}, 0);

  // For each pixel, we expect it to mirror the endpoints in a checkerboard
  // pattern
  for (int j = 0; j < 8; ++j) {
    for (int i = 0; i < 8; ++i) {
      if (((i ^ j) & 1) == 1) {
        EXPECT_THAT(logical_block.ColorAt(i, j), ElementsAre(123, 45, 67, 89));
      } else {
        EXPECT_THAT(logical_block.ColorAt(i, j), ElementsAre(101, 121, 31, 41));
      }
    }
  }
}

// Test whether or not setting weight values under different circumstances is
// supported and reflected in the query functions.
TEST(LogicalASTCBlockTest, SetWeightVals) {
  LogicalASTCBlock logical_block(Footprint::Get4x4());

  EXPECT_THAT(logical_block.GetFootprint(), Eq(Footprint::Get4x4()));

  // Not a dual plane by default
  EXPECT_FALSE(logical_block.IsDualPlane());
  logical_block.SetWeightAt(2, 3, 2);

  // Set the dual plane
  logical_block.SetDualPlaneChannel(0);
  EXPECT_TRUE(logical_block.IsDualPlane());

  // This shouldn't have reset our weight
  const LogicalASTCBlock other_block = logical_block;
  EXPECT_THAT(other_block.WeightAt(2, 3), Eq(2));
  EXPECT_THAT(other_block.DualPlaneWeightAt(0, 2, 3), Eq(2));

  // If we set the dual plane weight, it shouldn't change the original weight
  // value or the other channels
  logical_block.SetDualPlaneWeightAt(0, 2, 3, 1);
  EXPECT_THAT(logical_block.WeightAt(2, 3), Eq(2));
  EXPECT_THAT(logical_block.DualPlaneWeightAt(0, 2, 3), Eq(1));
  for (int i = 1; i < 4; ++i) {
    EXPECT_THAT(logical_block.DualPlaneWeightAt(i, 2, 3), Eq(2));
  }

  // Remove the dual plane
  logical_block.SetDualPlaneChannel(-1);
  EXPECT_FALSE(logical_block.IsDualPlane());

  // Now the original dual plane weight should be reset back to the others. Note
  // that we have to call DualPlaneWeightAt from a const logical block since
  // returning a reference to a weight that doesn't exist is illegal.
  const LogicalASTCBlock other_block2 = logical_block;
  EXPECT_THAT(logical_block.WeightAt(2, 3), Eq(2));
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(logical_block.WeightAt(2, 3),
              other_block2.DualPlaneWeightAt(i, 2, 3));
  }
}

}  // namespace

}  // namespace astc_codec
