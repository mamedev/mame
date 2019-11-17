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

#ifndef ASTC_CODEC_BASE_UINT128_H_
#define ASTC_CODEC_BASE_UINT128_H_

#include <cassert>
#include <cstdint>

namespace astc_codec {
namespace base {

class UInt128 {
 public:
  UInt128() = default;
  UInt128(uint64_t low) : low_(low) { }
  UInt128(uint64_t high, uint64_t low) : low_(low), high_(high) { }
  UInt128(const UInt128& other) : low_(other.low_), high_(other.high_) { }

  uint64_t LowBits() const { return low_; }
  uint64_t HighBits() const { return high_; }

  // Allow explicit casts to uint64_t.
  explicit operator uint64_t() const { return low_; }

  // Copy operators.
  UInt128& operator=(const UInt128& other) {
    high_ = other.high_;
    low_ = other.low_;
    return *this;
  }

  // Equality operators.
  bool operator==(const UInt128& other) const {
    return high_ == other.high_ && low_ == other.low_;
  }

  bool operator!=(const UInt128& other) const {
    return high_ != other.high_ || low_ != other.low_;
  }

  // Shifting.
  UInt128& operator<<=(int shift) {
    high_ = shift >= 64 ? (shift >= 128 ? 0 : low_ << (shift - 64))
                        : high_ << shift;

    if (shift > 0 && shift < 64) {
      const uint64_t overlappingBits = low_ >> (64 - shift);
      high_ |= overlappingBits;
    }

    low_ = shift >= 64 ? 0 : low_ << shift;
    return *this;
  }

  UInt128 operator<<(int shift) const {
    UInt128 result = *this;
    result <<= shift;
    return result;
  }

  UInt128& operator>>=(int shift) {
    low_ = shift >= 64 ? (shift >= 128 ? 0 : high_ >> (shift - 64))
                       : low_ >> shift;

    if (shift > 0 && shift < 64) {
      const uint64_t overlappingBits = high_ << (64 - shift);
      low_ |= overlappingBits;
    }

    high_ = shift >= 64 ? 0 : high_ >> shift;

    return *this;
  }

  UInt128 operator>>(int shift) const {
    UInt128 result = *this;
    result >>= shift;
    return result;
  }

  // Binary operations.
  UInt128& operator|=(const UInt128& other) {
    high_ |= other.high_;
    low_ |= other.low_;
    return *this;
  }

  UInt128 operator|(const UInt128& other) const {
    UInt128 result = *this;
    result |= other;
    return result;
  }

  UInt128& operator&=(const UInt128& other) {
    high_ &= other.high_;
    low_ &= other.low_;
    return *this;
  }

  UInt128 operator&(const UInt128& other) const {
    UInt128 result = *this;
    result &= other;
    return result;
  }

  UInt128& operator^=(const UInt128& other) {
    high_ ^= other.high_;
    low_ ^= other.low_;
    return *this;
  }

  UInt128 operator^(const UInt128& other) const {
    UInt128 result = *this;
    result ^= other;
    return result;
  }

  UInt128 operator~() const {
    UInt128 result = *this;
    result.high_ = ~high_;
    result.low_ = ~low_;
    return result;
  }

  // Addition/subtraction.
  UInt128& operator+=(const UInt128& other) {
    const uint64_t carry =
        (((low_ & other.low_) & 1) + (low_ >> 1) + (other.low_ >> 1)) >> 63;
    high_ += other.high_ + carry;
    low_ += other.low_;
    return *this;
  }

  UInt128 operator+(const UInt128& other) const {
    UInt128 result = *this;
    result += other;
    return result;
  }

  UInt128& operator-=(const UInt128& other) {
    low_ -= other.low_;
    const uint64_t carry =
        (((low_ & other.low_) & 1) + (low_ >> 1) + (other.low_ >> 1)) >> 63;
    high_ -= other.high_ + carry;
    return *this;
  }

  UInt128 operator-(const UInt128& other) const {
    UInt128 result = *this;
    result -= other;
    return result;
  }

 private:
  // TODO(google): Different order for little endian.
  uint64_t low_ = 0;
  uint64_t high_ = 0;
};

}  // namespace base
}  // namespace astc_codec

#endif  // ASTC_CODEC_BASE_UINT128_H_
