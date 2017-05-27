// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**************************************************************************/
/**
 * @file attotime.h
 * Support functions for working with attotime data.
 * @defgroup ATTOTIME
 * @{
 * Support functions for working with attotime data.
 *
 * @class attotime
 *  Attotime is an attosecond-accurate timing system implemented as
 *  96-bit integers.
 *
 *     1 second      = 1e0 seconds
 *     1 millisecond = 1e-3 seconds
 *     1 microsecond = 1e-6 seconds
 *     1 nanosecond  = 1e-9 seconds
 *     1 picosecond  = 1e-12 seconds
 *     1 femtosecond = 1e-15 seconds
 *     1 attosecond  = 1e-18 seconds
 *
 * This may seem insanely accurate, but it has its uses when multiple
 * clocks in the system are run by independent crystals. It is also
 * useful to compute the attotime for something small, say 1 clock tick,
 * and still have it be accurate and useful for scaling.
 *
 * Attotime consists of a 32-bit seconds count and a 64-bit attoseconds
 * count. Because the lower bits are kept as attoseconds and not as a
 * full 64-bit value, there is headroom to make some operations simpler.
 */
/**************************************************************************/
#ifndef MAME_EMU_ATTOTIME_H
#define MAME_EMU_ATTOTIME_H

#pragma once

#include <math.h>
#undef min
#undef max

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// core components of the attotime structure
typedef s64 attoseconds_t;
typedef s32 seconds_t;

// core definitions
const attoseconds_t ATTOSECONDS_PER_SECOND_SQRT = 1'000'000'000;
const attoseconds_t ATTOSECONDS_PER_SECOND = ATTOSECONDS_PER_SECOND_SQRT * ATTOSECONDS_PER_SECOND_SQRT;
const attoseconds_t ATTOSECONDS_PER_MILLISECOND = ATTOSECONDS_PER_SECOND / 1'000;
const attoseconds_t ATTOSECONDS_PER_MICROSECOND = ATTOSECONDS_PER_SECOND / 1'000'000;
const attoseconds_t ATTOSECONDS_PER_NANOSECOND = ATTOSECONDS_PER_SECOND / 1'000'000'000;

const seconds_t ATTOTIME_MAX_SECONDS = 1'000'000'000;



//**************************************************************************
//  MACROS
//**************************************************************************

// convert between a double and attoseconds
inline constexpr double ATTOSECONDS_TO_DOUBLE(attoseconds_t x) { return double(x) * 1e-18; }
inline constexpr attoseconds_t DOUBLE_TO_ATTOSECONDS(double x) { return attoseconds_t(x * 1e18); }

// convert between hertz (as a double) and attoseconds
inline constexpr double ATTOSECONDS_TO_HZ(attoseconds_t x) { return double(ATTOSECONDS_PER_SECOND) / double(x); }
template <typename T> inline constexpr attoseconds_t HZ_TO_ATTOSECONDS(T &&x) { return attoseconds_t(ATTOSECONDS_PER_SECOND / x); }

// macros for converting other seconds types to attoseconds
template <typename T> inline constexpr attoseconds_t ATTOSECONDS_IN_SEC(T &&x) { return attoseconds_t(x) * ATTOSECONDS_PER_SECOND; }
template <typename T> inline constexpr attoseconds_t ATTOSECONDS_IN_MSEC(T &&x) { return attoseconds_t(x) * ATTOSECONDS_PER_MILLISECOND; }
template <typename T> inline constexpr attoseconds_t ATTOSECONDS_IN_USEC(T &&x) { return attoseconds_t(x) * ATTOSECONDS_PER_MICROSECOND; }
template <typename T> inline constexpr attoseconds_t ATTOSECONDS_IN_NSEC(T &&x) { return attoseconds_t(x) * ATTOSECONDS_PER_NANOSECOND; }



//**************************************************************************
//  TYPE DEFINITIONS
//***************************************************************************/

// the attotime structure itself
class attotime
{
public:
	// construction/destruction
	constexpr attotime() : m_seconds(0), m_attoseconds(0) { }

	/** Constructs with @p secs seconds and @p attos attoseconds. */
	constexpr attotime(seconds_t secs, attoseconds_t attos) : m_seconds(secs), m_attoseconds(attos) { }

	constexpr attotime(const attotime& that) : m_seconds(that.m_seconds), m_attoseconds(that.m_attoseconds) { }

