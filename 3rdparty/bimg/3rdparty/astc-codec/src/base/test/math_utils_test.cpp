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

#include "src/base/math_utils.h"

#include <gtest/gtest.h>

namespace astc_codec {
namespace base {

TEST(MathUtils, Log2Floor) {
  EXPECT_EQ(-1, Log2Floor(0));

  for (int i = 0; i < 32; i++) {
    uint32_t n = 1U << i;
    EXPECT_EQ(i, Log2Floor(n));
    if (n > 2) {
      EXPECT_EQ(i - 1, Log2Floor(n - 1));
      EXPECT_EQ(i,     Log2Floor(n + 1));
    }
  }
}

TEST(MathUtils, CountOnes) {
  EXPECT_EQ(0, CountOnes(0));
  EXPECT_EQ(1, CountOnes(1));
  EXPECT_EQ(32, CountOnes(static_cast<uint32_t>(~0U)));
  EXPECT_EQ(1, CountOnes(0x8000000));

  for (int i = 0; i < 32; i++) {
    EXPECT_EQ(1, CountOnes(1U << i));
    EXPECT_EQ(31, CountOnes(static_cast<uint32_t>(~0U) ^ (1U << i)));
  }
}

TEST(MathUtils, ReverseBits) {
  EXPECT_EQ(ReverseBits(0u), 0u);
  EXPECT_EQ(ReverseBits(1u), 1u << 31);
  EXPECT_EQ(ReverseBits(0xffffffff), 0xffffffff);
  EXPECT_EQ(ReverseBits(0x00000001), 0x80000000);
  EXPECT_EQ(ReverseBits(0x80000000), 0x00000001);
  EXPECT_EQ(ReverseBits(0xaaaaaaaa), 0x55555555);
  EXPECT_EQ(ReverseBits(0x55555555), 0xaaaaaaaa);
  EXPECT_EQ(ReverseBits(0x7d5d7f53), 0xcafebabe);
  EXPECT_EQ(ReverseBits(0xcafebabe), 0x7d5d7f53);
}

TEST(MathUtils, GetBits) {
  EXPECT_EQ(GetBits(0u, 0, 1), 0u);
  EXPECT_EQ(GetBits(0u, 0, 32), 0u);
  EXPECT_EQ(GetBits(0x00000001u, 0, 1), 0x00000001);
  EXPECT_EQ(GetBits(0x00000001u, 0, 32), 0x00000001);
  EXPECT_EQ(GetBits(0x00000001u, 1, 31), 0x00000000);
  EXPECT_EQ(GetBits(0x00000001u, 31, 1), 0x00000000);

  EXPECT_DEBUG_DEATH(GetBits(0x00000000u, 1, 32), "");
  EXPECT_DEBUG_DEATH(GetBits(0x00000000u, 32, 0), "");
  EXPECT_DEBUG_DEATH(GetBits(0x00000000u, 32, 1), "");

  EXPECT_EQ(GetBits(0XFFFFFFFFu, 0, 4), 0x0000000F);
  EXPECT_EQ(GetBits(0XFFFFFFFFu, 16, 16), 0xFFFF);
  EXPECT_EQ(GetBits(0x80000000u, 31, 1), 1);
  EXPECT_EQ(GetBits(0xCAFEBABEu, 24, 8), 0xCA);
}

}  // namespace base
}  // namespace astc_codec
