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

#ifndef ASTC_CODEC_BASE_BIT_STREAM_H_
#define ASTC_CODEC_BASE_BIT_STREAM_H_

#include <cassert>
#include <cstdint>

namespace astc_codec {
namespace base {

// Represents a stream of bits that can be read or written in arbitrary-sized
// chunks.
template<typename IntType = uint64_t>
class BitStream {
 public:
  // Creates an empty BitStream.
  BitStream() = default;
  BitStream(IntType data, uint32_t data_size)
      : data_(data), data_size_(data_size) {
    assert(data_size_ <= sizeof(data_) * 8);
  }

  // Return the number of bits in the stream.
  uint32_t Bits() const { return data_size_; }

  // Put |size| bits into the stream.
  // Fails if there is not enough space in the buffer to store the bits.
  template<typename ResultType>
  void PutBits(ResultType x, uint32_t size) {
    assert(data_size_ + size <= sizeof(data_) * 8);

    data_ |= (IntType(x) & MaskFor(size)) << data_size_;
    data_size_ += size;
  }

  // Get |count| bits from the stream.
  // Returns true if |count| bits were successfully retrieved.
  template<typename ResultType>
  bool GetBits(uint32_t count, ResultType* result) {
    if (count <= data_size_) {
      *result = static_cast<ResultType>(data_ & MaskFor(count));
      data_ = data_ >> count;
      data_size_ -= count;
      return true;
    } else {
      *result = ResultType();
      return false;
    }
  }

 private:
  IntType MaskFor(uint32_t bits) const {
    return (bits == sizeof(IntType) * 8) ? ~IntType(0)
                                         : (IntType(1) << bits) - 1;
  }

  IntType data_ = IntType();
  uint32_t data_size_ = 0;
};

}  // namespace base
}  // namespace astc_codec

#endif  // ASTC_CODEC_BASE_BIT_STREAM_H_
