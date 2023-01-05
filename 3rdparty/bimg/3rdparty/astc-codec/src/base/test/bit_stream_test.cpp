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

#include "src/base/bit_stream.h"

#include <gtest/gtest.h>

namespace astc_codec {
namespace base {

namespace {
  static constexpr uint64_t kAllBits = 0xFFFFFFFFFFFFFFFF;
  static constexpr uint64_t k40Bits = 0x000000FFFFFFFFFF;
}

TEST(BitStream, Decode) {
  {
    BitStream<uint64_t> stream(0, 1);

    uint64_t bits = kAllBits;
    EXPECT_TRUE(stream.GetBits(1, &bits));
    EXPECT_EQ(bits, 0);
    EXPECT_FALSE(stream.GetBits(1, &bits));
  }

  {
    BitStream<uint64_t> stream(0b1010101010101010, 32);
    EXPECT_EQ(stream.Bits(), 32);

    uint64_t bits = 0;
    EXPECT_TRUE(stream.GetBits(1, &bits));
    EXPECT_EQ(bits, 0);

    EXPECT_TRUE(stream.GetBits(3, &bits));
    EXPECT_EQ(bits, 0b101);

    EXPECT_TRUE(stream.GetBits(8, &bits));
    EXPECT_EQ(bits, 0b10101010);

    EXPECT_EQ(stream.Bits(), 20);

    EXPECT_TRUE(stream.GetBits(20, &bits));
    EXPECT_EQ(bits, 0b1010);
    EXPECT_EQ(stream.Bits(), 0);
  }

  {
    BitStream<uint64_t> stream(kAllBits, 64);
    EXPECT_EQ(stream.Bits(), 64);

    uint64_t bits = 0;
    EXPECT_TRUE(stream.GetBits(64, &bits));
    EXPECT_EQ(bits, kAllBits);
    EXPECT_EQ(stream.Bits(), 0);
  }

  {
    BitStream<uint64_t> stream(kAllBits, 64);
    EXPECT_EQ(stream.Bits(), 64);

    uint64_t bits = 0;
    EXPECT_TRUE(stream.GetBits(40, &bits));
    EXPECT_EQ(bits, k40Bits);
    EXPECT_EQ(stream.Bits(), 24);
  }

  {
    BitStream<uint64_t> stream(kAllBits, 32);

    uint64_t bits = 0;
    EXPECT_TRUE(stream.GetBits(0, &bits));
    EXPECT_EQ(bits, 0);
    EXPECT_TRUE(stream.GetBits(32, &bits));
    EXPECT_EQ(bits, k40Bits & 0xFFFFFFFF);
    EXPECT_TRUE(stream.GetBits(0, &bits));
    EXPECT_EQ(bits, 0);
    EXPECT_EQ(stream.Bits(), 0);
  }
}

TEST(BitStream, Encode) {
  {
    BitStream<uint64_t> stream;

    stream.PutBits(0, 1);
    stream.PutBits(0b11, 2);
    EXPECT_EQ(stream.Bits(), 3);

    uint64_t bits = 0;
    EXPECT_TRUE(stream.GetBits(3, &bits));
    EXPECT_EQ(bits, 0b110);
  }

  {
    BitStream<uint64_t> stream;

    uint64_t bits = 0;
    stream.PutBits(kAllBits, 64);
    EXPECT_EQ(stream.Bits(), 64);

    EXPECT_TRUE(stream.GetBits(64, &bits));
    EXPECT_EQ(bits, kAllBits);
    EXPECT_EQ(stream.Bits(), 0);
  }

  {
    BitStream<uint64_t> stream;
    stream.PutBits(kAllBits, 40);

    uint64_t bits = 0;
    EXPECT_TRUE(stream.GetBits(40, &bits));
    EXPECT_EQ(bits, k40Bits);
    EXPECT_EQ(stream.Bits(), 0);
  }

  {
    BitStream<uint64_t> stream;
    stream.PutBits(0, 0);
    stream.PutBits(kAllBits, 32);
    stream.PutBits(0, 0);

    uint64_t bits = 0;
    EXPECT_TRUE(stream.GetBits(32, &bits));
    EXPECT_EQ(bits, k40Bits & 0xFFFFFFFF);
    EXPECT_EQ(stream.Bits(), 0);
  }
}

}  // namespace base
}  // namespace astc_codec
