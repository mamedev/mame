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

#ifndef ASTC_CODEC_BASE_MATH_UTILS_H_
#define ASTC_CODEC_BASE_MATH_UTILS_H_

#include "src/base/uint128.h"

#include <cassert>
#include <cstdint>
#include <type_traits>

namespace astc_codec {
namespace base {

inline int Log2Floor(uint32_t n) {
  if (n == 0) {
    return -1;
  }

  int log = 0;
  uint32_t value = n;
  for (int i = 4; i >= 0; --i) {
    int shift = (1 << i);
    uint32_t x = value >> shift;
    if (x != 0) {
      value = x;
      log += shift;
    }
  }
  assert(value == 1);
  return log;
}

inline int CountOnes(uint32_t n) {
  n -= ((n >> 1) & 0x55555555);
  n = ((n >> 2) & 0x33333333) + (n & 0x33333333);
  return static_cast<int>((((n + (n >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24);
}

template<typename T>
inline T ReverseBits(T value) {
  uint32_t s = sizeof(value) * 8;
  T mask = ~T(0);
  while ((s >>= 1) > 0) {
    mask ^= (mask << s);
    value = ((value >> s) & mask) | ((value << s) & ~mask);
  }

  return value;
}

template<typename T>
inline T GetBits(T source, uint32_t offset, uint32_t count) {
  static_assert(std::is_same<T, UInt128>::value || std::is_unsigned<T>::value,
                "T must be unsigned.");

  const uint32_t total_bits = sizeof(T) * 8;
  assert(count > 0);
  assert(offset + count <= total_bits);

  const T mask = count == total_bits ? ~T(0) : ~T(0) >> (total_bits - count);
  return (source >> offset) & mask;
}

}  // namespace base
}  // namespace astc_codec

#endif  // ASTC_CODEC_BASE_MATH_UTILS_H_
