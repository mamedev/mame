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
#define ATTR_NORETURN           __attribute__((noreturn))
#define ATTR_PRINTF(x,y)        __attribute__((format(printf, x, y)))
#define ATTR_CONST              __attribute__((const))
#define ATTR_FORCE_INLINE       __attribute__((always_inline))
#define ATTR_NONNULL(...)       __attribute__((nonnull(__VA_ARGS__)))
#define ATTR_DEPRECATED         __attribute__((deprecated))
#define ATTR_HOT                __attribute__((hot))
#define ATTR_COLD               __attribute__((cold))
#define UNEXPECTED(exp)         __builtin_expect(!!(exp), 0)
#define EXPECTED(exp)           __builtin_expect(!!(exp), 1)
#define RESTRICT                __restrict__
#else
#define ATTR_UNUSED
#define ATTR_NORETURN           __declspec(noreturn)
#define ATTR_PRINTF(x,y)
#define ATTR_CONST
#define ATTR_FORCE_INLINE       __forceinline
#define ATTR_NONNULL(...)
#define ATTR_DEPRECATED         __declspec(deprecated)
#define ATTR_HOT
#define ATTR_COLD
#define UNEXPECTED(exp)         (exp)
#define EXPECTED(exp)           (exp)
#define RESTRICT
#endif



/***************************************************************************
    FUNDAMENTAL TYPES
***************************************************************************/


/* 8-bit values */
using UINT8 = std::uint8_t;
using INT8 = std::int8_t;

/* 16-bit values */
using UINT16 = std::uint16_t;
using INT16 = std::int16_t;

/* 32-bit values */
using UINT32 = std::uint32_t;
using INT32 = std::int32_t;

/* 64-bit values */
using UINT64 = std::uint64_t;
using INT64 = std::int64_t;

/* pointer-sized values */
using FPTR = uintptr_t;

/* unicode types */
using utf16_char = std::uint16_t;
using unicode_char = std::uint32_t;




/***************************************************************************
    FUNDAMENTAL CONSTANTS
***************************************************************************/

/* Ensure that TRUE/FALSE are defined */
#ifndef TRUE
#define TRUE                1
#endif

#ifndef FALSE
#define FALSE               0
#endif



/***************************************************************************
    FUNDAMENTAL MACROS
***************************************************************************/

/* U64 and S64 are used to wrap long integer constants. */
#if defined(__GNUC__) || defined(_MSC_VER)
#define U64(val) val##ULL
#define S64(val) val##LL
#else
#define U64(val) val
#define S64(val) val
#endif


/* Concatenate/extract 32-bit halves of 64-bit values */
constexpr UINT64 concat_64(UINT32 hi, UINT32 lo) { return (UINT64(hi) << 32) | UINT32(lo); }
constexpr UINT32 extract_64hi(UINT64 val) { return UINT32(val >> 32); }
constexpr UINT32 extract_64lo(UINT64 val) { return UINT32(val); }

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
constexpr UINT16 flipendian_int16(UINT16 val) { return (val << 8) | (val >> 8); }

constexpr UINT32 flipendian_int32_partial16(UINT32 val) { return ((val << 8) & 0xFF00FF00U) | ((val >> 8) & 0x00FF00FFU); }
constexpr UINT32 flipendian_int32(UINT32 val) { return (flipendian_int32_partial16(val) << 16) | (flipendian_int32_partial16(val) >> 16); }

constexpr UINT64 flipendian_int64_partial16(UINT64 val) { return ((val << 8) & U64(0xFF00FF00FF00FF00)) | ((val >> 8) & U64(0x00FF00FF00FF00FF)); }
constexpr UINT64 flipendian_int64_partial32(UINT64 val) { return ((flipendian_int64_partial16(val) << 16) & U64(0xFFFF0000FFFF0000)) | ((flipendian_int64_partial16(val) >> 16) & U64(0x0000FFFF0000FFFF)); }
constexpr UINT64 flipendian_int64(UINT64 val) { return (flipendian_int64_partial32(val) << 32) | (flipendian_int64_partial32(val) >> 32); }

#ifdef LSB_FIRST
constexpr UINT16 BIG_ENDIANIZE_INT16(UINT16 x) { return flipendian_int16(x); }
constexpr UINT32 BIG_ENDIANIZE_INT32(UINT32 x) { return flipendian_int32(x); }
constexpr UINT64 BIG_ENDIANIZE_INT64(UINT64 x) { return flipendian_int64(x); }
#define LITTLE_ENDIANIZE_INT16(x)   (x)
#define LITTLE_ENDIANIZE_INT32(x)   (x)
#define LITTLE_ENDIANIZE_INT64(x)   (x)
#else
#define BIG_ENDIANIZE_INT16(x)      (x)
#define BIG_ENDIANIZE_INT32(x)      (x)
#define BIG_ENDIANIZE_INT64(x)      (x)
constexpr UINT16 LITTLE_ENDIANIZE_INT16(UINT16 x) { return flipendian_int16(x); }
constexpr UINT32 LITTLE_ENDIANIZE_INT32(UINT32 x) { return flipendian_int32(x); }
constexpr UINT64 LITTLE_ENDIANIZE_INT64(UINT64 x) { return flipendian_int64(x); }
#endif /* LSB_FIRST */

#ifdef _MSC_VER
#include <malloc.h>
using ssize_t = std::make_signed_t<size_t>;
#if _MSC_VER == 1900 // VS2015
#define __LINE__Var 0
#endif // VS2015
#if _MSC_VER < 1900 // VS2013 or earlier
#define snprintf _snprintf
#define __func__ __FUNCTION__
#else // VS2015
#define _CRT_STDIO_LEGACY_WIDE_SPECIFIERS
#endif
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca(size)  __builtin_alloca(size)
#endif
#endif

#endif  /* MAME_OSD_OSDCOMM_H */