	// assignment
	attotime& operator=(const attotime& that)
	{
		this->m_seconds = that.m_seconds;
		this->m_attoseconds = that.m_attoseconds;
		return *this;
	}

	// queries
	constexpr bool is_zero() const { return (m_seconds == 0 && m_attoseconds == 0); }
	/** Test if value is above @ref ATTOTIME_MAX_SECONDS (considered an overflow) */
	constexpr bool is_never() const { return (m_seconds >= ATTOTIME_MAX_SECONDS); }

	// conversion to other forms
	constexpr double as_double() const { return double(m_seconds) + ATTOSECONDS_TO_DOUBLE(m_attoseconds); }
	constexpr attoseconds_t as_attoseconds() const;
	u64 as_ticks(u32 frequency) const;
	/** Convert to string using at @p precision */
	const char *as_string(int precision = 9) const;

	/** @return the attoseconds portion. */
	constexpr attoseconds_t attoseconds() const { return m_attoseconds; }
	/** @return the seconds portion. */
	constexpr seconds_t seconds() const { return m_seconds; }

	static attotime from_double(double _time);
	static attotime from_ticks(u64 ticks, u32 frequency);
	/** Create an attotime from a integer count of seconds @seconds */
	static constexpr attotime from_seconds(s32 seconds) { return attotime(seconds, 0); }
	/** Create an attotime from a integer count of milliseconds @msec */
	static constexpr attotime from_msec(s64 msec) { return attotime(msec / 1000, (msec % 1000) * (ATTOSECONDS_PER_SECOND / 1000)); }
	/** Create an attotime from a integer count of microseconds @usec */
	static constexpr attotime from_usec(s64 usec) { return attotime(usec / 1000000, (usec % 1000000) * (ATTOSECONDS_PER_SECOND / 1000000)); }
	/** Create an attotime from a integer count of nanoseconds @nsec */
	static constexpr attotime from_nsec(s64 nsec) { return attotime(nsec / 1000000000, (nsec % 1000000000) * (ATTOSECONDS_PER_SECOND / 1000000000)); }
	/** Create an attotime from at the given frequency @frequency */
	static attotime from_hz(double frequency) { assert(frequency > 0); double d = 1 / frequency; return attotime(floor(d), modf(d, &d) * ATTOSECONDS_PER_SECOND); }

	// math
	attotime &operator+=(const attotime &right);
	attotime &operator-=(const attotime &right);
	attotime &operator*=(u32 factor);
	attotime &operator/=(u32 factor);

	// members
	seconds_t       m_seconds;
	attoseconds_t   m_attoseconds;

	// constants
	static const attotime never;
	static const attotime zero;
};
/** @} */


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

/** handle addition between two attotimes */
inline attotime operator+(const attotime &left, const attotime &right)
{
	attotime result;

	// if one of the items is never, return never
	if (left.m_seconds >= ATTOTIME_MAX_SECONDS || right.m_seconds >= ATTOTIME_MAX_SECONDS)
		return attotime::never;

	// add the seconds and attoseconds
	result.m_attoseconds = left.m_attoseconds + right.m_attoseconds;
	result.m_seconds = left.m_seconds + right.m_seconds;

	// normalize and return
	if (result.m_attoseconds >= ATTOSECONDS_PER_SECOND)
	{
		result.m_attoseconds -= ATTOSECONDS_PER_SECOND;
		result.m_seconds++;
	}

	// overflow
	if (result.m_seconds >= ATTOTIME_MAX_SECONDS)
		return attotime::never;
	return result;
}

inline attotime &attotime::operator+=(const attotime &right)
{
	// if one of the items is never, return never
	if (this->m_seconds >= ATTOTIME_MAX_SECONDS || right.m_seconds >= ATTOTIME_MAX_SECONDS)
		return *this = never;

	// add the seconds and attoseconds
	m_attoseconds += right.m_attoseconds;
	m_seconds += right.m_seconds;

	// normalize and return
	if (this->m_attoseconds >= ATTOSECONDS_PER_SECOND)
	{
		this->m_attoseconds -= ATTOSECONDS_PER_SECOND;
		this->m_seconds++;
	}

	// overflow
	if (this->m_seconds >= ATTOTIME_MAX_SECONDS)
		return *this = never;
	return *this;
}


