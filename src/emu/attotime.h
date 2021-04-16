// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

attotime.h

Support functions for working with attotime data.

***************************************************************************/
#ifndef MAME_EMU_ATTOTIME_H
#define MAME_EMU_ATTOTIME_H

#pragma once

#include "emucore.h"
#include "xtal.h"

#include <cmath>
#undef min
#undef max


//**************************************************************************
//  TYPE DEFINITIONS
//***************************************************************************/

class save_manager;

//
// "Attotime" is the name for the core timing structure in MAME. It comes from
// the level of accuracy it attempts to capture, which is 1 attosecond, or
// 10e-18. This may seem ridiculously overaccurate, but the goal was to be
// able to compute the time of a clock pulse in the GHz range, and increment
// it at a granularity of a single tick, and not create significant drift over
// the course of a second.
//
// There are two structures used to represent these times: a full 'attotime'
// and a 'subsecond'.
//
// An attotime is conceptually a 96-bit signed integral value that holds 62 bits
// of fractional seconds, plus 32 bits of seconds, plus 2 bits of flags. A
// signed 32-bit value for seconds gives +/-2e32 seconds, or about +/-68 years
// of time, which sould be enough for anyone. ;) The subseconds portion is
// a 62-bit fractional value, and the 2 bits of flags are used to indicate the
// special value of "never".
//
// A subsecond is a 64-bit chunk of the full attotime that is used for
// computations on short times of +/-2 seconds. It retains the full subsecond
// accuracy of 62 bits, and uses the remaining 2 bits as a signed seconds value.
// The subsecond cannot retain the value of "never"; it just maxes out at the
// extremes.
//
// Visually the relationship is:
//
//   attotime:                                                                      never flag (01) -----vv
//  +----------------+----------------+----------------+----------------+----------------+--------------+--+
//  |    seconds (32-bits, signed)    |                       subseconds (62 bits)                      |xx|
//  +----------------+----------------+----------------+----------------+----------------+--------------+--+
//
//   subsecond:                     vv----seconds (2 bits, signed)
//                                 +--+----------------+----------------+----------------+--------------+
//                                 |  |                       subseconds (62 bits)                      |
//                                 +--+----------------+----------------+----------------+--------------+
//
// The special value of "never" is indicated when the lower two bits of a full
// attotime are set to non-zero (01, specifically). "Never" is checked for when
// doing core math operations, meaning that, for example, adding any value to
// "never" produces "never".
//

// ======================> subseconds

// subseconds is a class that represents a fully-accurate value from -2 to +2 seconds
class subseconds
{
	// save_manager needs direct access for save/restore purposes
	friend class save_manager;

	// a divide that rounds up; this logic is such that when you convert
	// a frequency to subseconds, and then add those up over the course
	// of a second, you end up slightly ahead rather than slightly behind
	// due to rounding; in general this is the more useful behavior
	static constexpr s64 divide_up(s64 dividend, s64 divisor)
	{
		return (dividend - 1) / divisor + 1;
	}

	// internal constructor from raw
	constexpr subseconds(s64 raw) :
		m_subseconds(raw)
	{
	}

public:
	// number of units per time
	static constexpr s64 PER_SECOND = 0x4000000000000000ll;
	static constexpr s64 PER_MILLISECOND = PER_SECOND / 1000;
	static constexpr s64 PER_MICROSECOND = PER_SECOND / 1000000;
	static constexpr s64 PER_NANOSECOND = PER_SECOND / 1000000000;

	// minimum/maximum raw values
	static constexpr s64 MAX_RAW = PER_SECOND + (PER_SECOND - 1);
	static constexpr s64 MIN_RAW = -2 * PER_SECOND;

	// empty constructor
	constexpr subseconds() noexcept :
		m_subseconds(0)
	{
	}

	// copy constructor
	constexpr subseconds(subseconds const &src) noexcept :
		m_subseconds(src.m_subseconds)
	{
	}

