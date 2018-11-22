// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    osdcomm.h

    Common definitions shared by the OSD layer. This includes the most
    fundamental integral types as well as compiler-specific tweaks.

***************************************************************************/

#pragma once

#ifndef MAME_OSD_OSDCOMM_H
#define MAME_OSD_OSDCOMM_H

#include <stdio.h>
#include <string.h>
#include <cstdint>
#include <type_traits>


/***************************************************************************
    COMPILER-SPECIFIC NASTINESS
***************************************************************************/

/* The Win32 port requires this constant for variable arg routines. */
#ifndef CLIB_DECL
#define CLIB_DECL
#endif


/* Some optimizations/warnings cleanups for GCC */
#if defined(__GNUC__)
#define ATTR_UNUSED             __attribute__((__unused__))
#define ATTR_PRINTF(x,y)        __attribute__((format(printf, x, y)))
#define ATTR_CONST              __attribute__((const))
#define ATTR_FORCE_INLINE       __attribute__((always_inline))
#define ATTR_HOT                __attribute__((hot))
#define ATTR_COLD               __attribute__((cold))
#define UNEXPECTED(exp)         __builtin_expect(!!(exp), 0)
#define EXPECTED(exp)           __builtin_expect(!!(exp), 1)
#define RESTRICT                __restrict__
#else
#define ATTR_UNUSED
#define ATTR_PRINTF(x,y)
#define ATTR_CONST
#define ATTR_FORCE_INLINE       __forceinline
#define ATTR_HOT
#define ATTR_COLD
#define UNEXPECTED(exp)         (exp)
#define EXPECTED(exp)           (exp)
#define RESTRICT
#endif



/***************************************************************************
    FUNDAMENTAL TYPES
***************************************************************************/

namespace osd {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

} // namespace OSD

/***************************************************************************
    FUNDAMENTAL MACROS
***************************************************************************/

/* Concatenate/extract 32-bit halves of 64-bit values */
constexpr uint64_t concat_64(uint32_t hi, uint32_t lo) { return (uint64_t(hi) << 32) | uint32_t(lo); }
constexpr uint32_t extract_64hi(uint64_t val) { return uint32_t(val >> 32); }
constexpr uint32_t extract_64lo(uint64_t val) { return uint32_t(val); }

// Highly useful template for compile-time knowledge of an array size
template <typename T, size_t N> constexpr size_t ARRAY_LENGTH(T (&)[N]) { return N;}

// For declaring an array of the same dimensions as another array (including multi-dimensional arrays)
template <typename T, typename U> struct equivalent_array_or_type { typedef T type; };
template <typename T, typename U, std::size_t N> struct equivalent_array_or_type<T, U[N]> { typedef typename equivalent_array_or_type<T, U>::type type[N]; };
template <typename T, typename U> using equivalent_array_or_type_t = typename equivalent_array_or_type<T, U>::type;
template <typename T, typename U> struct equivalent_array { };
template <typename T, typename U, std::size_t N> struct equivalent_array<T, U[N]> { typedef equivalent_array_or_type_t<T, U> type[N]; };
template <typename T, typename U> using equivalent_array_t = typename equivalent_array<T, U>::type;
#define EQUIVALENT_ARRAY(a, T) equivalent_array_t<T, std::remove_reference_t<decltype(a)> >

/* Macros for normalizing data into big or little endian formats */
constexpr uint16_t swapendian_int16(uint16_t val) { return (val << 8) | (val >> 8); }

constexpr uint32_t swapendian_int32_partial16(uint32_t val) { return ((val << 8) & 0xFF00FF00U) | ((val >> 8) & 0x00FF00FFU); }
constexpr uint32_t swapendian_int32(uint32_t val) { return (swapendian_int32_partial16(val) << 16) | (swapendian_int32_partial16(val) >> 16); }

constexpr uint64_t swapendian_int64_partial16(uint64_t val) { return ((val << 8) & 0xFF00FF00FF00FF00U) | ((val >> 8) & 0x00FF00FF00FF00FFU); }
constexpr uint64_t swapendian_int64_partial32(uint64_t val) { return ((swapendian_int64_partial16(val) << 16) & 0xFFFF0000FFFF0000U) | ((swapendian_int64_partial16(val) >> 16) & 0x0000FFFF0000FFFFU); }
constexpr uint64_t swapendian_int64(uint64_t val) { return (swapendian_int64_partial32(val) << 32) | (swapendian_int64_partial32(val) >> 32); }

#ifdef LSB_FIRST
constexpr uint16_t big_endianize_int16(uint16_t x) { return swapendian_int16(x); }
constexpr uint32_t big_endianize_int32(uint32_t x) { return swapendian_int32(x); }
constexpr uint64_t big_endianize_int64(uint64_t x) { return swapendian_int64(x); }
constexpr uint16_t little_endianize_int16(uint16_t x) { return x; }
constexpr uint32_t little_endianize_int32(uint32_t x) { return x; }
constexpr uint64_t little_endianize_int64(uint64_t x) { return x; }
#else
constexpr uint16_t big_endianize_int16(uint16_t x) { return x; }
constexpr uint32_t big_endianize_int32(uint32_t x) { return x; }
constexpr uint64_t big_endianize_int64(uint64_t x) { return x; }
constexpr uint16_t little_endianize_int16(uint16_t x) { return swapendian_int16(x); }
constexpr uint32_t little_endianize_int32(uint32_t x) { return swapendian_int32(x); }
constexpr uint64_t little_endianize_int64(uint64_t x) { return swapendian_int64(x); }
#endif /* LSB_FIRST */

#ifdef _MSC_VER
using ssize_t = std::make_signed_t<size_t>;
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca(size)  __builtin_alloca(size)
#endif
#endif

#endif  /* MAME_OSD_OSDCOMM_H */
