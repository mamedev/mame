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

#include "src/decoder/integer_sequence_codec.h"
#include "src/base/uint128.h"

#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

using astc_codec::base::UInt128;
using astc_codec::base::BitStream;
using astc_codec::IntegerSequenceCodec;
using astc_codec::IntegerSequenceEncoder;
using astc_codec::IntegerSequenceDecoder;

namespace {

// Make sure that the counts returned for a specific range match what's
// expected. In particular, make sure that it fits with Table C.2.7
TEST(ASTCIntegerSequenceCodecTest, TestGetCountsForRange) {
  std::array<int, 3> kExpectedCounts[31] = {
    {{ 0, 0, 1 }},  // 1
    {{ 1, 0, 0 }},  // 2
    {{ 0, 0, 2 }},  // 3
    {{ 0, 1, 0 }},  // 4
    {{ 1, 0, 1 }},  // 5
    {{ 0, 0, 3 }},  // 6
    {{ 0, 0, 3 }},  // 7
    {{ 0, 1, 1 }},  // 8
    {{ 0, 1, 1 }},  // 9
    {{ 1, 0, 2 }},  // 10
    {{ 1, 0, 2 }},  // 11
    {{ 0, 0, 4 }},  // 12
    {{ 0, 0, 4 }},  // 13
    {{ 0, 0, 4 }},  // 14
    {{ 0, 0, 4 }},  // 15
    {{ 0, 1, 2 }},  // 16
    {{ 0, 1, 2 }},  // 17
    {{ 0, 1, 2 }},  // 18
    {{ 0, 1, 2 }},  // 19
    {{ 1, 0, 3 }},  // 20
    {{ 1, 0, 3 }},  // 21
    {{ 1, 0, 3 }},  // 22
    {{ 1, 0, 3 }},  // 23
    {{ 0, 0, 5 }},  // 24
    {{ 0, 0, 5 }},  // 25
    {{ 0, 0, 5 }},  // 26
    {{ 0, 0, 5 }},  // 27
    {{ 0, 0, 5 }},  // 28
    {{ 0, 0, 5 }},  // 29
    {{ 0, 0, 5 }},  // 30
    {{ 0, 0, 5 }},  // 31
  };

  int t, q, b;
  for (int i = 1; i < 32; ++i) {
    IntegerSequenceCodec::GetCountsForRange(i, &t, &q, &b);
    EXPECT_EQ(t, kExpectedCounts[i - 1][0]);
    EXPECT_EQ(q, kExpectedCounts[i - 1][1]);
    EXPECT_EQ(b, kExpectedCounts[i - 1][2]);
  }

  ASSERT_DEATH(IntegerSequenceCodec::GetCountsForRange(0, &t, &q, &b), "");
  ASSERT_DEATH(IntegerSequenceCodec::GetCountsForRange(256, &t, &q, &b), "");

  IntegerSequenceCodec::GetCountsForRange(1, &t, &q, &b);
  EXPECT_EQ(t, 0);
  EXPECT_EQ(q, 0);
  EXPECT_EQ(b, 1);
}

// Test to make sure that we're calculating the number of bits needed to
// encode a given number of values based on the range of the values.
TEST(ASTCIntegerSequenceCodecTest, TestNumBitsForCounts) {
  int trits = 0;
  int quints = 0;
  int bits = 0;

  // A range of one should have single bits, so n 1-bit values should be n bits.
  trits = 0;
  quints = 0;
  bits = 1;
  for (int i = 0; i < 64; ++i) {
    EXPECT_EQ(IntegerSequenceCodec::GetBitCount(i, trits, quints, bits), i);
    EXPECT_EQ(IntegerSequenceCodec::GetBitCountForRange(i, 1), i);
  }

  // Similarly, N two-bit values should be 2n bits...
  trits = 0;
  quints = 0;
  bits = 2;
  for (int i = 0; i < 64; ++i) {
    int bit_counts = IntegerSequenceCodec::GetBitCount(i, trits, quints, bits);
    EXPECT_EQ(bit_counts, 2 * i);
    EXPECT_EQ(IntegerSequenceCodec::GetBitCountForRange(i, 3), 2 * i);
  }

  // Trits are a bit more complicated -- there are five trits in a block, so
  // if we encode 15 values with 3 bits each in trits, we'd get three blocks,
  // each with eight bits of trits.
  trits = 1;
  quints = 0;
  bits = 3;
  EXPECT_EQ(IntegerSequenceCodec::GetBitCount(15, trits, quints, bits),
            8 * 3 + 15 * 3);
  EXPECT_EQ(IntegerSequenceCodec::GetBitCountForRange(15, 23),
            IntegerSequenceCodec::GetBitCount(15, trits, quints, bits));

  // However, if instead we encode 13 values, we don't need to use the remaining
  // two values, so we only need bits as they will be encoded. As it turns out,
  // this means we can avoid three bits in the final block (one for the high
  // order trit encoding and two for one of the values), resulting in 47 bits.
  trits = 1;
  quints = 0;
  bits = 2;
  EXPECT_EQ(IntegerSequenceCodec::GetBitCount(13, trits, quints, bits), 47);
  EXPECT_EQ(IntegerSequenceCodec::GetBitCountForRange(13, 11),
            IntegerSequenceCodec::GetBitCount(13, trits, quints, bits));

  // Quints have a similar property -- if we encode six values using a quint and
  // four bits, then we have two quint blocks each with three values and a seven
  // bit encoded quint triplet...
  trits = 0;
  quints = 1;
  bits = 4;
  EXPECT_EQ(IntegerSequenceCodec::GetBitCount(6, trits, quints, bits),
            7 * 2 + 6 * 4);
  EXPECT_EQ(IntegerSequenceCodec::GetBitCountForRange(6, 79),
            IntegerSequenceCodec::GetBitCount(6, trits, quints, bits));

  // If we have fewer values than blocks we can again avoid about 2 + nbits
  // bits...
  trits = 0;
  quints = 1;
  bits = 3;
  EXPECT_EQ(IntegerSequenceCodec::GetBitCount(7, trits, quints, bits),
            /* first two quint blocks */ 7 * 2 +
            /* first two blocks of bits */ 6 * 3 +
            /* last quint block without the high order four bits */ 3 +
            /* last block with one set of three bits */ 3);
}

// Tests that the encoder knows how to encode values of the form 5*2^k.
TEST(ASTCIntegerSequenceCodecTest, TestQuintCodec) {
  // In this case, k = 4

  // Setup bit src/sink
  BitStream<UInt128> bit_sink;

  const int kValueRange = 79;
  IntegerSequenceEncoder enc(kValueRange);
  enc.AddValue(3);
  enc.AddValue(79);
  enc.AddValue(37);
  enc.Encode(&bit_sink);

  // quint: 1000101 m0: 0011 m1: 1111 m2: 0101
  // 100 0100 0111 1101 0010
  // interleaved 10m200m1101m0
  // should be 100 1010 0111 1101 0011 = 0x4A7D3
  EXPECT_EQ(bit_sink.Bits(), 19);

  uint64_t encoded = 0;
  bit_sink.GetBits(19, &encoded);
  EXPECT_EQ(encoded, 0x4A7D3);

  // Now check that decoding it works as well
  BitStream<UInt128> bit_src(encoded, 19);

  IntegerSequenceDecoder dec(kValueRange);
  auto decoded_vals = dec.Decode(3, &bit_src);
  ASSERT_EQ(decoded_vals.size(), 3);
  EXPECT_EQ(decoded_vals[0], 3);
  EXPECT_EQ(decoded_vals[1], 79);
  EXPECT_EQ(decoded_vals[2], 37);
}

// Tests that the encoder knows how to encode values of the form 3*2^k.
TEST(ASTCIntegerSequenceCodecTest, TestTritCodec) {
  uint64_t encoded = 0;

  // Setup bit src/sink
  BitStream<UInt128> bit_sink(encoded, 0);

  const int kValueRange = 11;
  IntegerSequenceEncoder enc(kValueRange);
  enc.AddValue(7);
  enc.AddValue(5);
  enc.AddValue(3);
  enc.AddValue(6);
  enc.AddValue(10);
  enc.Encode(&bit_sink);

  EXPECT_EQ(bit_sink.Bits(), 18);

  bit_sink.GetBits(18, &encoded);
  EXPECT_EQ(encoded, 0x37357);

  // Now check that decoding it works as well
  BitStream<UInt128> bit_src(encoded, 19);

  IntegerSequenceDecoder dec(kValueRange);
  auto decoded_vals = dec.Decode(5, &bit_src);
  ASSERT_EQ(decoded_vals.size(), 5);
  EXPECT_EQ(decoded_vals[0], 7);
  EXPECT_EQ(decoded_vals[1], 5);
  EXPECT_EQ(decoded_vals[2], 3);
  EXPECT_EQ(decoded_vals[3], 6);
  EXPECT_EQ(decoded_vals[4], 10);
}

// Test a specific quint encoding/decoding. This test makes sure that the way we
// encode and decode integer sequences matches what we should expect out of the
// reference ASTC encoder.
TEST(ASTCIntegerSequenceCodecTest, TestDecodeThenEncode) {
  std::vector<int> vals = {{ 16, 18, 17, 4, 7, 14, 10, 0 }};
  const uint64_t kValEncoding = 0x2b9c83dc;

  BitStream<UInt128> bit_src(kValEncoding, 64);
  IntegerSequenceDecoder dec(19);
  auto decoded_vals = dec.Decode(8, &bit_src);
  ASSERT_EQ(decoded_vals.size(), vals.size());
  for (size_t i = 0; i < decoded_vals.size(); ++i) {
    EXPECT_EQ(decoded_vals[i], vals[i]);
  }

  // Setup bit src/sink
  BitStream<UInt128> bit_sink;
  IntegerSequenceEncoder enc(19);
  for (const auto& v : vals) {
    enc.AddValue(v);
  }
  enc.Encode(&bit_sink);
  EXPECT_EQ(bit_sink.Bits(), 35);

  uint64_t encoded = 0;
  EXPECT_TRUE(bit_sink.GetBits(35, &encoded));
  EXPECT_EQ(encoded, kValEncoding)
      << std::hex << encoded << " -- " << kValEncoding;
}

// Same as the previous test, except it uses a trit encoding rather than a
// quint encoding.
TEST(ASTCIntegerSequenceCodecTest, TestDecodeThenEncodeTrits) {
  std::vector<int> vals = {{ 6, 0, 0, 2, 0, 0, 0, 0, 8, 0, 0, 0, 0, 8, 8, 0 }};
  const uint64_t kValEncoding = 0x0004c0100001006ULL;

  BitStream<UInt128> bit_src(kValEncoding, 64);
  IntegerSequenceDecoder dec(11);
  auto decoded_vals = dec.Decode(vals.size(), &bit_src);
  ASSERT_EQ(decoded_vals.size(), vals.size());
  for (size_t i = 0; i < decoded_vals.size(); ++i) {
    EXPECT_EQ(decoded_vals[i], vals[i]);
  }

  // Setup bit src/sink
  BitStream<UInt128> bit_sink;
  IntegerSequenceEncoder enc(11);
  for (const auto& v : vals) {
    enc.AddValue(v);
  }
  enc.Encode(&bit_sink);
  EXPECT_EQ(bit_sink.Bits(), 58);

  uint64_t encoded = 0;
  EXPECT_TRUE(bit_sink.GetBits(58, &encoded));
  EXPECT_EQ(encoded, kValEncoding)
      << std::hex << encoded << " -- " << kValEncoding;
}

// Generate a random sequence of integer codings with different ranges to test
// the reciprocability of our codec (encoded sequences should be able to
// decoded)
TEST(ASTCIntegerSequenceCodecTest, TestRandomReciprocation) {
  std::mt19937 mt(0xbad7357);
  std::uniform_int_distribution<int> rand(0, 255);

  for (int test = 0; test < 1600; ++test) {
    // Generate a random number of values and a random range
    int num_vals = 4 + rand(mt) % 44;  // Up to 48 weights in a grid
    int range = 1 + rand(mt) % 63;

    // If this produces a bit pattern larger than our buffer, then ignore
    // it... we already know what our bounds are for the integer sequences
    int num_bits = IntegerSequenceCodec::GetBitCountForRange(num_vals, range);
    if (num_bits >= 64) {
      continue;
    }

    std::vector<int> generated_vals(num_vals);
    for (auto& val : generated_vals) {
      val = rand(mt) % (range + 1);
    }

    // Encode the values using the
    BitStream<UInt128> bit_sink;

    // Add them to the encoder
    IntegerSequenceEncoder enc(range);
    for (int v : generated_vals) {
      enc.AddValue(v);
    }
    enc.Encode(&bit_sink);

    uint64_t encoded = 0;
    bit_sink.GetBits(bit_sink.Bits(), &encoded);
    ASSERT_GE(encoded, 0);
    EXPECT_LT(encoded, 1ULL << num_bits);

    BitStream<UInt128> bit_src(encoded, 64);

    IntegerSequenceDecoder dec(range);
    auto decoded_vals = dec.Decode(num_vals, &bit_src);

    ASSERT_EQ(decoded_vals.size(), generated_vals.size());
    for (size_t i = 0; i < decoded_vals.size(); ++i) {
      EXPECT_EQ(decoded_vals[i], generated_vals[i]);
    }
  }
}

}  // namespace