	// copy assignment
	constexpr subseconds &operator=(subseconds const &src) noexcept
	{
		m_subseconds = src.m_subseconds;
		return *this;
	}

	// simple getters
	constexpr s64 raw() const noexcept { return m_subseconds; }

	// queries
	constexpr bool is_zero() const noexcept { return (m_subseconds == 0); }

	// constant values
	static constexpr subseconds max() noexcept { return subseconds::from_raw(MAX_RAW); }
	static constexpr subseconds min() noexcept { return subseconds::from_raw(MIN_RAW); }
	static constexpr subseconds unit() noexcept { return subseconds::from_raw(1); }
	static constexpr subseconds one_second() noexcept { return subseconds::from_raw(PER_SECOND); }
	static constexpr subseconds zero() noexcept { return subseconds::from_raw(0); }

	// static creators
	static constexpr subseconds from_raw(s64 raw) noexcept { return subseconds(raw); }
	static constexpr subseconds from_double(double secs) noexcept { return subseconds(s64(secs * double(PER_SECOND))); }
	static constexpr subseconds from_msec(s32 msec) noexcept { return subseconds(s64(msec) * PER_MILLISECOND); }
	static constexpr subseconds from_usec(s32 usec) noexcept { return subseconds(s64(usec) * PER_MICROSECOND); }
	static constexpr subseconds from_nsec(s32 nsec) noexcept { return subseconds(s64(nsec) * PER_NANOSECOND); }

	// static creator from integral hz value
	template<typename T>
	static constexpr std::enable_if_t<std::is_integral<T>::value, subseconds> from_hz(T hz) noexcept
	{
		return subseconds((hz > 0) ? divide_up(PER_SECOND, hz) : MAX_RAW);
	}

	// static creator from floating-point hz value
	static constexpr subseconds from_hz(double hz) noexcept
	{
		// prefer to use integral logic if the value is effectively an integer
		assert(hz > 0);
		s64 const hz_int = s64(hz);
		if (hz == double(hz_int))
			return subseconds(divide_up(PER_SECOND, hz_int));

		// otherwise, the do it in floating-point
		return subseconds(s64(double(PER_SECOND) / hz) + 1);
	}

	// static creator from XTAL; just extract the double value and use that
	static constexpr subseconds from_hz(XTAL const &xtal) noexcept { return from_hz(xtal.dvalue()); }

	// conversion to subseconds from ticks
	template<typename T>
	static constexpr subseconds from_ticks(s64 ticks, T hz) noexcept { return ticks * from_hz(hz); }

	// conversion helpers
	constexpr double as_hz() const noexcept { assert(!is_zero()); return double(PER_SECOND) / double(m_subseconds); }
	constexpr double as_khz() const noexcept { assert(!is_zero()); return (double(PER_SECOND) * 1e-3) / double(m_subseconds); }
	constexpr double as_mhz() const noexcept { assert(!is_zero()); return (double(PER_SECOND) * 1e-6) / double(m_subseconds); }
	constexpr double as_double() const noexcept { return m_subseconds * (1.0 / double(PER_SECOND)); }
	constexpr double as_msec() const noexcept { return m_subseconds * (1.0 / double(PER_MILLISECOND)); }
	constexpr double as_usec() const noexcept { return m_subseconds * (1.0 / double(PER_MICROSECOND)); }
	constexpr double as_nsec() const noexcept { return m_subseconds * (1.0 / double(PER_NANOSECOND)); }
	constexpr s32 as_msec_int() const noexcept { return m_subseconds / PER_MILLISECOND; }
	constexpr s32 as_usec_int() const noexcept { return m_subseconds / PER_MICROSECOND; }
	constexpr s32 as_nsec_int() const noexcept { return m_subseconds / PER_NANOSECOND; }

