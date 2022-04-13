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

#ifndef ASTC_CODEC_DECODER_INTEGER_SEQUENCE_CODEC_H_
#define ASTC_CODEC_DECODER_INTEGER_SEQUENCE_CODEC_H_

#include "src/base/bit_stream.h"
#include "src/base/uint128.h"

#include <array>
#include <string>
#include <vector>

namespace astc_codec {

// The maximum number of bits that we would need to encode an ISE value. The
// ASTC specification does not give a maximum number, however unquantized color
// values have a maximum range of 255, meaning that we can't feasibly have more
// than eight bits per value.
constexpr int kLog2MaxRangeForBits = 8;

// Ranges can take any of the the forms 2^k, 3*2^k, or 5*2^k for k up to
// kLog2MaxRangeForBits. Hence we have three types of ranges. Since the
// maximum encoded value is 255, k won't go larger than 8. We don't have quints
// that accompany [6, 8]-bits, as (5 * 2^6 = 320 > 255) and we don't have trits
// that accompany [7, 8]-bits, as (3 * 2^7 = 384 > 255). But we do have trits
// and quints that accompany no bits. Hence we have a total of
// 3 * kLog2MaxRangeForBits - 3 - 2 + 2 total ranges.
constexpr int kNumPossibleRanges = 3 * kLog2MaxRangeForBits - 3;

// Returns an iterator through the available ASTC ranges.
std::array<int, kNumPossibleRanges>::const_iterator ISERangeBegin();
std::array<int, kNumPossibleRanges>::const_iterator ISERangeEnd();

// Base class for ASTC integer sequence encoders and decoders. These codecs
// operate on sequences of integers and produce bit patterns that pack the
// integers based on the encoding scheme specified in the ASTC specification
// Section C.2.12. The resulting bit pattern is a sequence of encoded blocks.
// All blocks in a sequence are one of the following encodings:
//
//   (1 -- bit encoding) one encoded value of the form 2^k
//   (2 -- trit encoding) five encoded values of the form 3*2^k
//   (3 -- quint encoding) three encoded values of the form 5*2^k
//
// The layouts of each block are designed such that the blocks can be truncated
// during encoding in order to support variable length input sequences (i.e. a
// sequence of values that are encoded using trit encoded blocks does not
// need to have a multiple-of-five length).
class IntegerSequenceCodec {
 public:
  // Returns the number of trits, quints, and bits needed to encode values in
  // [0, range]. This is used to determine the layout of ISE encoded bit
  // streams. The returned array holds the number of trits, quints, and bits
  // respectively. range is expected to be within the interval [1, 5242879]
  static void GetCountsForRange(int range, int* trits, int* quints, int* bits);

  // Returns the number of bits needed to encode the given number of values with
  // respect to the number of trits, quints, and bits specified in ise_counts
  // (in that order). It is expected that either trits or quints can be
  // nonzero, but not both, and neither can be larger than one. Anything else is
  // undefined.
  static int GetBitCount(int num_vals, int trits, int quints, int bits);

  // Convenience function that returns the number of bits needed to encoded
  // num_vals within the range [0, range] (inclusive).
  static inline int GetBitCountForRange(int num_vals, int range) {
    int trits, quints, bits;
    GetCountsForRange(range, &trits, &quints, &bits);
    return GetBitCount(num_vals, trits, quints, bits);
  }

 protected:
  explicit IntegerSequenceCodec(int range);
  IntegerSequenceCodec(int trits, int quints, int bits);

  // The encoding mode -- since having trits and quints are mutually exclusive,
  // we can store the encoding we decide on in this enum.
  enum EncodingMode {
    kTritEncoding = 0,
    kQuintEncoding,
    kBitEncoding,
  };

  EncodingMode encoding_;
  int bits_;

  // Returns the number of values stored in a single ISE block. Since quints and
  // trits are packed three/five to a bit pattern (respectively), each sequence
  // is chunked into blocks in order to encode it. For only bit-encodings, the
  // block size is one.
  int NumValsPerBlock() const;

  // Returns the size of a single ISE block in bits (see NumValsPerBlock).
  int EncodedBlockSize() const;

 private:
  // Determines the encoding mode.
  void InitializeWithCounts(int trits, int quints, int bits);
};

// The integer sequence decoder. The decoder only remembers the given encoding
// but each invocation of Decode operates independently on the input bits.
class IntegerSequenceDecoder : public IntegerSequenceCodec {
 public:
  // Creates a decoder that decodes values within [0, range] (inclusive).
  explicit IntegerSequenceDecoder(int range)
      : IntegerSequenceCodec(range) { }

  // Creates a decoder based on the number of trits, quints, and bits expected
  // in the bit stream passed to Decode.
  IntegerSequenceDecoder(int trits, int quints, int bits)
      : IntegerSequenceCodec(trits, quints, bits) { }

  // Decodes num_vals from the bit_src. The number of bits read is dependent
  // on the number of bits required to encode num_vals based on the calculation
  // provided in Section C.2.22 of the ASTC specification. The return value
  // always contains exactly num_vals.
  std::vector<int> Decode(int num_vals,
                          base::BitStream<base::UInt128>* bit_src) const;
};

// The integer sequence encoder. The encoder accepts values one by one and
// places them into a temporary array that it holds. When needed the user
// may call Encode to produce an encoded bit stream of the associated values.
class IntegerSequenceEncoder : public IntegerSequenceCodec {
 public:
  // Creates an encoder that encodes values within [0, range] (inclusive).
  explicit IntegerSequenceEncoder(int range)
      : IntegerSequenceCodec(range) { }

  // Creates an encoder based on the number of trits, quints, and bits for
  // the bit stream produced by Encode.
  IntegerSequenceEncoder(int trits, int quints, int bits)
      : IntegerSequenceCodec(trits, quints, bits) { }

  // Adds a value to the encoding sequence.
  void AddValue(int val) {
    // Make sure it's within bounds
    assert(encoding_ != EncodingMode::kTritEncoding || val < 3 * (1 << bits_));
    assert(encoding_ != EncodingMode::kQuintEncoding || val < 5 * (1 << bits_));
    assert(encoding_ != EncodingMode::kBitEncoding || val < (1 << bits_));
    vals_.push_back(val);
  }

  // Writes the encoding for vals_ to the bit_sink. Multiple calls to Encode
  // will produce the same result.
  void Encode(base::BitStream<base::UInt128>* bit_sink) const;

  // Removes all of the previously added values to the encoder.
  void Reset() { vals_.clear(); }

 private:
  std::vector<int> vals_;
};

}  // namespace astc_codec

#endif  // ASTC_CODEC_DECODER_INTEGER_SEQUENCE_CODEC_H_
