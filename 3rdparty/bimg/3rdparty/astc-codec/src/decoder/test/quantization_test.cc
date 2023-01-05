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

#include "src/decoder/quantization.h"
#include "src/decoder/integer_sequence_codec.h"

#include <gtest/gtest.h>

#include <functional>
#include <string>
#include <vector>

namespace astc_codec {

namespace {

// Make sure that we never exceed the maximum range that we pass in.
TEST(QuantizationTest, TestQuantizeMaxRange) {
  for (int i = kEndpointRangeMinValue; i < 256; ++i) {
    EXPECT_LE(QuantizeCEValueToRange(255, i), i);
  }

  for (int i = 1; i < kWeightRangeMaxValue; ++i) {
    EXPECT_LE(QuantizeWeightToRange(64, i), i);
  }
}

// Make sure that whenever we unquantize and requantize a value we get back
// what we started with.
TEST(QuantizationTest, TestReversibility) {
  for (auto itr = ISERangeBegin(); itr != ISERangeEnd(); itr++) {
    const int range = *itr;
    if (range <= kWeightRangeMaxValue) {
      for (int j = 0; j <= range; ++j) {
        const int q = UnquantizeWeightFromRange(j, range);
        EXPECT_EQ(QuantizeWeightToRange(q, range), j);
      }
    }

    if (range >= kEndpointRangeMinValue) {
      for (int j = 0; j <= range; ++j) {
        const int q = UnquantizeCEValueFromRange(j, range);
        EXPECT_EQ(QuantizeCEValueToRange(q, range), j);
      }
    }
  }
}

// Make sure that whenever we quantize a non-maximal value it gets sent to the
// proper range
TEST(QuantizationTest, TestQuantizationRange) {
  for (auto itr = ISERangeBegin(); itr != ISERangeEnd(); itr++) {
    const int range = *itr;
    if (range >= kEndpointRangeMinValue) {
      EXPECT_LE(QuantizeCEValueToRange(0, range), range);
      EXPECT_LE(QuantizeCEValueToRange(4, range), range);
      EXPECT_LE(QuantizeCEValueToRange(15, range), range);
      EXPECT_LE(QuantizeCEValueToRange(22, range), range);
      EXPECT_LE(QuantizeCEValueToRange(66, range), range);
      EXPECT_LE(QuantizeCEValueToRange(91, range), range);
      EXPECT_LE(QuantizeCEValueToRange(126, range), range);
    }

    if (range <= kWeightRangeMaxValue) {
      EXPECT_LE(QuantizeWeightToRange(0, range), range);
      EXPECT_LE(QuantizeWeightToRange(4, range), range);
      EXPECT_LE(QuantizeWeightToRange(15, range), range);
      EXPECT_LE(QuantizeWeightToRange(22, range), range);
    }
  }
}

// Make sure that whenever we unquantize a value it remains within [0, 255]
TEST(QuantizationTest, TestUnquantizationRange) {
  EXPECT_LT(UnquantizeCEValueFromRange(2, 7), 256);
  EXPECT_LT(UnquantizeCEValueFromRange(7, 7), 256);
  EXPECT_LT(UnquantizeCEValueFromRange(39, 63), 256);
  EXPECT_LT(UnquantizeCEValueFromRange(66, 79), 256);
  EXPECT_LT(UnquantizeCEValueFromRange(91, 191), 256);
  EXPECT_LT(UnquantizeCEValueFromRange(126, 255), 256);
  EXPECT_LT(UnquantizeCEValueFromRange(255, 255), 256);

  EXPECT_LE(UnquantizeWeightFromRange(0, 1), 64);
  EXPECT_LE(UnquantizeWeightFromRange(2, 7), 64);
  EXPECT_LE(UnquantizeWeightFromRange(7, 7), 64);
  EXPECT_LE(UnquantizeWeightFromRange(29, 31), 64);
}

// When we quantize a value, it should use the largest quantization range that
// does not exceed the desired range.
TEST(QuantizationTest, TestUpperBoundRanges) {
  auto expected_range_itr = ISERangeBegin();
  for (int desired_range = 1; desired_range < 256; ++desired_range) {
    if (desired_range == *(expected_range_itr + 1)) {
      ++expected_range_itr;
    }
    const int expected_range = *expected_range_itr;
    ASSERT_LE(expected_range, desired_range);

    if (desired_range >= kEndpointRangeMinValue) {
      EXPECT_EQ(QuantizeCEValueToRange(0, desired_range),
                QuantizeCEValueToRange(0, expected_range));

      EXPECT_EQ(QuantizeCEValueToRange(208, desired_range),
                QuantizeCEValueToRange(208, expected_range));

      EXPECT_EQ(QuantizeCEValueToRange(173, desired_range),
                QuantizeCEValueToRange(173, expected_range));

      EXPECT_EQ(QuantizeCEValueToRange(13, desired_range),
                QuantizeCEValueToRange(13, expected_range));

      EXPECT_EQ(QuantizeCEValueToRange(255, desired_range),
                QuantizeCEValueToRange(255, expected_range));
    }

    if (desired_range <= kWeightRangeMaxValue) {
      EXPECT_EQ(QuantizeWeightToRange(0, desired_range),
                QuantizeWeightToRange(0, expected_range));

      EXPECT_EQ(QuantizeWeightToRange(63, desired_range),
                QuantizeWeightToRange(63, expected_range));

      EXPECT_EQ(QuantizeWeightToRange(12, desired_range),
                QuantizeWeightToRange(12, expected_range));

      EXPECT_EQ(QuantizeWeightToRange(23, desired_range),
                QuantizeWeightToRange(23, expected_range));
    }
  }

  // Make sure that we covered all the possible ranges
  ASSERT_EQ(std::next(expected_range_itr), ISERangeEnd());
}

// Make sure that quantizing to the largest range is the identity function.
TEST(QuantizationTest, TestIdentity) {
  for (int i = 0; i < 256; ++i) {
    EXPECT_EQ(QuantizeCEValueToRange(i, 255), i);
  }

  // Note: This doesn't apply to weights since there's a weird hack to convert
  // values from [0, 31] to [0, 64].
}

// Make sure that bit quantization is monotonic with respect to the input,
// since quantizing and dequantizing bits is a matter of truncation and bit
// replication
TEST(QuantizationTest, TestMonotonicBitPacking) {
  for (int num_bits = 3; num_bits < 8; ++num_bits) {
    const int range = (1 << num_bits) - 1;
    int last_quant_val = -1;
    for (int i = 0; i < 256; ++i) {
      const int quant_val = QuantizeCEValueToRange(i, range);
      EXPECT_LE(last_quant_val, quant_val);
      last_quant_val = quant_val;
    }

    // Also expect the last quantization val to be equal to the range
    EXPECT_EQ(last_quant_val, range);

    if (range <= kWeightRangeMaxValue) {
      last_quant_val = -1;
      for (int i = 0; i <= 64; ++i) {
        const int quant_val = QuantizeWeightToRange(i, range);
        EXPECT_LE(last_quant_val, quant_val);
        last_quant_val = quant_val;
      }
      EXPECT_EQ(last_quant_val, range);
    }
  }
}

// Make sure that bit quantization reflects that quantized values below the bit
// replication threshold get mapped to zero
TEST(QuantizationTest, TestSmallBitPacking) {
  for (int num_bits = 1; num_bits <= 8; ++num_bits) {
    const int range = (1 << num_bits) - 1;

    // The largest number that should map to zero is one less than half of the
    // smallest representation w.r.t. range. For example: if we have a range
    // of 7, it means that we have 3 total bits abc for quantized values. If we
    // unquantize to 8 bits, it means that our resulting value will be abcabcab.
    // Hence, we map 000 to 0 and 001 to 0b00100100 = 36. The earliest value
    // that should not map to zero with three bits is therefore 0b00001111 = 15.
    // This ends up being (1 << (8 - 3 - 1)) - 1. We don't use 0b00011111 = 31
    // because this would "round up" to 1 during quantization. This value is not
    // necessarily the largest, but it is the largest that we can *guarantee*
    // should map to zero.

    if (range >= kEndpointRangeMinValue) {
      constexpr int cev_bits = 8;
      const int half_max_quant_bits = std::max(0, cev_bits - num_bits - 1);
      const int largest_cev_to_zero = (1 << half_max_quant_bits) - 1;
      EXPECT_EQ(QuantizeCEValueToRange(largest_cev_to_zero, range), 0)
          << " Largest CEV to zero: " << largest_cev_to_zero
          << " Range: " << range;
    }

    if (range <= kWeightRangeMaxValue) {
      constexpr int weight_bits = 6;
      const int half_max_quant_bits = std::max(0, weight_bits - num_bits - 1);
      const int largest_weight_to_zero = (1 << half_max_quant_bits) - 1;
      EXPECT_EQ(QuantizeWeightToRange(largest_weight_to_zero, range), 0)
          << " Largest weight to zero: " << largest_weight_to_zero
          << " Range: " << range;
    }
  }
}

// Test specific quint and trit weight encodings with values that were obtained
// using the reference ASTC codec.
TEST(QuantizationTest, TestSpecificQuintTritPackings) {
  std::vector<int> vals = { 4, 6, 4, 6, 7, 5, 7, 5 };
  std::vector<int> quantized;

  // Test a quint packing
  std::transform(
      vals.begin(), vals.end(), std::back_inserter(quantized),
      std::bind(UnquantizeWeightFromRange, std::placeholders::_1, 9));
  const std::vector<int> quintExpected = {14, 21, 14, 21, 43, 50, 43, 50 };
  EXPECT_EQ(quantized, quintExpected);

  // Test a trit packing
  std::transform(
      vals.begin(), vals.end(), quantized.begin(),
      std::bind(UnquantizeWeightFromRange, std::placeholders::_1, 11));
  const std::vector<int> tritExpected = { 5, 23, 5, 23, 41, 59, 41, 59 };
  EXPECT_EQ(quantized, tritExpected);
}

// Make sure that we properly die when we pass in values below the minimum
// allowed ranges for our quantization intervals.
TEST(QuantizationDeathTest, TestInvalidMinRange) {
  for (int i = 0; i < kEndpointRangeMinValue; ++i) {
    EXPECT_DEBUG_DEATH(QuantizeCEValueToRange(0, i), "");
    EXPECT_DEBUG_DEATH(UnquantizeCEValueFromRange(0, i), "");
  }

  EXPECT_DEBUG_DEATH(QuantizeWeightToRange(0, 0), "");
  EXPECT_DEBUG_DEATH(UnquantizeWeightFromRange(0, 0), "");
}

// Make sure that we properly die when we pass in bogus values.
TEST(QuantizationDeathTest, TestOutOfRange) {
  EXPECT_DEBUG_DEATH(QuantizeCEValueToRange(-1, 10), "");
  EXPECT_DEBUG_DEATH(QuantizeCEValueToRange(256, 7), "");
  EXPECT_DEBUG_DEATH(QuantizeCEValueToRange(10000, 17), "");

  EXPECT_DEBUG_DEATH(UnquantizeCEValueFromRange(-1, 10), "");
  EXPECT_DEBUG_DEATH(UnquantizeCEValueFromRange(8, 7), "");
  EXPECT_DEBUG_DEATH(UnquantizeCEValueFromRange(-1000, 17), "");

  EXPECT_DEBUG_DEATH(QuantizeCEValueToRange(0, -7), "");
  EXPECT_DEBUG_DEATH(UnquantizeCEValueFromRange(0, -17), "");

  EXPECT_DEBUG_DEATH(QuantizeCEValueToRange(0, 257), "");
  EXPECT_DEBUG_DEATH(UnquantizeCEValueFromRange(0, 256), "");

  EXPECT_DEBUG_DEATH(QuantizeWeightToRange(-1, 10), "");
  EXPECT_DEBUG_DEATH(QuantizeWeightToRange(256, 7), "");
  EXPECT_DEBUG_DEATH(QuantizeWeightToRange(10000, 17), "");

  EXPECT_DEBUG_DEATH(UnquantizeWeightFromRange(-1, 10), "");
  EXPECT_DEBUG_DEATH(UnquantizeWeightFromRange(8, 7), "");
  EXPECT_DEBUG_DEATH(UnquantizeWeightFromRange(-1000, 17), "");

  EXPECT_DEBUG_DEATH(QuantizeWeightToRange(0, -7), "");
  EXPECT_DEBUG_DEATH(UnquantizeWeightFromRange(0, -17), "");

  EXPECT_DEBUG_DEATH(QuantizeWeightToRange(0, 32), "");
  EXPECT_DEBUG_DEATH(UnquantizeWeightFromRange(0, 64), "");
}

}  // namespace

}  // namespace astc_codec
