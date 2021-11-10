// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    endianness.h

    Endianness types and utility functions.

***************************************************************************/

#ifndef MAME_LIB_UTIL_ENDIANNESS_H
#define MAME_LIB_UTIL_ENDIANNESS_H

#pragma once

#include <string_view>


namespace util {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// constants for expression endianness
enum class endianness
{
	little,
	big,
#ifdef LSB_FIRST
	native = little
#else
	native = big
#endif
};


//**************************************************************************
//  MACROS AND INLINE FUNCTIONS
//**************************************************************************

constexpr std::string_view endian_to_string_view(endianness e) { using namespace std::literals; return e == endianness::little ? "little"sv : "big"sv; }

// endian-based value: first value is if native endianness is little-endian, second is if native is big-endian
#define NATIVE_ENDIAN_VALUE_LE_BE(leval,beval)  ((util::endianness::native == util::endianness::little) ? (leval) : (beval))


// inline functions for accessing bytes and words within larger chunks

// read/write a byte to a 16-bit space
template <typename T> constexpr T BYTE_XOR_BE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0); }
template <typename T> constexpr T BYTE_XOR_LE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(0,1); }

// read/write a byte to a 32-bit space
template <typename T> constexpr T BYTE4_XOR_BE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(3,0); }
template <typename T> constexpr T BYTE4_XOR_LE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(0,3); }

// read/write a word to a 32-bit space
template <typename T> constexpr T WORD_XOR_BE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0); }
template <typename T> constexpr T WORD_XOR_LE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(0,2); }

// read/write a byte to a 64-bit space
template <typename T> constexpr T BYTE8_XOR_BE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(7,0); }
template <typename T> constexpr T BYTE8_XOR_LE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(0,7); }

// read/write a word to a 64-bit space
template <typename T> constexpr T WORD2_XOR_BE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(6,0); }
template <typename T> constexpr T WORD2_XOR_LE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(0,6); }

// read/write a dword to a 64-bit space
template <typename T> constexpr T DWORD_XOR_BE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(4,0); }
template <typename T> constexpr T DWORD_XOR_LE(T a) { return a ^ NATIVE_ENDIAN_VALUE_LE_BE(0,4); }

} // namespace util

#endif // MAME_LIB_UTIL_ENDIANNESS_H
