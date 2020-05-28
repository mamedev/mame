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

#include "src/decoder/codec.h"
#include "include/astc-codec/astc-codec.h"
#include "src/decoder/test/image_utils.h"

#include <gtest/gtest.h>

#include <string>

namespace astc_codec {

static void PrintTo(FootprintType footprint, std::ostream* os) {
    switch (footprint) {
        case FootprintType::k4x4:   *os << "FootprintType::k4x4";   break;
        case FootprintType::k5x4:   *os << "FootprintType::k5x4";   break;
        case FootprintType::k5x5:   *os << "FootprintType::k5x5";   break;
        case FootprintType::k6x5:   *os << "FootprintType::k6x5";   break;
        case FootprintType::k6x6:   *os << "FootprintType::k6x6";   break;
        case FootprintType::k8x5:   *os << "FootprintType::k8x5";   break;
        case FootprintType::k8x6:   *os << "FootprintType::k8x6";   break;
        case FootprintType::k10x5:  *os << "FootprintType::k10x5";  break;
        case FootprintType::k10x6:  *os << "FootprintType::k10x6";  break;
        case FootprintType::k8x8:   *os << "FootprintType::k8x8";   break;
        case FootprintType::k10x8:  *os << "FootprintType::k10x8";  break;
        case FootprintType::k10x10: *os << "FootprintType::k10x10"; break;
        case FootprintType::k12x10: *os << "FootprintType::k12x10"; break;
        case FootprintType::k12x12: *os << "FootprintType::k12x12"; break;
        default:
          *os << "<Unexpected FootprintType "
              << static_cast<uint32_t>(footprint) << ">";
    }
}

namespace {

using ::testing::TestWithParam;
using ::testing::ValuesIn;

ImageBuffer LoadGoldenImageWithAlpha(std::string basename) {
  const std::string filename =
      std::string("src/decoder/testdata/") + basename + ".bmp";
  ImageBuffer result;
  LoadGoldenBmp(filename, &result);
  EXPECT_EQ(result.BytesPerPixel(), 4);
  return result;
}

struct ImageTestParams {
  std::string image_name;
  FootprintType footprint;
  size_t width;
  size_t height;
};

static void PrintTo(const ImageTestParams& params, std::ostream* os) {
  *os << "ImageTestParams(" << params.image_name << ", " << params.width << "x"
      << params.height << ", ";
  PrintTo(params.footprint, os);
  *os << ")";
}

TEST(CodecTest, InvalidInput) {
  const size_t valid_width = 16;
  const size_t valid_height = 16;
  const size_t valid_stride = valid_width * 4;

  const std::vector<uint8_t> data(256);
  std::vector<uint8_t> output(valid_width * valid_height * 4);

  // Invalid footprint.
  EXPECT_FALSE(ASTCDecompressToRGBA(
      data.data(), data.size(), valid_width, valid_height,
      FootprintType::kCount, output.data(), output.size(), valid_stride));

  // Fail for 0 width or height.
  EXPECT_FALSE(ASTCDecompressToRGBA(data.data(), data.size(), 0, valid_height,
                                    FootprintType::k4x4, output.data(),
                                    output.size(), valid_stride));
  EXPECT_FALSE(ASTCDecompressToRGBA(data.data(), data.size(), valid_width, 0,
                                    FootprintType::k4x4, output.data(),
                                    output.size(), valid_stride));

  // Fail for data size that's not a multiple of block size.
  EXPECT_FALSE(ASTCDecompressToRGBA(
      data.data(), data.size() - 1, valid_width, valid_height,
      FootprintType::k4x4, output.data(), output.size(), valid_stride));
  // Fail for data size that doesn't match the block count.
  EXPECT_FALSE(ASTCDecompressToRGBA(
      data.data(), data.size() - 16, valid_width, valid_height,
      FootprintType::k4x4, output.data(), output.size(), valid_stride));

  // Fail for invalid stride.
  EXPECT_FALSE(ASTCDecompressToRGBA(
      data.data(), data.size(), valid_width, valid_height, FootprintType::k4x4,
      output.data(), output.size(), valid_stride - 1));

  // Fail for invalid output size.
  EXPECT_FALSE(ASTCDecompressToRGBA(
      data.data(), data.size(), valid_width, valid_height, FootprintType::k4x4,
      output.data(), output.size() - 1, valid_stride));
}

class CodecTest : public TestWithParam<ImageTestParams> {};

TEST_P(CodecTest, PublicAPI) {
  const auto& params = GetParam();
  const std::string astc = LoadASTCFile(params.image_name);

  ImageBuffer our_decoded_image;
  our_decoded_image.Allocate(params.width, params.height, 4);
  ASSERT_TRUE(ASTCDecompressToRGBA(
      reinterpret_cast<const uint8_t*>(astc.data()), astc.size(), params.width,
      params.height, params.footprint, our_decoded_image.Data().data(),
      our_decoded_image.DataSize(), our_decoded_image.Stride()));

  // Check that the decoded image is *very* similar to the library decoding
  // of an ASTC texture. They may not be exact due to differences in how we
  // convert a 16-bit float to an 8-bit integer.
  ImageBuffer decoded_image = LoadGoldenImageWithAlpha(params.image_name);
  CompareSumOfSquaredDifferences(decoded_image, our_decoded_image, 1.0);
}

TEST_P(CodecTest, DecompressToImage) {
  const auto& params = GetParam();

  std::string error;
  std::unique_ptr<ASTCFile> image_file = ASTCFile::LoadFile(
      std::string("src/decoder/testdata/") + params.image_name + ".astc",
      &error);
  ASSERT_TRUE(image_file) << "Failed to load " << params.image_name << ": "
                          << error;

  ASSERT_TRUE(image_file->GetFootprint());
  EXPECT_EQ(params.footprint, image_file->GetFootprint().value().Type());

  ImageBuffer our_decoded_image;
  our_decoded_image.Allocate(image_file->GetWidth(), image_file->GetHeight(),
                             4);

  ASSERT_TRUE(DecompressToImage(*image_file, our_decoded_image.Data().data(),
                                our_decoded_image.DataSize(),
                                our_decoded_image.Stride()));

  // Check that the decoded image is *very* similar to the library decoding
  // of an ASTC texture. They may not be exact due to differences in how we
  // convert a 16-bit float to an 8-bit integer.
  ImageBuffer decoded_image = LoadGoldenImageWithAlpha(params.image_name);
  CompareSumOfSquaredDifferences(decoded_image, our_decoded_image, 1.0);
}

// Test to make sure that reading out color values from blocks in a real-world
// image isn't terribly wrong, either.
std::vector<ImageTestParams> GetTransparentImageTestParams() {
  return {
    // image_name         astc footprint        width    height
    { "atlas_small_4x4",  FootprintType::k4x4,  256,     256 },
    { "atlas_small_5x5",  FootprintType::k5x5,  256,     256 },
    { "atlas_small_6x6",  FootprintType::k6x6,  256,     256 },
    { "atlas_small_8x8",  FootprintType::k8x8,  256,     256 },
  };
}

INSTANTIATE_TEST_CASE_P(Transparent, CodecTest,
                        ValuesIn(GetTransparentImageTestParams()));

}  // namespace

}  // namespace astc_codec
