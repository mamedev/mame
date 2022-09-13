// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    endianness.h

    Endianness types and utility functions.

***************************************************************************/

#ifndef MAME_LIB_UTIL_ENDIANNESS_H
#define MAME_LIB_UTIL_ENDIANNESS_H

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>


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


// helper for accessing data adjusted for endianness
template <typename In, typename Out, endianness Endian>
class offset_endian_cast
{
private:
	static inline constexpr std::ptrdiff_t SWIZZLE = (sizeof(In) / sizeof(Out)) - 1;

	static_assert(!(sizeof(In) % sizeof(Out)), "input size must be a multiple of output size");
	static_assert(!((sizeof(In) / sizeof(Out)) & SWIZZLE), "ratio of input size to output size must be a power of two");

	Out *m_ptr;
	std::ptrdiff_t m_offs;

public:
	constexpr offset_endian_cast(In *ptr, std::ptrdiff_t offs) noexcept : m_ptr(reinterpret_cast<Out *>(ptr)), m_offs(offs) { }

	constexpr Out &operator[](std::ptrdiff_t i) const noexcept { return m_ptr[(m_offs + i) ^ ((Endian != endianness::native) ? SWIZZLE : 0)]; }
	constexpr Out &operator*() const noexcept { return m_ptr[m_offs ^ ((Endian != endianness::native) ? SWIZZLE : 0)]; }

	constexpr offset_endian_cast operator+(std::ptrdiff_t i) const noexcept { return offset_endian_cast(*this) += i; }
	constexpr offset_endian_cast operator-(std::ptrdiff_t i) const noexcept { return offset_endian_cast(*this) -= i; }

	offset_endian_cast &operator+=(std::ptrdiff_t i) noexcept { m_offs += i; return *this; }
	offset_endian_cast &operator-=(std::ptrdiff_t i) noexcept { m_offs -= i; return *this; }
	offset_endian_cast &operator++() noexcept { ++m_offs; return *this; }
	offset_endian_cast &operator--() noexcept { --m_offs; return *this; }
	offset_endian_cast operator++(int) noexcept { offset_endian_cast result(*this); ++m_offs; return result; }
	offset_endian_cast operator--(int) noexcept { offset_endian_cast result(*this); --m_offs; return result; }
};


// helper for accessing data adjusted for endianness
template <typename In, typename Out, endianness Endian>
class endian_cast
{
private:
	static inline constexpr std::ptrdiff_t SWIZZLE = (sizeof(In) / sizeof(Out)) - 1;

	static_assert(!(sizeof(In) % sizeof(Out)), "input size must be a multiple of output size");
	static_assert(!((sizeof(In) / sizeof(Out)) & SWIZZLE), "ratio of input size to output size must be a power of two");

	Out *m_ptr;

public:
	constexpr endian_cast(In *ptr) noexcept : m_ptr(reinterpret_cast<Out *>(ptr)) { }

	constexpr Out &operator[](std::ptrdiff_t i) const noexcept { return m_ptr[i ^ ((Endian != endianness::native) ? SWIZZLE : 0)]; }

	constexpr auto operator+(std::ptrdiff_t offs) const noexcept
	{
		using required_const = std::conditional_t<std::is_const_v<Out>, std::add_const_t<In>, In>;
		using required_cv = std::conditional_t<std::is_volatile_v<Out>, std::add_volatile<required_const>, required_const>;
		return offset_endian_cast<required_cv, Out, Endian>(reinterpret_cast<required_cv *>(m_ptr), offs);
	}

	constexpr auto operator-(std::ptrdiff_t offs) const noexcept
	{
		using required_const = std::conditional_t<std::is_const_v<Out>, std::add_const_t<In>, In>;
		using required_cv = std::conditional_t<std::is_volatile_v<Out>, std::add_volatile<required_const>, required_const>;
		return offset_endian_cast<required_cv, Out, Endian>(reinterpret_cast<required_cv *>(m_ptr), -offs);
	}
};



//**************************************************************************
//  MACROS AND INLINE FUNCTIONS
//**************************************************************************

constexpr std::string_view endian_to_string_view(endianness e) { using namespace std::literals; return e == endianness::little ? "little"sv : "big"sv; }

// endian-based value: first value is if native endianness is little-endian, second is if native is big-endian
#define NATIVE_ENDIAN_VALUE_LE_BE(leval, beval)  ((util::endianness::native == util::endianness::little) ? (leval) : (beval))


// inline functions for accessing bytes and words within larger chunks

template <typename T, typename U>
auto big_endian_cast(U *ptr)
{
	using requested_const = std::conditional_t<std::is_const_v<U>, std::add_const_t<T>, T>;
	using requested_cv = std::conditional_t<std::is_volatile_v<U>, std::add_volatile<requested_const>, requested_const>;
	return endian_cast<U, requested_cv, endianness::big>(ptr);
}

template <typename T, typename U>
auto little_endian_cast(U *ptr)
{
	using requested_const = std::conditional_t<std::is_const_v<U>, std::add_const_t<T>, T>;
	using requested_cv = std::conditional_t<std::is_volatile_v<U>, std::add_volatile<requested_const>, requested_const>;
	return endian_cast<U, requested_cv, endianness::little>(ptr);
}

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
