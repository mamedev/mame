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

#include "src/decoder/endpoint_codec.h"
#include "src/decoder/intermediate_astc_block.h"
#include "src/decoder/test/image_utils.h"

#include <random>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace astc_codec {

namespace {

using ::testing::AllOf;
using ::testing::AnyOf;
using ::testing::Each;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Le;
using ::testing::Ne;
using ::testing::Pointwise;
using ::testing::SizeIs;
using ::testing::Pair;

constexpr std::array<EndpointEncodingMode, 6> kEndpointEncodingModes = {{
    EndpointEncodingMode::kDirectLuma,
    EndpointEncodingMode::kDirectLumaAlpha,
    EndpointEncodingMode::kBaseScaleRGB,
    EndpointEncodingMode::kBaseScaleRGBA,
    EndpointEncodingMode::kDirectRGB,
    EndpointEncodingMode::kDirectRGBA }};

const std::array<std::pair<RgbaColor, RgbaColor>, 3> kBlueContractPairs = {{
    std::make_pair(RgbaColor{{ 22, 18, 30, 59 }},
                   RgbaColor{{ 162, 148, 155, 59 }}),
    std::make_pair(RgbaColor{{ 22, 30, 27, 36 }},
                   RgbaColor{{ 228, 221, 207, 36 }}),
    std::make_pair(RgbaColor{{ 54, 60, 55, 255 }},
                   RgbaColor{{ 23, 30, 27, 255 }})
  }};

// Used to directly initialize std::pairs of colors with initializer lists
//   e.g. MakeColors({{ r, g, b, a }}, {{ r, g, b, a }});
std::pair<RgbaColor, RgbaColor> MakeColors(RgbaColor&& a, RgbaColor&& b) {
  return std::make_pair(a, b);
}

// Returns |high| and |low| as they would be decoded using the quantization
// factor |quant| for the ColorEndpointMode |mode|.
std::pair<RgbaColor, RgbaColor> TestColors(
    RgbaColor low, RgbaColor high, int quant, EndpointEncodingMode mode) {
  ColorEndpointMode astc_mode;
  std::vector<int> encoded;
  const bool needs_swap =
      EncodeColorsForMode(low, high, quant, mode, &astc_mode, &encoded);

  RgbaColor decoded_low, decoded_high;
  DecodeColorsForMode(encoded, quant, astc_mode, &decoded_low, &decoded_high);

  if (needs_swap) {
    return std::make_pair(decoded_high, decoded_low);
  } else {
    return std::make_pair(decoded_low, decoded_high);
  }
}

// Returns true if the argument tuple entries only differ by at most x.
MATCHER_P(IsCloseTo, x, "") {
  const auto& a = ::testing::get<0>(arg);
  const auto& b = ::testing::get<1>(arg);
  return (a > b) ? ((a - b) <= x) : ((b - a) <= x);
}

// Test to make sure that the range of values that we get as they are
// quantized remains within what we pass as |quant|.
TEST(EndpointCodecTest, QuantRanges) {
  const RgbaColor low {{ 0, 0, 0, 0 }};
  const RgbaColor high {{ 255, 255, 255, 255 }};

  std::vector<int> result;
  for (const auto& mode : kEndpointEncodingModes) {
    for (int i = 5; i < 256; ++i) {
      ColorEndpointMode astc_mode;
      const bool needs_swap =
          EncodeColorsForMode(low, high, i, mode, &astc_mode, &result);
      EXPECT_EQ(result.size(), NumValuesForEncodingMode(mode)) << i;
      EXPECT_EQ(result.size(), NumColorValuesForEndpointMode(astc_mode)) << i;

      // ASTC mode shouldn't use base/offset when endpoints are so far apart.
      EXPECT_THAT(astc_mode, Ne(ColorEndpointMode::kLDRRGBBaseOffset));
      EXPECT_THAT(astc_mode, Ne(ColorEndpointMode::kLDRRGBABaseOffset));

      EXPECT_THAT(result, Each(AllOf(Ge(0), Le(i))))
          << "Mode: " << static_cast<int>(mode);
      // We don't care if we need to swap the weights in this test
      EXPECT_TRUE(needs_swap || !needs_swap);
    }
  }
}

// Test to make sure that each mode that directly encodes colors can effectively
// encode both black and white
TEST(EndpointCodecTest, ExtremeDirectEncodings) {
  const RgbaColor kWhite {{ 255, 255, 255, 255 }};
  const RgbaColor kBlack {{ 0, 0, 0, 255 }};

  std::vector<int> encoded;
  for (const auto& mode : kEndpointEncodingModes) {
    for (int i = 5; i < 256; ++i) {
      const auto expected = std::make_pair(kWhite, kBlack);
      EXPECT_EQ(TestColors(kWhite, kBlack, i, mode), expected)
          << "Range: " << i << ", Mode: " << static_cast<int>(mode);
    }
  }
}

// According to the spec, this is used for colors close to gray. The values
// chosen here were according to the spec.
TEST(EndpointCodecTest, UsesBlueContract) {
  std::vector<int> vals = { 132, 127, 116, 112, 183, 180, 31, 22 };
  EXPECT_TRUE(UsesBlueContract(255, ColorEndpointMode::kLDRRGBDirect, vals));
  EXPECT_TRUE(UsesBlueContract(255, ColorEndpointMode::kLDRRGBADirect, vals));

  // For the offset modes the only way to trigger the blue contract mode is if
  // we force the subtraction in the decoding procedure (See section C.2.14 of
  // the spec), so we need to set the 7th bit to 1 for all of the odd-numbered
  // values
  vals[1] &= 0xBF;
  vals[3] &= 0xBF;
  vals[5] &= 0xBF;
  vals[7] &= 0xBF;

  EXPECT_FALSE(
      UsesBlueContract(255, ColorEndpointMode::kLDRRGBBaseOffset, vals));
  EXPECT_FALSE(
      UsesBlueContract(255, ColorEndpointMode::kLDRRGBABaseOffset, vals));

  vals[1] |= 0x40;
  vals[3] |= 0x40;
  vals[5] |= 0x40;
  vals[7] |= 0x40;

  EXPECT_TRUE(
      UsesBlueContract(255, ColorEndpointMode::kLDRRGBBaseOffset, vals));
  EXPECT_TRUE(
      UsesBlueContract(255, ColorEndpointMode::kLDRRGBABaseOffset, vals));

  // All other LDR endpoint modes should return no blue contract
  for (int max_val : { 255, 127, 11 }) {
    for (auto mode : { ColorEndpointMode::kLDRLumaDirect,
            ColorEndpointMode::kLDRLumaBaseOffset,
            ColorEndpointMode::kLDRLumaAlphaDirect,
            ColorEndpointMode::kLDRLumaAlphaBaseOffset,
            ColorEndpointMode::kLDRRGBBaseScale,
            ColorEndpointMode::kLDRRGBBaseScaleTwoA }) {
      EXPECT_FALSE(UsesBlueContract(max_val, mode, vals));
    }
  }
}

// Make sure that encoding and decoding for the direct luminance mode works.
TEST(EndpointCodecTest, LumaDirect) {
  const auto mode = EndpointEncodingMode::kDirectLuma;

  // With a 255 quantizer, all greys should be exact.
  for (int i = 0; i < 255; ++i) {
    for (int j = 0; j < 255; ++j) {
      EXPECT_EQ(TestColors({{ i, i, i, 255 }}, {{ j, j, j, 255 }}, 255, mode),
                MakeColors({{ i, i, i, 255 }}, {{ j, j, j, 255 }}));
    }
  }

  // If we have almost grey, then they should encode to grey.
  EXPECT_EQ(TestColors({{ 247, 248, 246, 255 }}, {{ 2, 3, 1, 255 }}, 255, mode),
            MakeColors({{ 247, 247, 247, 255 }}, {{ 2, 2, 2, 255 }}));

  EXPECT_EQ(TestColors({{ 80, 80, 50, 255 }}, {{ 99, 255, 6, 255 }}, 255, mode),
            MakeColors({{ 70, 70, 70, 255 }}, {{ 120, 120, 120, 255 }}));

  // If we have almost greys and a really small quantizer, it should be white
  // and black (literally).
  EXPECT_EQ(TestColors({{ 247, 248, 246, 255 }}, {{ 2, 3, 1, 255 }}, 15, mode),
            MakeColors({{ 255, 255, 255, 255 }}, {{ 0, 0, 0, 255 }}));

  // The average of 64, 127, and 192 is 127.666..., so it should round to
  // 130 instead of 125.
  EXPECT_EQ(TestColors({{ 64, 127, 192, 255 }}, {{ 0, 0, 0, 255 }}, 63, mode),
            MakeColors({{ 130, 130, 130, 255 }}, {{ 0, 0, 0, 255 }}));

  // If we have almost grey, then they should encode to grey -- similar to
  // direct encoding since the encoded colors differ by < 63.
  EXPECT_EQ(TestColors({{ 80, 80, 50, 255 }}, {{ 99, 255, 6, 255 }}, 255, mode),
            MakeColors({{ 70, 70, 70, 255 }}, {{ 120, 120, 120, 255 }}));

  // Low precision colors should still encode pretty well with base/offset.
  EXPECT_EQ(TestColors({{ 35, 36, 38, 255 }}, {{ 42, 43, 40, 255 }}, 47, mode),
            MakeColors({{ 38, 38, 38, 255 }}, {{ 43, 43, 43, 255 }}));

  EXPECT_EQ(TestColors({{ 39, 42, 40, 255 }}, {{ 18, 20, 21, 255 }}, 39, mode),
            MakeColors({{ 39, 39, 39, 255 }}, {{ 19, 19, 19, 255 }}));
}

// Test encoding and decoding for the base-offset luminance mode.
TEST(EndpointCodecTest, LumaAlphaDirect) {
  const auto mode = EndpointEncodingMode::kDirectLumaAlpha;

  // With a 255 quantizer, all greys should be exact.
  for (int i = 0; i < 255; ++i) {
    for (int j = 0; j < 255; ++j) {
      EXPECT_EQ(TestColors({{ i, i, i, j }}, {{ j, j, j, i }}, 255, mode),
                MakeColors({{ i, i, i, j }}, {{ j, j, j, i }}));
    }
  }

  // If we have almost grey, then they should encode to grey.
  EXPECT_EQ(TestColors({{ 247, 248, 246, 250 }}, {{ 2, 3, 1, 172 }}, 255, mode),
            MakeColors({{ 247, 247, 247, 250 }}, {{ 2, 2, 2, 172 }}));

  EXPECT_EQ(TestColors({{ 80, 80, 50, 0 }}, {{ 99, 255, 6, 255 }}, 255, mode),
            MakeColors({{ 70, 70, 70, 0 }}, {{ 120, 120, 120, 255 }}));

  // If we have almost greys and a really small quantizer, it should be white
  // and black (literally).
  EXPECT_EQ(TestColors({{ 247, 248, 246, 253 }}, {{ 2, 3, 1, 3 }}, 15, mode),
            MakeColors({{ 255, 255, 255, 255 }}, {{ 0, 0, 0, 0 }}));

  // The average of 64, 127, and 192 is 127.666..., so it should round to
  // 130 instead of 125. The alpha in this case is independent.
  EXPECT_EQ(TestColors({{ 64, 127, 192, 127 }}, {{ 0, 0, 0, 20 }}, 63, mode),
            MakeColors({{ 130, 130, 130, 125 }}, {{ 0, 0, 0, 20 }}));
}

// Test encoding for the direct RGB mode.
TEST(EndpointCodecTest, RGBDirect) {
  const auto mode = EndpointEncodingMode::kDirectRGB;

  // Colors should be encoded exactly with a 255 quantizer.
  std::mt19937 random(0xdeadbeef);
  std::uniform_int_distribution<int> byte_distribution(0, 255);

  for (int i = 0; i < 100; ++i) {
    RgbaColor low, high;
    for (auto& x : high) { x = byte_distribution(random); }
    for (auto& x : low) { x = byte_distribution(random); }
    high[3] = low[3] = 255;  // RGB Direct mode has opaque alpha.

    EXPECT_EQ(TestColors(low, high, 255, mode), std::make_pair(low, high))
        << "Random iter: " << i;
  }

  // For each of the following tests, order of endpoints shouldn't have any
  // bearing on the quantization properties, so we should be able to switch
  // endpoints as we see fit and have them generate the same flipped encoded
  // pairs.

  EXPECT_EQ(TestColors({{ 64, 127, 192, 255 }}, {{ 0, 0, 0, 255 }}, 63, mode),
            MakeColors({{ 65, 125, 190, 255 }}, {{ 0, 0, 0, 255 }}));

  EXPECT_EQ(TestColors({{ 0, 0, 0, 255 }}, {{ 64, 127, 192, 255 }}, 63, mode),
            MakeColors({{ 0, 0, 0, 255 }}, {{ 65, 125, 190, 255 }}));

  EXPECT_EQ(TestColors({{ 1, 2, 94, 255 }}, {{ 168, 255, 13, 255 }}, 7, mode),
            MakeColors({{ 0, 0, 109, 255 }}, {{ 182, 255, 0, 255 }}));

  // Colors close to grey will likely need a blue contract.
  EXPECT_EQ(TestColors(kBlueContractPairs[0].first,
                       kBlueContractPairs[0].second, 31, mode),
            MakeColors({{ 24, 20, 33, 255 }}, {{ 160, 148, 156, 255 }}));

  EXPECT_EQ(TestColors(kBlueContractPairs[0].second,
                       kBlueContractPairs[0].first, 31, mode),
            MakeColors({{ 160, 148, 156, 255 }}, {{ 24, 20, 33, 255 }}));

  EXPECT_EQ(TestColors(kBlueContractPairs[1].first,
                       kBlueContractPairs[1].second, 7, mode),
            MakeColors({{ 18, 36, 36, 255 }}, {{ 237, 219, 219, 255 }}));

  EXPECT_EQ(TestColors(kBlueContractPairs[1].second,
                       kBlueContractPairs[1].first, 7, mode),
            MakeColors({{ 237, 219, 219, 255 }}, {{ 18, 36, 36, 255 }}));

  // Colors close to grey (and each other) will likely need a blue contract AND
  // use the offset mode for encoding
  EXPECT_EQ(TestColors(kBlueContractPairs[2].first,
                       kBlueContractPairs[2].second, 31, mode),
            MakeColors({{ 53, 59, 53, 255 }}, {{ 24, 30, 26, 255 }}));

  EXPECT_EQ(TestColors(kBlueContractPairs[2].second,
                       kBlueContractPairs[2].first, 31, mode),
            MakeColors({{ 24, 30, 26, 255 }}, {{ 53, 59, 53, 255 }}));

  // Colors close to each other, but not to grey will likely only use the offset
  // mode and not the blue-contract modes.
  EXPECT_EQ(TestColors({{ 22, 148, 30, 59 }}, {{ 162, 18, 155, 59 }}, 31, mode),
            MakeColors({{ 24, 148, 33, 255 }}, {{ 165, 16, 156, 255 }}));

  EXPECT_EQ(TestColors({{ 162, 18, 155, 59 }}, {{ 22, 148, 30, 59 }}, 31, mode),
            MakeColors({{ 165, 16, 156, 255 }}, {{ 24, 148, 33, 255 }}));
}

// Make sure that certain endpoint pairs result in the blue-contract path as
// we'd expect, such that we can make sure that we're hitting all of the encode
// paths.
TEST(EndpointCodecTest, RGBDirectMakesBlueContract) {
  constexpr int kEndpointRange = 31;
  for (const auto& endpoint_pair : kBlueContractPairs) {
    ColorEndpointMode astc_mode;
    std::vector<int> vals;
    bool needs_swap = EncodeColorsForMode(
        endpoint_pair.first, endpoint_pair.second,
        kEndpointRange, EndpointEncodingMode::kDirectRGB, &astc_mode, &vals);
    (void)(needs_swap);  // Don't really care.

    EXPECT_TRUE(UsesBlueContract(kEndpointRange, astc_mode, vals));
  }
}

// Make sure that encoding and decoding for the RGB base-scale mode works.
TEST(EndpointCodecTest, RGBBaseScale) {
  const auto mode = EndpointEncodingMode::kBaseScaleRGB;
  const auto close_to = [](RgbaColor c, int x) {
    return Pointwise(IsCloseTo(x), c);
  };

  // Identical colors should be encoded with a 255 scale factor. Since ASTC
  // decodes the scaled color by doing (x * s) >> 8, the decoded color will be
  // multiplied by 255/256. This might cause rounding errors sometimes, so we
  // check that every channel only deviates by 1.
  std::mt19937 random(0xdeadbeef);
  std::uniform_int_distribution<int> byte_distribution(0, 255);

  for (int i = 0; i < 100; ++i) {
    RgbaColor color{{byte_distribution(random), byte_distribution(random),
                     byte_distribution(random), 255}};
    const auto test_result = TestColors(color, color, 255, mode);
    EXPECT_THAT(test_result, Pair(close_to(color, 1), close_to(color, 1)));
  }

  // Make sure that if we want to scale by e.g. 1/4 then we can do that exactly:
  const RgbaColor low = {{ 20, 4, 40, 255 }};
  const RgbaColor high = {{ 80, 16, 160, 255 }};
  EXPECT_THAT(TestColors(low, high, 255, mode),
              Pair(close_to(low, 0), close_to(high, 0)));

  // And if we quantize it, then we get roughly the same thing. The scale factor
  // should be representable with most quantization levels. The problem is that
  // if we're off on the 'high' color, then we will be off on the 'low' color.
  EXPECT_THAT(TestColors(low, high, 127, mode),
              Pair(close_to(low, 1), close_to(high, 1)));

  EXPECT_THAT(TestColors(low, high, 63, mode),
              Pair(close_to(low, 1), close_to(high, 2)));

  EXPECT_THAT(TestColors(low, high, 31, mode),
              Pair(close_to(low, 1), close_to(high, 4)));

  EXPECT_THAT(TestColors(low, high, 15, mode),
              Pair(close_to(low, 2), close_to(high, 8)));
}

// Make sure that encoding and decoding for the RGB base-offset mode works.
// Since we don't have a decoder, this is currently only a test that should work
// based on reasoning about what's written in the spec.
// TODO(krajcevski): Write an encoder.
TEST(EndpointCodecTest, RGBBaseOffset) {
  const auto test_colors = [](const RgbaColor& low, const RgbaColor& high) {
    const RgbaColor diff = {{ high[0] - low[0], high[1] - low[1],
                              high[2] - low[2], high[3] - low[3] }};

    std::vector<int> vals;
    for (int i = 0; i < 3; ++i) {
      // If the base is "large", then it grabs it's most significant bit from
      // the offset value. Hence, we need to save it here.
      const bool is_large = low[i] >= 128;
      vals.push_back((low[i] * 2) & 0xFF);
      vals.push_back(diff[i] * 2);

      // Give the "large" bases their bits back.
      if (is_large) {
        vals.back() |= 0x80;
      }
    }

    RgbaColor dec_low, dec_high;
    DecodeColorsForMode(vals, 255, ColorEndpointMode::kLDRRGBBaseOffset,
                        &dec_low, &dec_high);

    EXPECT_THAT(std::make_pair(dec_low, dec_high), Pair(Eq(low), Eq(high)));
  };

  // Test the "direct encoding" path.
  test_colors({{ 80, 16, 112, 255 }}, {{ 87, 18, 132, 255 }});
  test_colors({{ 80, 74, 82, 255 }}, {{ 90, 92, 110, 255 }});
  test_colors({{ 0, 0, 0, 255 }}, {{ 2, 2, 2, 255 }});

  // Identical endpoints should always encode exactly, provided they satisfy the
  // requirements for the base encoding.
  std::mt19937 random(0xdeadbeef);
  std::uniform_int_distribution<int> byte_distribution(0, 255);
  for (int i = 0; i < 100; ++i) {
    RgbaColor color{{byte_distribution(random), byte_distribution(random),
                     byte_distribution(random), 255}};
    if ((color[0] | color[1] | color[2]) & 1) {
      continue;
    }
    test_colors(color, color);
  }

  // TODO(google): Test the "blue contract" path.
}

// Make sure that we can decode colors that are given to us straight out of the
// ASTC codec.
TEST(EndpointCodecTest, DecodeCheckerboard) {
  const RgbaColor kWhite {{ 255, 255, 255, 255 }};
  const RgbaColor kBlack {{ 0, 0, 0, 255 }};

  const std::string astc = LoadASTCFile("checkerboard");
  for (int i = 0; i < astc.size(); i += 16) {
    base::UInt128 block;
    memcpy(&block, &astc[i], sizeof(block));

    const auto intermediate = UnpackIntermediateBlock(PhysicalASTCBlock(block));
    ASSERT_TRUE(intermediate) << "Block is void extent???";

    const auto block_data = &intermediate.value();
    ASSERT_THAT(block_data->endpoints, SizeIs(Eq(1)));

    const int color_range = EndpointRangeForBlock(*block_data);
    const auto& endpoints = block_data->endpoints[0];

    RgbaColor low, high;
    DecodeColorsForMode(endpoints.colors, color_range, endpoints.mode,
                        &low, &high);

    // Expect that the endpoints are black and white, but either order.
    EXPECT_THAT(std::make_pair(low, high),
                AnyOf(
                    Pair(Eq(kWhite), Eq(kBlack)),
                    Pair(Eq(kBlack), Eq(kWhite))));
  }
}

}  // namespace

}  // namespace astc_codec
