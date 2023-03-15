// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    osdcomm.h

    Common definitions shared by the OSD layer. This includes the most
    fundamental integral types as well as compiler-specific tweaks.

***************************************************************************/
#ifndef MAME_OSD_OSDCOMM_H
#define MAME_OSD_OSDCOMM_H

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <type_traits>


/***************************************************************************
    COMPILER-SPECIFIC NASTINESS
***************************************************************************/

// The Win32 port requires this constant for variable arg routines.
#ifndef CLIB_DECL
#define CLIB_DECL
#endif


// Some optimizations/warnings cleanups for GCC
#if defined(__GNUC__)
#define ATTR_PRINTF(x,y)        __attribute__((format(printf, x, y)))
#define ATTR_CONST              __attribute__((const))
#define ATTR_FORCE_INLINE       __attribute__((always_inline))
#define ATTR_HOT                __attribute__((hot))
#define ATTR_COLD               __attribute__((cold))
#define UNEXPECTED(exp)         __builtin_expect(!!(exp), 0)
#define EXPECTED(exp)           __builtin_expect(!!(exp), 1)
#define RESTRICT                __restrict__
#else
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

// Macros for normalizing data into big or little endian formats
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
#endif // LSB_FIRST

#ifdef _MSC_VER
using ssize_t = std::make_signed_t<size_t>;
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca(size)  __builtin_alloca(size)
#endif
#endif


// macro for defining a copy constructor and assignment operator to prevent copying
#define DISABLE_COPYING(TYPE) \
	TYPE(const TYPE &) = delete; \
	TYPE &operator=(const TYPE &) = delete

// macro for declaring enumeration operators that increment/decrement like plain old C
#define DECLARE_ENUM_INCDEC_OPERATORS(TYPE) \
inline TYPE &operator++(TYPE &value) { return value = TYPE(std::underlying_type_t<TYPE>(value) + 1); } \
inline TYPE &operator--(TYPE &value) { return value = TYPE(std::underlying_type_t<TYPE>(value) - 1); } \
inline TYPE operator++(TYPE &value, int) { TYPE const old(value); ++value; return old; } \
inline TYPE operator--(TYPE &value, int) { TYPE const old(value); --value; return old; }

// macro for declaring bitwise operators for an enumerated type
#define DECLARE_ENUM_BITWISE_OPERATORS(TYPE) \
constexpr TYPE operator~(TYPE value) { return TYPE(~std::underlying_type_t<TYPE>(value)); } \
constexpr TYPE operator&(TYPE a, TYPE b) { return TYPE(std::underlying_type_t<TYPE>(a) & std::underlying_type_t<TYPE>(b)); } \
constexpr TYPE operator|(TYPE a, TYPE b) { return TYPE(std::underlying_type_t<TYPE>(a) | std::underlying_type_t<TYPE>(b)); } \
inline TYPE &operator&=(TYPE &a, TYPE b) { return a = a & b; } \
inline TYPE &operator|=(TYPE &a, TYPE b) { return a = a | b; }

#endif // MAME_OSD_OSDCOMM_H