	// conversion to ticks from subseconds
	template<typename T>
	constexpr std::enable_if_t<std::is_integral<T>::value, s64> as_ticks(T hz) const noexcept
	{
		assert(hz > 0);
		return (m_subseconds >> 62) * hz + mulu_64x64_hi(m_subseconds << 2, hz);
	}
	constexpr s64 as_ticks(double hz) const noexcept
	{
		assert(hz > 0);
		return as_ticks(s64(hz * 2e31)) >> 31;
	}
	constexpr s64 as_ticks(XTAL const &xtal) const noexcept { return as_ticks(xtal.dvalue()); }
	constexpr s64 as_ticks(subseconds period) const noexcept { return *this / period; }

	// conversion to string
	const char *as_string(int precision = 9, bool dividers = false) const noexcept;
	std::string to_string(int precision = 9, bool dividers = true) const;

	// functions for addition of two subseconds
	constexpr subseconds &operator+=(subseconds const &right) noexcept
	{
		m_subseconds += right.m_subseconds;
		return *this;
	}
	friend constexpr subseconds operator+(subseconds const &left, subseconds const &right) noexcept;

	// functions for negation of a subseconds value
	friend constexpr subseconds operator-(subseconds const &left) noexcept;

	// functions for subtraction of two subseconds
	constexpr subseconds &operator-=(subseconds const &right) noexcept
	{
		m_subseconds -= right.m_subseconds;
		return *this;
	}
	friend constexpr subseconds operator-(subseconds const &left, subseconds const &right) noexcept;

	// functions for multiplication of subseconds by an integral factor
	template<typename T>
	constexpr std::enable_if_t<std::is_integral<T>::value, subseconds> &operator*=(T factor) noexcept
	{
		m_subseconds *= s64(factor);
		return *this;
	}
	template<typename T> friend constexpr std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(subseconds const &left, T factor) noexcept;
	template<typename T> friend constexpr std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(T factor, subseconds const &right) noexcept;

	// functions for division of subseconds by an integral factor
	template<typename T>
	constexpr std::enable_if_t<std::is_integral<T>::value, subseconds> &operator/=(T factor)
	{
		m_subseconds /= s64(factor);
		return *this;
	}
	template<typename T> friend constexpr std::enable_if_t<std::is_integral<T>::value, subseconds> operator/(subseconds const &left, T factor) noexcept;

	// functions for division of subseconds by another subseconds value
	friend constexpr s64 operator/(subseconds const &left, subseconds const &right) noexcept;

