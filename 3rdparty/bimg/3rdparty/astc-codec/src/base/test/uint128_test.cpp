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

#include "src/base/uint128.h"

#include <gtest/gtest.h>

namespace astc_codec {
namespace base {

TEST(UInt128, Equality) {
  const UInt128 zero(0);
  const UInt128 max64(~0ULL);

  EXPECT_EQ(zero, zero);
  EXPECT_NE(zero, max64);
  EXPECT_EQ(zero, UInt128(0));
  EXPECT_NE(zero, UInt128(1));
  EXPECT_EQ(max64, max64);
}

TEST(UInt128, Shifting) {
  const UInt128 max64(~0ULL);
  const UInt128 upper64(~0ULL, 0);
  EXPECT_EQ(upper64.HighBits(), ~0ULL);
  EXPECT_EQ(upper64.LowBits(), 0);

  EXPECT_EQ(upper64 >> 64, max64);

  EXPECT_EQ(UInt128(1) << 1, UInt128(2));
  EXPECT_EQ(UInt128(0) << 0, UInt128(0));
  EXPECT_EQ(max64 << 0, max64);
  EXPECT_EQ(max64 >> 0, max64);
  EXPECT_EQ(upper64 << 0, upper64);
  EXPECT_EQ(upper64 >> 0, upper64);

  {
    const UInt128 bit63 = UInt128(1ULL << 62) << 1;
    EXPECT_EQ(bit63.LowBits(), 1ULL << 63);
    EXPECT_EQ(bit63.HighBits(), 0);
  }

  {
    const UInt128 bit64 = UInt128(1ULL << 63) << 1;
    EXPECT_EQ(bit64.LowBits(), 0);
    EXPECT_EQ(bit64.HighBits(), 1);
    EXPECT_EQ(bit64 >> 1, UInt128(1ULL << 63));
  }

  {
    const UInt128 overshift = max64 << 128;
    EXPECT_EQ(overshift.HighBits(), 0);
    EXPECT_EQ(overshift.LowBits(), 0);
  }

  {
    const UInt128 overlap = upper64 >> 32;
    EXPECT_EQ(overlap.HighBits(), 0x00000000FFFFFFFF);
    EXPECT_EQ(overlap.LowBits(), 0xFFFFFFFF00000000);
  }

  {
    const UInt128 overlap = max64 << 32;
    EXPECT_EQ(overlap.HighBits(), 0x00000000FFFFFFFF);
    EXPECT_EQ(overlap.LowBits(), 0xFFFFFFFF00000000);
  }
}

TEST(UInt128, LargeShift) {
  const UInt128 base(0xFF);
  EXPECT_EQ(base << 64, UInt128(0xFFULL, 0));
  EXPECT_EQ(base << 72, UInt128(0xFF00ULL, 0));
  EXPECT_EQ(base << 80, UInt128(0xFF0000ULL, 0));
  EXPECT_EQ(base << 88, UInt128(0xFF000000ULL, 0));
  EXPECT_EQ(base << 96, UInt128(0xFF00000000ULL, 0));
  EXPECT_EQ(base << 104, UInt128(0xFF0000000000ULL, 0));
  EXPECT_EQ(base << 112, UInt128(0xFF000000000000ULL, 0));
  EXPECT_EQ(base << 120, UInt128(0xFF00000000000000ULL, 0));

  const UInt128 upper(0xFF00000000000000ULL, 0);
  EXPECT_EQ(upper >> 64, UInt128(0, 0xFF00000000000000ULL));
  EXPECT_EQ(upper >> 72, UInt128(0, 0xFF000000000000ULL));
  EXPECT_EQ(upper >> 80, UInt128(0, 0xFF0000000000ULL));
  EXPECT_EQ(upper >> 88, UInt128(0, 0xFF00000000ULL));
  EXPECT_EQ(upper >> 96, UInt128(0, 0xFF000000ULL));
  EXPECT_EQ(upper >> 104, UInt128(0, 0xFF0000ULL));
  EXPECT_EQ(upper >> 112, UInt128(0, 0xFF00ULL));
  EXPECT_EQ(upper >> 120, UInt128(0, 0xFFULL));
}

TEST(UInt128, BooleanOperators) {
  const UInt128 allOnes(~0ULL, ~0ULL);
  EXPECT_EQ(allOnes.HighBits(), ~0ULL);
  EXPECT_EQ(allOnes.LowBits(), ~0ULL);

  EXPECT_EQ(~allOnes, UInt128(0));
  EXPECT_EQ(~UInt128(0), allOnes);

  EXPECT_EQ(UInt128(0xFFFF00) & UInt128(0x00FFFF), UInt128(0x00FF00));
  EXPECT_EQ(UInt128(0xFFFF00) | UInt128(0x00FFFF), UInt128(0xFFFFFF));
  EXPECT_EQ(UInt128(0xFFFF00) ^ UInt128(0x00FFFF), UInt128(0xFF00FF));
}

TEST(UInt128, Addition) {
  const UInt128 bit63(1ULL << 63);

  EXPECT_EQ(UInt128(1) + 1, UInt128(2));
  EXPECT_EQ(bit63 + bit63, UInt128(1) << 64);

  const UInt128 carryUp = UInt128(~0ULL) + 1;
  EXPECT_EQ(carryUp.HighBits(), 1);
  EXPECT_EQ(carryUp.LowBits(), 0);

  const UInt128 allOnes(~0ULL, ~0ULL);
  EXPECT_EQ(allOnes + 1, UInt128(0));
}

TEST(UInt128, Subtraction) {
  const UInt128 bit64 = UInt128(1) << 64;
  EXPECT_EQ(bit64 - 1, UInt128(~0ULL));

  EXPECT_EQ(UInt128(1) - 1, UInt128(0));

  const UInt128 allOnes(~0ULL, ~0ULL);
  EXPECT_EQ(UInt128(0) - 1, allOnes);
}

}  // namespace base
}  // namespace astc_codec
