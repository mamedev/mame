// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    corefloat.h

    Floating-point utility functions.

***************************************************************************/

#ifndef MAME_UTIL_COREFLOAT_H
#define MAME_UTIL_COREFLOAT_H

#pragma once

#include <bit>
#include <cmath>
#include <cstdint>
#include <numbers>


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

// convert 32 bits to IEEE single-precision floating point
constexpr float u2f(std::uint32_t v) noexcept
{
	return std::bit_cast<float>(v);
}

// convert IEEE single-precision floating point to 32 bits
constexpr std::uint32_t f2u(float f) noexcept
{
	return std::bit_cast<std::uint32_t>(f);
}

// convert 64 bits to IEEE double-precision floating point
constexpr double u2d(std::uint64_t v) noexcept
{
	return std::bit_cast<double>(v);
}

// convert IEEE double-precision floating point to 64 bits
constexpr std::uint64_t d2u(double d) noexcept
{
	return std::bit_cast<std::uint64_t>(d);
}


// convert degrees to radians
template <typename T>
inline auto DEGREE_TO_RADIAN(T const &angle) noexcept
{
	return angle * std::numbers::pi_v<T> / T(180);
}

// convert radians to degrees
template <typename T>
inline auto RADIAN_TO_DEGREE(T const &angle) noexcept
{
	return angle * T(180) / std::numbers::pi_v<T>;
}

// efficient way to get the *positive*, floating-point x mod 1
template <typename T>
inline T fpmod1(T x)
{
	static_assert(std::is_floating_point_v<T>);
	if (x < T(0.0) || x >= T(1.0))
		x -= std::floor(x);
	return x;
}

// maps a floating-point number from [old_min, old_max] to [new_min, new_max]
template <typename T>
inline T fmaprange(T x, T old_min, T old_max, T new_min, T new_max)
{
	static_assert(std::is_floating_point_v<T>);
	return (new_max - new_min) * (x - old_min) / (old_max - old_min) + new_min;
}

// maps a floating-point number from [-1, 1] to [new_min, new_max]
template <typename T>
inline T fmaprange(T x, T new_min, T new_max)
{
	return fmaprange(x, T(-1.0), T(1.0), new_min, new_max);
}

#endif // MAME_UTIL_COREFLOAT_H
