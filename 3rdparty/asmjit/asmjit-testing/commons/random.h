// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef TESTING_COMMONS_RANDOM_H_INCLUDED
#define TESTING_COMMONS_RANDOM_H_INCLUDED

#include <stdint.h>
#include <string.h>

namespace TestUtils {
namespace {

// A pseudo random number generator based on a paper by Sebastiano Vigna:
//   http://vigna.di.unimi.it/ftp/papers/xorshiftplus.pdf
class Random {
public:
  // Constants suggested as `23/18/5`.
  static inline constexpr uint32_t kStep1_SHL = 23;
  static inline constexpr uint32_t kStep2_SHR = 18;
  static inline constexpr uint32_t kStep3_SHR = 5;

  uint64_t _state[2];

  inline explicit Random(uint64_t seed = 0) noexcept { reset(seed); }
  inline Random(const Random& other) noexcept = default;

  inline void reset(uint64_t seed = 0) noexcept {
    // The number is arbitrary, it means nothing.
    constexpr uint64_t kZeroSeed = 0x1F0A2BE71D163FA0u;

    // Generate the state data by using splitmix64.
    for (uint32_t i = 0; i < 2; i++) {
      seed += 0x9E3779B97F4A7C15u;
      uint64_t x = seed;
      x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9u;
      x = (x ^ (x >> 27)) * 0x94D049BB133111EBu;
      x = (x ^ (x >> 31));
      _state[i] = x != 0 ? x : kZeroSeed;
    }
  }

  inline uint32_t next_uint32() noexcept {
    return uint32_t(next_uint64() >> 32);
  }

  inline uint64_t next_uint64() noexcept {
    uint64_t x = _state[0];
    uint64_t y = _state[1];

    x ^= x << kStep1_SHL;
    y ^= y >> kStep3_SHR;
    x ^= x >> kStep2_SHR;
    x ^= y;

    _state[0] = y;
    _state[1] = x;
    return x + y;
  }

  inline double next_double() noexcept {
    constexpr uint32_t kMantissaShift = 64 - 52;
    constexpr uint64_t kExpMsk = 0x3FF0000000000000u;

    uint64_t u = (next_uint64() >> kMantissaShift) | kExpMsk;
    double d = 0.0;

    memcpy(&d, &u, 8);
    return d - 1.0;
  }
};

} // {anonymous}
} // {TestUtils}

#endif // TESTING_COMMONS_RANDOM_H_INCLUDED
