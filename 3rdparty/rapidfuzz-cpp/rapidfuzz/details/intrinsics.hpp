/* SPDX-License-Identifier: MIT */
/* Copyright © 2021 Max Bachmann */

#pragma once

#include <cstdint>

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace rapidfuzz {
namespace intrinsics {

static inline uint64_t addc64(uint64_t a, uint64_t b, uint64_t carryin, uint64_t* carryout)
{
    /* todo should use _addcarry_u64 when available */
    a += carryin;
    *carryout = a < carryin;
    a += b;
    *carryout |= a < b;
    return a;
}

static inline std::size_t popcount64(uint64_t x)
{
    const uint64_t m1 = 0x5555555555555555;
    const uint64_t m2 = 0x3333333333333333;
    const uint64_t m4 = 0x0f0f0f0f0f0f0f0f;
    const uint64_t h01 = 0x0101010101010101;

    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    return static_cast<std::size_t>((x * h01) >> 56);
}

/**
 * Extract the lowest set bit from a. If no bits are set in a returns 0.
 */
template <typename T>
T blsi(T a)
{
    return a & -a;
}

/**
 * Clear the lowest set bit in a.
 */
template <typename T>
T blsr(T x)
{
    return x & (x - 1);
}

/**
 * Sets all the lower bits of the result to 1 up to and including lowest set bit (=1) in a.
 * If a is zero, blsmsk sets all bits to 1.
 */
template <typename T>
T blsmsk(T a)
{
    return a ^ (a - 1);
}

#if defined(_MSC_VER) && !defined(__clang__)
static inline int tzcnt(uint32_t x) {
    unsigned long trailing_zero = 0;
    _BitScanForward(&trailing_zero, x);
    return trailing_zero;
}

#if defined(_M_ARM) || defined(_M_X64)
static inline int tzcnt(uint64_t x) {
    unsigned long trailing_zero = 0;
    _BitScanForward64(&trailing_zero, x);
    return trailing_zero;
}
#else
static inline int tzcnt(uint64_t x) {
    uint32_t msh = (uint32_t)(x >> 32);
    uint32_t lsh = (uint32_t)(x & 0xFFFFFFFF);
    if (lsh != 0) {
        return tzcnt(lsh);
    }
    return 32 + tzcnt(msh);
}
#endif

#else /*  gcc / clang */
static inline int tzcnt(uint32_t x)
{
    return __builtin_ctz(x);
}

static inline int tzcnt(uint64_t x)
{
    return __builtin_ctzll(x);
}
#endif

    
} // namespace intrinsics
} // namespace rapidfuzz