	// functions for equality comparisons
	friend constexpr bool operator==(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator!=(subseconds const &left, subseconds const &right) noexcept;

	// functions for less than comparisons
	friend constexpr bool operator<(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator<=(subseconds const &left, subseconds const &right) noexcept;

	// functions for greater than comparisons
	friend constexpr bool operator>(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator>=(subseconds const &left, subseconds const &right) noexcept;

private:
	// internal state
	s64 m_subseconds;
};

// addition of two subseconds
inline constexpr subseconds operator+(subseconds const &left, subseconds const &right) noexcept
{
	return subseconds(left.m_subseconds + right.m_subseconds);
}

// negation of a subseconds value
inline constexpr subseconds operator-(subseconds const &left) noexcept
{
	return subseconds(-left.m_subseconds);
}

// subtactions of two subseconds
inline constexpr subseconds operator-(subseconds const &left, subseconds const &right) noexcept
{
	return subseconds(left.m_subseconds - right.m_subseconds);
}

// multiplication of subseconds by an integral factor
template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(subseconds const &left, T factor) noexcept
{
	return subseconds(left.m_subseconds * s64(factor));
}

template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(T factor, subseconds const &right) noexcept
{
	return subseconds(s64(factor) * right.m_subseconds);
}

// division of subseconds by an integral factor
template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, subseconds> operator/(subseconds const &left, T factor) noexcept
{
	return subseconds(left.m_subseconds / s64(factor));
}

// division of subseconds by another subseconds value
inline constexpr s64 operator/(subseconds const &left, subseconds const &right) noexcept
{
	return left.m_subseconds / right.m_subseconds;
}

// equality comparisons
inline constexpr bool operator==(subseconds const &left, subseconds const &right) noexcept
{
	return left.m_subseconds == right.m_subseconds;
}
inline constexpr bool operator!=(subseconds const &left, subseconds const &right) noexcept
{
	return left.m_subseconds != right.m_subseconds;
}

// less than comparisons
inline constexpr bool operator<(subseconds const &left, subseconds const &right) noexcept
{
	return left.m_subseconds < right.m_subseconds;
}
inline constexpr bool operator<=(subseconds const &left, subseconds const &right) noexcept
{
	return left.m_subseconds <= right.m_subseconds;
}

// greater than comparisons
inline constexpr bool operator>(subseconds const &left, subseconds const &right) noexcept
{
	return left.m_subseconds > right.m_subseconds;
}
inline constexpr bool operator>=(subseconds const &left, subseconds const &right) noexcept
{
	return left.m_subseconds >= right.m_subseconds;
}


// ======================> attotime

// attotime is a class holds a 94-bit time value, with a signed 32-bit seconds value
// and a 62-bit subseconds value
class attotime
{
	// save_manager needs direct access for save/restore purposes
	friend class ::save_manager;

	// coarse and fine resolutions
	static constexpr double COARSE_FACTOR = double(1ll << 32);
	static constexpr double FINE_FACTOR = double(1ll << 62);
	static constexpr double COARSE_FACTOR_INV = 1.0 / COARSE_FACTOR;
	static constexpr double FINE_FACTOR_INV = 1.0 / FINE_FACTOR;

	// maximum values
	static constexpr s32 MAX_SECONDS = s32(0x7fffffff);
	static constexpr s32 MIN_SECONDS = s32(0x80000000);

	// internal helper for creation
	constexpr attotime(s64 coarse, u32 fine) noexcept :
		m_fine(fine), m_coarse(coarse)
	{
	}

	// internal helper for creating strings
	// maximum length = sign + 9 seconds digits + decimal + 19 sub-digits + 19/3 dividers + terminator
	static constexpr int MAX_STRING_LEN = 1 + 9 + 1 + 19 + 19/3 + 1;
	char const *generate_string(char *buffer, int precision = 9, bool dividers = true) const noexcept;

	// never check
	constexpr static bool is_never(u64 fineval) noexcept { return (BIT(fineval, 0, 2) != 0); }

public:
	// constants
	static attotime const zero;
	static attotime const never;

	// construction/destruction
	constexpr attotime() noexcept :
		m_fine(0),
		m_coarse(0)
	{
	}

	// construction with integral seconds
	constexpr explicit attotime(s32 secs) noexcept :
		attotime(s64(secs) << 32, 0)
	{
	}

	// construction from subseconds
	constexpr attotime(subseconds subs) noexcept :
		attotime(subs.raw() >> 30, u32(subs.raw() << 2))
	{
	}

	// construction with integral seconds plus fractional subseconds
	constexpr attotime(s32 secs, subseconds subs) noexcept :
		attotime((s64(secs) << 32) + (subs.raw() >> 30), u32(subs.raw() << 2))
	{
	}

	// copy constructor
	constexpr attotime(attotime const &src) noexcept :
		m_fine(src.m_fine),
		m_coarse(src.m_coarse)
	{
	}

	// assignment
	constexpr attotime &operator=(attotime const &src) noexcept
	{
		m_coarse = src.m_coarse;
		m_fine = src.m_fine;
		return *this;
	}

	// extraction of full seconds and fractional subseconds
	constexpr s32 seconds() const noexcept { return s32(m_coarse >> 32); }
	constexpr subseconds frac() const noexcept { return subseconds::from_raw(((m_coarse & 0xffffffffll) << 30) | (m_fine >> 2)); }

	// queries
	constexpr bool is_zero() const noexcept { return ((m_coarse | m_fine) == 0); }
	constexpr bool is_never() const noexcept { return is_never(m_fine); }

	// static creators
	static constexpr attotime from_raw(s64 coarse, u32 fine) noexcept { return attotime(coarse, fine); }
	static constexpr attotime from_seconds(s32 seconds) noexcept { return attotime(seconds); }
	static constexpr attotime from_msec(s64 msec) noexcept { return attotime(msec / 1000, subseconds::from_msec(msec % 1000)); }
	static constexpr attotime from_usec(s64 usec) noexcept { return attotime(usec / 1000000, subseconds::from_usec(usec % 1000000)); }
	static constexpr attotime from_nsec(s64 nsec) noexcept { return attotime(nsec / 1000000000, subseconds::from_nsec(nsec % 1000000000)); }

	// static creator from double
	static constexpr attotime from_double(double _time) noexcept
	{
		if (_time > MAX_SECONDS || _time < MIN_SECONDS)
			return never;
		double upper = floor(_time * COARSE_FACTOR);
		s64 coarse = s64(upper);
		u32 fine = u32((_time - upper) * (FINE_FACTOR / COARSE_FACTOR));
		return attotime(coarse, fine);
	}

	// static creator from integral hz value
	template<typename T>
	static constexpr std::enable_if_t<std::is_integral<T>::value, attotime> from_hz(T hz) noexcept
	{
		return (hz > 0) ? attotime(subseconds::from_hz(hz)) : never;
	}

	// static creator from floating-point hz value
	static constexpr attotime from_hz(double hz) noexcept
	{
		if (hz > 1.0)
			return attotime(subseconds::from_hz(hz));
		else if (hz > 0.0)
			return from_double(1.0 / hz);
		else
			return never;
	}

	// static creator from XTAL; just extract the double value and use that
	static constexpr attotime from_hz(const XTAL &xtal) noexcept { return from_hz(xtal.dvalue()); }

	// conversion to attotime from ticks
	template<typename T>
	static constexpr std::enable_if_t<std::is_integral<T>::value, attotime> from_ticks(s64 ticks, T hz) noexcept
	{
		return attotime(ticks / s64(hz), subseconds::from_ticks(ticks % s64(hz), hz));
	}
	static constexpr attotime from_ticks(s64 ticks, double hz) noexcept
	{
		// prefer to use integral logic if the value is effectively an integer
		assert(hz > 0);
		s64 const hz_int = s64(hz);
		if (hz == double(hz_int))
			return from_ticks(ticks, hz_int);

		// otherwise, just do it in floating-point
		return from_double(double(ticks) / hz);
	}
	static constexpr attotime from_ticks(s64 ticks, const XTAL &xtal) noexcept { return from_ticks(ticks, xtal.dvalue()); }

	// conversion helpers
	constexpr double as_hz() const noexcept { assert(!is_zero()); return 1.0 / as_double(); }
	constexpr double as_khz() const noexcept { assert(!is_zero()); return 1e-3 / as_double(); }
	constexpr double as_mhz() const noexcept { assert(!is_zero()); return 1e-6 / as_double(); }
	constexpr double as_double() const noexcept { return double(m_coarse) * COARSE_FACTOR_INV + double(m_fine >> 2) * FINE_FACTOR_INV; }
	constexpr double as_msec() const noexcept { return double(m_coarse) * (COARSE_FACTOR_INV * 1e3) + double(m_fine >> 2) * (FINE_FACTOR_INV * 1e3); }
	constexpr double as_usec() const noexcept { return double(m_coarse) * (COARSE_FACTOR_INV * 1e6) + double(m_fine >> 2) * (FINE_FACTOR_INV * 1e6); }
	constexpr double as_nsec() const noexcept { return double(m_coarse) * (COARSE_FACTOR_INV * 1e9) + double(m_fine >> 2) * (FINE_FACTOR_INV * 1e9); }
	constexpr s64 as_msec_int() const noexcept { return as_ticks(1000); }
	constexpr s64 as_usec_int() const noexcept { return as_ticks(1000000); }
	constexpr s64 as_nsec_int() const noexcept { return as_ticks(1000000000); }

	// conversion to ticks from attotime
	template<typename T>
	constexpr std::enable_if_t<std::is_integral<T>::value, s64> as_ticks(T hz) const noexcept
	{
		assert(hz > 0);
		return s64(seconds()) * s64(hz) + frac().as_ticks(hz);
	}
	constexpr s64 as_ticks(double hz) const
	{
		assert(hz > 0);
		return double(seconds()) * hz + frac().as_ticks(hz);
	}
	constexpr s64 as_ticks(XTAL const &xtal) const noexcept { return as_ticks(xtal.dvalue()); }
	constexpr s64 as_ticks(subseconds period) const noexcept { return *this / period; }

	// conversion to subseconds
	constexpr subseconds as_subseconds() const noexcept
	{
		s64 value = m_coarse << 30;
		if ((value >> 30) == m_coarse)
			return subseconds::from_raw(value | (m_fine >> 2));
		return (m_coarse >= 0) ? subseconds::max() : subseconds::min();
	}

	// conversion to string
	const char *as_string(int precision = 9, bool dividers = false) const noexcept;
	std::string to_string(int precision = 9, bool dividers = true) const;

	// functions for addition of two attotimes
	constexpr attotime &operator+=(attotime const &right) noexcept;
	friend constexpr attotime operator+(attotime const &left, attotime const &right) noexcept;

	// functions for negation of an attotime value
	friend constexpr attotime operator-(attotime const &left) noexcept;

	// functions for subtraction of two attotimes
	constexpr attotime &operator-=(attotime const &right) noexcept;
	friend constexpr attotime operator-(attotime const &left, attotime const &right) noexcept;

	// functions for multiplication of attotime by an integral factor
	template<typename T> constexpr std::enable_if_t<std::is_integral<T>::value, attotime> &operator*=(T factor) noexcept;
	template<typename T> friend constexpr std::enable_if_t<std::is_integral<T>::value, attotime> operator*(attotime const &left, T factor) noexcept;
	template<typename T> friend constexpr std::enable_if_t<std::is_integral<T>::value, attotime> operator*(T factor, attotime const &right) noexcept;

	// functions for division of attotime by an integral factor
	template<typename T> constexpr std::enable_if_t<std::is_integral<T>::value, attotime> &operator/=(T factor) noexcept;
	template<typename T> friend constexpr std::enable_if_t<std::is_integral<T>::value, attotime> operator/(attotime const &left, T factor) noexcept;

	// functions for division of attotime by a subseconds value
	constexpr s64 operator/=(subseconds factor) noexcept;
	friend constexpr s64 operator/(attotime const &left, subseconds factor) noexcept;

	// functions for equality comparisons
	friend constexpr bool operator==(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator!=(attotime const &left, attotime const &right) noexcept;

	// functions for less than comparisons
	friend constexpr bool operator<(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator<=(attotime const &left, attotime const &right) noexcept;

	// functions for greater than comparisons
	friend constexpr bool operator>(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator>=(attotime const &left, attotime const &right) noexcept;

private:
	// members
	u64 m_fine;
	s64 m_coarse;
};

// functions for addition of two attotimes
inline constexpr attotime operator+(const attotime &left, const attotime &right) noexcept
{
	// first add the two fine parts
	u64 fine = left.m_fine + right.m_fine;

	// if the low bits are set, one of them was never, so return never
	if (UNEXPECTED(attotime::is_never(fine)))
		return attotime::never;

	// compute the coarse portion
	s64 coarse = left.m_coarse + right.m_coarse + (fine >> 32);
	return attotime(coarse, u32(fine));
}

inline constexpr attotime &attotime::operator+=(const attotime &right) noexcept
{
	// first add the two fine parts
	u64 fine = m_fine + right.m_fine;

	// if the low bits are set, one of them was never, so return never
	if (UNEXPECTED(is_never(fine)))
		return *this = never;

	// compute the coarse portion
	m_coarse = m_coarse + right.m_coarse + (fine >> 32);
	m_fine = u32(fine);
	return *this;
}

// functions for negation of an attotime value
inline constexpr attotime operator-(const attotime &left) noexcept
{
	// check for never
	if (UNEXPECTED(left.is_never()))
		return attotime::never;

	return attotime(-left.m_coarse - (left.m_fine != 0), -left.m_fine);
}

// functions for subtraction of two attotimes
inline constexpr attotime operator-(const attotime &left, const attotime &right) noexcept
{
	// check for never
	if (UNEXPECTED(attotime::is_never(left.m_fine | right.m_fine)))
		return attotime::never;

	// first subtract the two fine parts
	s64 fine = left.m_fine - right.m_fine;

	// compute the coarse portion
	s64 coarse = left.m_coarse - right.m_coarse + (fine >> 32);
	return attotime(coarse, u32(fine));
}

inline constexpr attotime &attotime::operator-=(const attotime &right) noexcept
{
	// check for never
	if (UNEXPECTED(attotime::is_never(m_fine | right.m_fine)))
		return *this = never;

	// first subtract the two fine parts
	s64 fine = m_fine - right.m_fine;

	// compute the coarse portion
	m_coarse = m_coarse - right.m_coarse + (fine >> 32);
	m_fine = u32(fine);
	return *this;
}

// functions for multiplication of attotime by an integral factor
template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, attotime> operator*(attotime const &left, T factor) noexcept
{
	// check for never
	if (UNEXPECTED(left.is_never()))
		return attotime::never;

	if (sizeof(T) <= 4 || factor == s32(factor))
	{
		// multiply the fine part; both are 32 bits so this won't overflow
		s64 fine = s64(left.m_fine) * s64(factor);

		// compute the coarse portion and add the carry from the fine part
		s64 coarse = left.m_coarse * s64(factor) + (fine >> 32);
		return attotime(coarse, u32(fine));
	}
	else if (sizeof(T) == 8)
	{
		// multiply the 32-bit fine value by the 64-bit factor
		s64 fine_hi;
		s64 fine = mul_64x64(left.m_fine, factor, fine_hi);

		// compute the coarse portion, adding top 32 bits from fine and lower 32 bits from fine_hi
		s64 coarse = left.m_coarse * s64(factor) + (fine_hi << 32) + (fine >> 32);
		return attotime(coarse, u32(fine));
	}
}

template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, attotime> operator*(T factor, attotime const &right) noexcept
{
	// multiplication is commutative
	return operator*(right, factor);
}

template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, attotime> &attotime::operator*=(T factor) noexcept
{
	// check for never
	if (UNEXPECTED(is_never()))
		return *this;

	if (sizeof(T) <= 4 || factor == s32(factor))
	{
		// multiply the fine part; both are 32 bits so this won't overflow
		s64 fine = s64(m_fine) * s64(factor);

		// compute the coarse portion and add the carry from the fine part
		m_coarse = m_coarse * s64(factor) + (fine >> 32);
		m_fine = u32(fine);
	}
	else if (sizeof(T) == 8)
	{
		// multiply the 32-bit fine value by the 64-bit factor
		s64 fine_hi;
		s64 fine = mul_64x64(m_fine, factor, fine_hi);

		// compute the coarse portion, adding top 32 bits from fine and lower 32 bits from fine_hi
		m_coarse = m_coarse * s64(factor) + (fine_hi << 32) + (fine >> 32);
		m_fine = u32(fine);
	}
	return *this;
}

// functions for division of attotime by an integral factor
template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, attotime> operator/(const attotime &left, T factor) noexcept
{
	// check for never
	if (UNEXPECTED(left.is_never()))
		return attotime::never;

	// handle signed cases up front so the core code is unsigned-only
	if (UNEXPECTED(left.m_coarse < 0))
		return -(-left / factor);
	if (std::is_signed<T>::value && UNEXPECTED(factor < 0))
		return -(left / -factor);

	if (sizeof(T) <= 4 || factor == u32(factor))
	{
		// compute the coarse part; this is a 64/32-bit divide, with a remainder
		// that fits in 32 bits
		u32 remainder;
		u64 coarse = divu_64x32_rem(left.m_coarse, factor, remainder);

		// compute the fine part by combining the remainder with the fine part
		u64 fine = ((u64(remainder) << 32) | left.m_fine) / factor;
		return attotime(coarse, u32(fine) & ~3);
	}
	else if (sizeof(T) == 8)
	{
		// shift the dividend down into a 96-bit value and use the 128-bit divide
		u64 lower = left.m_fine | (left.m_coarse << 32);
		s64 upper = left.m_coarse >> 32;
		s64 result = div_128x64(upper, lower, factor);
		return attotime(result >> 32, u32(result) & ~3);
	}
}

template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, attotime> &attotime::operator/=(T factor) noexcept
{
	// check for never
	if (UNEXPECTED(is_never()))
		return *this;

	// handle signed cases up front so the core code is unsigned-only
	if (UNEXPECTED(m_coarse < 0))
		return *this = -(-*this / factor);
	if (std::is_signed<T>::value && UNEXPECTED(factor < 0))
		return *this = -(*this / -factor);

	if (sizeof(T) <= 4 || factor == u32(factor))
	{
		// compute the coarse part; this is a 64/32-bit divide, with a remainder
		// that fits in 32 bits
		u64 coarse = divu_64x32_rem(m_coarse, factor, remainder);

		// compute the fine part by combining the remainder with the fine part
		u64 fine = ((u64(remainder) << 32) | m_fine) / factor;
		m_coarse = coarse;
		m_fine = u32(fine) & ~3;
	}
	else if (sizeof(T) == 8)
	{
		// shift the dividend down into a 96-bit value and use the 128-bit divide
		u64 lower = m_fine | (m_coarse << 32);
		s64 upper = m_coarse >> 32;
		s64 result = div_128x64(upper, lower, factor);
		return attotime(result >> 32, u32(result) & ~3);
	}
	return *this;
}

// functions for division of attotime by a subseconds value
inline constexpr s64 operator/(const attotime &left, subseconds factor) noexcept
{
	// check for never
	if (UNEXPECTED(left.is_never()))
		return 0x7fffffffffffffffll;

	// shift the dividend down into a 94-bit value and use the 128-bit divide
	u64 lower = (left.m_fine >> 2) | (left.m_coarse << 30);
	s64 upper = left.m_coarse >> 34;
	return div_128x64(upper, lower, factor.raw());
}

inline constexpr s64 attotime::operator/=(subseconds factor) noexcept
{
	// check for never
	if (UNEXPECTED(is_never()))
		return 0x7fffffffffffffffll;

	// shift the dividend down into a 94-bit value and use the 128-bit divide
	u64 lower = (m_fine >> 2) | (m_coarse << 30);
	s64 upper = m_coarse >> 34;
	return div_128x64(upper, lower, factor.raw());
}

// functions for equality comparisons
inline constexpr bool operator==(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse == right.m_coarse && left.m_fine == right.m_fine);
}

inline constexpr bool operator!=(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse != right.m_coarse || left.m_fine != right.m_fine);
}

// functions for less than comparisons
inline constexpr bool operator<(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse < right.m_coarse || (left.m_coarse == right.m_coarse && left.m_fine < right.m_fine));
}

inline constexpr bool operator<=(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse < right.m_coarse || (left.m_coarse == right.m_coarse && left.m_fine <= right.m_fine));
}

// functions for greater than comparisons
inline constexpr bool operator>(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse > right.m_coarse || (left.m_coarse == right.m_coarse && left.m_fine > right.m_fine));
}

inline constexpr bool operator>=(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse > right.m_coarse || (left.m_coarse == right.m_coarse && left.m_fine >= right.m_fine));
}



//**************************************************************************
//  INLINE FUNCTIONS
//***************************************************************************/

inline const char *subseconds::as_string(int precision, bool dividers) const noexcept
{
	return attotime(*this).as_string(precision, dividers);
}

inline std::string subseconds::to_string(int precision, bool dividers) const
{
	return attotime(*this).to_string(precision, dividers);
}

#endif // MAME_EMU_ATTOTIME_H
