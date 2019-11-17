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

#include "src/decoder/weight_infill.h"
#include "src/decoder/footprint.h"

#include <gtest/gtest.h>

#include <vector>

namespace astc_codec {

namespace {

// Make sure that the physical size of the bit representations for certain
// dimensions of weight grids matches our expectations
TEST(ASTCWeightInfillTest, TestGetBitCount) {
  // Bit encodings
  EXPECT_EQ(32, CountBitsForWeights(4, 4, 3));
  EXPECT_EQ(48, CountBitsForWeights(4, 4, 7));
  EXPECT_EQ(24, CountBitsForWeights(2, 4, 7));
  EXPECT_EQ(8, CountBitsForWeights(2, 4, 1));

  // Trit encodings
  EXPECT_EQ(32, CountBitsForWeights(4, 5, 2));
  EXPECT_EQ(26, CountBitsForWeights(4, 4, 2));
  EXPECT_EQ(52, CountBitsForWeights(4, 5, 5));
  EXPECT_EQ(42, CountBitsForWeights(4, 4, 5));

  // Quint encodings
  EXPECT_EQ(21, CountBitsForWeights(3, 3, 4));
  EXPECT_EQ(38, CountBitsForWeights(4, 4, 4));
  EXPECT_EQ(49, CountBitsForWeights(3, 7, 4));
  EXPECT_EQ(52, CountBitsForWeights(4, 3, 19));
  EXPECT_EQ(70, CountBitsForWeights(4, 4, 19));
}

// Make sure that we bilerp our weights properly
TEST(ASTCWeightInfillTest, TestInfillBilerp) {
  std::vector<int> weights = InfillWeights(
      {{ 1, 3, 5, 3, 5, 7, 5, 7, 9 }}, Footprint::Get5x5(), 3, 3);

  std::vector<int> expected_weights = {
      1, 2, 3, 4, 5,
      2, 3, 4, 5, 6,
      3, 4, 5, 6, 7,
      4, 5, 6, 7, 8,
      5, 6, 7, 8, 9 };

  ASSERT_EQ(weights.size(), expected_weights.size());
  for (int i = 0; i < weights.size(); ++i) {
    EXPECT_EQ(weights[i], expected_weights[i]);
  }
}

}  // namespace

}  // namespace astc_codec