/** handle subtraction between two attotimes */
inline attotime operator-(const attotime &left, const attotime &right)
{
	attotime result;

	// if time1 is never, return never
	if (left.m_seconds >= ATTOTIME_MAX_SECONDS)
		return attotime::never;

	// add the seconds and attoseconds
	result.m_attoseconds = left.m_attoseconds - right.m_attoseconds;
	result.m_seconds = left.m_seconds - right.m_seconds;

	// normalize and return
	if (result.m_attoseconds < 0)
	{
		result.m_attoseconds += ATTOSECONDS_PER_SECOND;
		result.m_seconds--;
	}
	return result;
}

inline attotime &attotime::operator-=(const attotime &right)
{
	// if time1 is never, return never
	if (this->m_seconds >= ATTOTIME_MAX_SECONDS)
		return *this = never;

	// add the seconds and attoseconds
	m_attoseconds -= right.m_attoseconds;
	m_seconds -= right.m_seconds;

	// normalize and return
	if (this->m_attoseconds < 0)
	{
		this->m_attoseconds += ATTOSECONDS_PER_SECOND;
		this->m_seconds--;
	}
	return *this;
}


/** handle multiplication by an integral factor; defined in terms of the assignment operators */
inline attotime operator*(const attotime &left, u32 factor)
{
	attotime result = left;
	result *= factor;
	return result;
}

inline attotime operator*(u32 factor, const attotime &right)
{
	attotime result = right;
	result *= factor;
	return result;
}

/** handle division by an integral factor; defined in terms of the assignment operators */
inline attotime operator/(const attotime &left, u32 factor)
{
	attotime result = left;
	result /= factor;
	return result;
}


/** handle comparisons between attotimes */
inline constexpr bool operator==(const attotime &left, const attotime &right)
{
	return (left.m_seconds == right.m_seconds && left.m_attoseconds == right.m_attoseconds);
}

inline constexpr bool operator!=(const attotime &left, const attotime &right)
{
	return (left.m_seconds != right.m_seconds || left.m_attoseconds != right.m_attoseconds);
}

inline constexpr bool operator<(const attotime &left, const attotime &right)
{
	return (left.m_seconds < right.m_seconds || (left.m_seconds == right.m_seconds && left.m_attoseconds < right.m_attoseconds));
}

inline constexpr bool operator<=(const attotime &left, const attotime &right)
{
	return (left.m_seconds < right.m_seconds || (left.m_seconds == right.m_seconds && left.m_attoseconds <= right.m_attoseconds));
}

inline constexpr bool operator>(const attotime &left, const attotime &right)
{
	return (left.m_seconds > right.m_seconds || (left.m_seconds == right.m_seconds && left.m_attoseconds > right.m_attoseconds));
}

inline constexpr bool operator>=(const attotime &left, const attotime &right)
{
	return (left.m_seconds > right.m_seconds || (left.m_seconds == right.m_seconds && left.m_attoseconds >= right.m_attoseconds));
}


/** Convert to an attoseconds value, clamping to +/- 1 second */
inline constexpr attoseconds_t attotime::as_attoseconds() const
{
	return
			(m_seconds == 0) ? m_attoseconds :                              // positive values between 0 and 1 second
			(m_seconds == -1) ? (m_attoseconds - ATTOSECONDS_PER_SECOND) :  // negative values between -1 and 0 seconds
			(m_seconds > 0) ? ATTOSECONDS_PER_SECOND :                      // out-of-range positive values
			-ATTOSECONDS_PER_SECOND;                                        // out-of-range negative values
}


/** as_ticks - convert to ticks at @p frequency */
inline u64 attotime::as_ticks(u32 frequency) const
{
	u32 fracticks = (attotime(0, m_attoseconds) * frequency).m_seconds;
	return mulu_32x32(m_seconds, frequency) + fracticks;
}


/** Create an attotime from a tick count @ticks at the given frequency @frequency  */
inline attotime attotime::from_ticks(u64 ticks, u32 frequency)
{
	attoseconds_t attos_per_tick = HZ_TO_ATTOSECONDS(frequency);

	if (ticks < frequency)
		return attotime(0, ticks * attos_per_tick);

	u32 remainder;
	s32 secs = divu_64x32_rem(ticks, frequency, &remainder);
	return attotime(secs, u64(remainder) * attos_per_tick);
}

/** Create an attotime from floating point count of seconds @p _time */
inline attotime attotime::from_double(double _time)
{
	seconds_t secs = floor(_time);
	_time -= double(secs);
	attoseconds_t attos = DOUBLE_TO_ATTOSECONDS(_time);
	return attotime(secs, attos);
}


#endif  // MAME_EMU_ATTOTIME_H
