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
 * Attotime consists of a 32-bit seconds count and a 64-bit subseconds
 * count. Because the lower bits are kept as subseconds and not as a
 * full 64-bit value, there is headroom to make some operations simpler.
 */
/**************************************************************************/
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

namespace oldtime
{

// ======================> subseconds

// subseconds is a class that represents a fully-accurate value from -1 to +2 seconds
class subseconds
{
	// save_manager needs direct access for save/restore purposes
	friend class save_manager;

	// a divide that rounds up
	static constexpr s64 divide_up(s64 dividend, s64 divisor)
	{
		return (dividend - 1) / divisor + 1;
	}

public:
	// core definitions
	static constexpr s64 PER_SECOND_SQRT = 1'000'000'000;
	static constexpr s64 PER_SECOND = PER_SECOND_SQRT * PER_SECOND_SQRT;
	static constexpr s64 PER_MILLISECOND = PER_SECOND / 1'000;
	static constexpr s64 PER_MICROSECOND = PER_SECOND / 1'000'000;
	static constexpr s64 PER_NANOSECOND = PER_SECOND / 1'000'000'000;

	// maximum raw value
	static constexpr s64 MAX_RAW = PER_SECOND + (PER_SECOND - 1);
	static constexpr s64 MIN_RAW = -2 * PER_SECOND;

	// constructor
	constexpr subseconds() :
		m_subseconds(0)
	{
	}

	// copy constructor
	subseconds(subseconds const &src) :
		m_subseconds(src.m_subseconds)
	{
	}

	// copy assignment
	subseconds &operator=(subseconds const &src)
	{
		m_subseconds = src.m_subseconds;
		return *this;
	}

	// add to another subseconds value
	subseconds &operator+=(subseconds const &right) noexcept
	{
		m_subseconds += right.m_subseconds;
		return *this;
	}

	// subtract another subseconds value from us
	subseconds &operator-=(subseconds const &right) noexcept
	{
		m_subseconds -= right.m_subseconds;
		return *this;
	}

	// multiply by an integral factor
	template<typename T>
	std::enable_if_t<std::is_integral<T>::value, subseconds> &operator*=(T factor) noexcept
	{
		m_subseconds *= s64(factor);
		return *this;
	}

	// divide by an integral factor
	template<typename T>
	std::enable_if_t<std::is_integral<T>::value, subseconds> &operator/=(T factor)
	{
		m_subseconds /= s64(factor);
		return *this;
	}

	// simple getters
	constexpr bool is_zero() const { return (m_subseconds == 0); }
	constexpr s64 raw() const { return m_subseconds; }

	// constant values
	static constexpr subseconds max() { return subseconds::from_raw(MAX_RAW); }
	static constexpr subseconds min() { return subseconds::from_raw(MIN_RAW); }
	static constexpr subseconds unit() { return subseconds::from_raw(1); }
	static constexpr subseconds zero() { return subseconds::from_raw(0); }

	// static creators
	static constexpr subseconds from_raw(s64 raw) { return subseconds(raw); }
	static constexpr subseconds from_double(double secs) { return subseconds(s64(secs * double(PER_SECOND))); }
	static constexpr subseconds from_msec(s32 msec) { return subseconds(s64(msec) * PER_MILLISECOND); }
	static constexpr subseconds from_usec(s32 usec) { return subseconds(s64(usec) * PER_MICROSECOND); }
	static constexpr subseconds from_nsec(s32 nsec) { return subseconds(s64(nsec) * PER_NANOSECOND); }
	static constexpr subseconds from_hz(double hz)
	{
		s64 const hz_int = s64(hz);
		if (hz == double(hz_int))
			return subseconds(divide_up(PER_SECOND, hz_int));
		return subseconds(s64(double(PER_SECOND) / hz) + 1);
	}
	static constexpr subseconds from_hz(u32 hz) { return subseconds((hz > 0) ? divide_up(PER_SECOND, hz) : MAX_RAW); }
	static constexpr subseconds from_hz(u64 hz) { return subseconds((hz > 0) ? divide_up(PER_SECOND, hz) : MAX_RAW); }
	static constexpr subseconds from_hz(int hz) { return subseconds((hz > 0) ? divide_up(PER_SECOND, hz) : MAX_RAW); }
	static subseconds from_hz(XTAL const &xtal) { return from_hz(xtal.dvalue()); }

	// conversion helpers
	constexpr double as_hz() const { return double(PER_SECOND) / double(m_subseconds); }
	constexpr double as_double() const { return m_subseconds * (1.0 / double(PER_SECOND)); }
	constexpr double as_msec() const { return m_subseconds * (1.0 / double(PER_MILLISECOND)); }
	constexpr double as_usec() const { return m_subseconds * (1.0 / double(PER_MICROSECOND)); }
	constexpr double as_nsec() const { return m_subseconds * (1.0 / double(PER_NANOSECOND)); }
	constexpr s32 as_msec_int() const { return m_subseconds / PER_MILLISECOND; }
	constexpr s32 as_usec_int() const { return m_subseconds / PER_MICROSECOND; }
	constexpr s32 as_nsec_int() const { return m_subseconds / PER_NANOSECOND; }

	// friend functions
	friend subseconds operator+(subseconds const &left, subseconds const &right) noexcept;
	friend subseconds operator-(subseconds const &left, subseconds const &right) noexcept;
	friend subseconds operator-(subseconds const &left) noexcept;
	template<typename T> friend std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(subseconds const &left, T factor) noexcept;
	template<typename T> friend std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(T factor, subseconds const &right) noexcept;
	friend s64 operator/(subseconds const &left, subseconds const &right);
	template<typename T> friend std::enable_if_t<std::is_integral<T>::value, subseconds> operator/(subseconds const &left, T factor);
	friend constexpr bool operator==(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator!=(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator<(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator<=(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator>(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator>=(subseconds const &left, subseconds const &right) noexcept;

private:
	// internal constructor from raw
	constexpr subseconds(s64 raw) :
		m_subseconds(raw)
	{
	}

	// internal state
	s64 m_subseconds;
};

// addition of two subseconds
inline subseconds operator+(subseconds const &left, subseconds const &right) noexcept
{
	return subseconds(left.m_subseconds + right.m_subseconds);
}

// negation of subseconds
inline subseconds operator-(subseconds const &left) noexcept
{
	return subseconds(-left.m_subseconds);
}

// subtactions of two subseconds
inline subseconds operator-(subseconds const &left, subseconds const &right) noexcept
{
	return subseconds(left.m_subseconds - right.m_subseconds);
}

// multiplication of a subseconds value by an integral factor
template<typename T>
inline std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(subseconds const &left, T factor) noexcept
{
	return subseconds(left.m_subseconds * s64(factor));
}
template<typename T>
inline std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(T factor, subseconds const &right) noexcept
{
	return subseconds(s64(factor) * right.m_subseconds);
}

// division of a subseconds value by another subseconds value, producing a raw integer
inline s64 operator/(subseconds const &left, subseconds const &right)
{
	return left.m_subseconds / right.m_subseconds;
}

// division of a subseconds value by an integral factor
template<typename T>
inline std::enable_if_t<std::is_integral<T>::value, subseconds> operator/(subseconds const &left, T factor)
{
	return subseconds(left.m_subseconds / s64(factor));
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

// attotime is a class holds a 96-bit time value, consisting of seconds plus subseconds
class attotime
{
	// save_manager needs direct access for save/restore purposes
	friend class save_manager;

public:
	static constexpr s32 MAX_SECONDS = 1'000'000'000;

	// construction/destruction
	constexpr attotime() noexcept : m_subseconds(0), m_seconds(0) { }

	/** Constructs with @p secs seconds and @p subs subseconds. */
	constexpr attotime(s32 secs, subseconds subs) noexcept : m_subseconds(subs.raw() % subseconds::PER_SECOND), m_seconds(secs) { }
	constexpr attotime(s32 secs) noexcept : m_subseconds(0), m_seconds(secs) { }
	constexpr attotime(subseconds subs) noexcept : m_subseconds(subs.raw() % subseconds::PER_SECOND), m_seconds(subs.raw() / subseconds::PER_SECOND) { }

	constexpr attotime(attotime const& that) noexcept : m_subseconds(that.m_subseconds), m_seconds(that.m_seconds) { }

	// assignment
	attotime &operator=(attotime const& that) noexcept
	{
		this->m_seconds = that.m_seconds;
		this->m_subseconds = that.m_subseconds;
		return *this;
	}

	// queries
	constexpr bool is_zero() const noexcept { return (m_subseconds == 0 && m_seconds == 0); }
	/** Test if value is above @ref MAX_SECONDS (considered an overflow) */
	constexpr bool is_never() const noexcept { return (m_seconds >= MAX_SECONDS); }

	// conversion to other forms
	constexpr double as_double() const noexcept { return double(m_seconds) + double(m_subseconds) * 1e-18; }
	constexpr subseconds as_subseconds() const noexcept;
	double as_hz() const noexcept { assert(!is_zero()); return m_seconds == 0 ? (double(subseconds::PER_SECOND) / double(m_subseconds)) : is_never() ? 0.0 : 1.0 / as_double(); }
	double as_khz() const noexcept { assert(!is_zero()); return m_seconds == 0 ? double(subseconds::PER_MILLISECOND) / double(m_subseconds) : is_never() ? 0.0 : 1e-3 / as_double(); }
	double as_mhz() const noexcept { assert(!is_zero()); return m_seconds == 0 ? double(subseconds::PER_MICROSECOND) / double(m_subseconds) : is_never() ? 0.0 : 1e-6 / as_double(); }
	u64 as_ticks(u32 frequency) const { return as_ticks(subseconds::from_hz(frequency)); }
	u64 as_ticks(const XTAL &xtal) const { return as_ticks(xtal.value()); }
	u64 as_ticks(subseconds period) const;
	/** Convert to string using at @p precision */
	const char *as_string(int precision = 9) const;
	double as_msec() const { return m_seconds * 1e3 + subseconds::from_raw(m_subseconds).as_msec(); }
	double as_usec() const { return m_seconds * 1e6 + subseconds::from_raw(m_subseconds).as_usec(); }
	double as_nsec() const { return m_seconds * 1e9 + subseconds::from_raw(m_subseconds).as_nsec(); }
	s64 as_msec_int() const { return s64(m_seconds) * 1000ll + subseconds::from_raw(m_subseconds).as_msec_int(); }
	s64 as_usec_int() const { return s64(m_seconds) * 1000000ll + subseconds::from_raw(m_subseconds).as_usec_int(); }
	s64 as_nsec_int() const { return s64(m_seconds) * 1000000000ll + subseconds::from_raw(m_subseconds).as_nsec_int(); }

	/** Convert to string for human readability in logs */
	std::string to_string() const;

	/** @return the raw subseconds portion. */
	constexpr subseconds frac() const noexcept { return subseconds::from_raw(m_subseconds); }
	/** @return the seconds portion. */
	constexpr s32 seconds() const noexcept { return m_seconds; }

	static attotime from_double(double _time);
	static attotime from_ticks(u64 ticks, u32 frequency);
	static attotime from_ticks(u64 ticks, const XTAL &xtal) { return from_ticks(ticks, xtal.value()); }
	/** Create an attotime from a integer count of seconds @seconds */
	static constexpr attotime from_seconds(s32 seconds) { return attotime(seconds); }
	/** Create an attotime from a integer count of milliseconds @msec */
	static constexpr attotime from_msec(s64 msec) { return attotime(msec / 1000, (msec % 1000) * subseconds::PER_MILLISECOND); }
	/** Create an attotime from a integer count of microseconds @usec */
	static constexpr attotime from_usec(s64 usec) { return attotime(usec / 1000000, (usec % 1000000) * subseconds::PER_MICROSECOND); }
	/** Create an attotime from a integer count of nanoseconds @nsec */
	static constexpr attotime from_nsec(s64 nsec) { return attotime(nsec / 1000000000, (nsec % 1000000000) * subseconds::PER_NANOSECOND); }
	/** Create an attotime from at the given frequency @frequency */
	static constexpr attotime from_hz(u32 frequency) { return (frequency > 1) ? attotime(subseconds::from_hz(frequency)) : (frequency == 1) ? attotime::from_seconds(1) : attotime::never; }
	static constexpr attotime from_hz(int frequency) { return (frequency > 0) ? from_hz(u32(frequency)) : attotime::never; }
	static attotime from_hz(const XTAL &xtal) { return (xtal.dvalue() > 1.0) ? attotime(subseconds::from_hz(xtal)) : from_hz(xtal.dvalue()); }
	static attotime from_hz(double frequency)
	{
		if (frequency > 1.0)
			return attotime(subseconds::from_hz(frequency));
		else if (frequency > 0.0)
			return from_double(1.0 / frequency);
		else
			return attotime::never;
	}

	// math
	attotime &operator+=(attotime const &right) noexcept;
	attotime &operator-=(attotime const &right) noexcept;
	template<typename T> std::enable_if_t<std::is_integral<T>::value, attotime> &operator*=(T factor);
	template<typename T> std::enable_if_t<std::is_integral<T>::value && sizeof(T) < 8, attotime> &operator/=(T factor);

	// constants
	static attotime const never;
	static attotime const zero;

	// friend functions
	friend attotime operator+(attotime const &left, attotime const &right) noexcept;
	friend attotime operator-(attotime const &left, attotime const &right) noexcept;
	friend attotime operator-(attotime const &left) noexcept;
	template<typename T> friend std::enable_if_t<std::is_integral<T>::value, attotime> operator*(attotime const &left, T factor);
	template<typename T> friend std::enable_if_t<std::is_integral<T>::value, attotime> operator*(T factor, attotime const &right);
	template<typename T> friend std::enable_if_t<std::is_integral<T>::value && sizeof(T) < 8, attotime> operator/(attotime const &left, T factor);
	friend constexpr bool operator==(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator!=(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator<(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator<=(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator>(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator>=(attotime const &left, attotime const &right) noexcept;

private:
	constexpr attotime(s32 secs, s64 subs) :
		m_subseconds(subs), m_seconds(secs)
	{
	}

	// members
	s64   m_subseconds;
	s32   m_seconds;
};
/** @} */


#if 0
class precise_clock
{
public:
	precise_clock(u32 rate = 1, u32 divider = 1) :
		m_rate(rate),
		m_divider(1),
		m_period((rate == 0) ? subseconds::PER_SECOND : ((subseconds::PER_SECOND + rate - 1) / rate)),
		m_base(attotime::zero),
		m_tick_base(0)
	{
	}

	void reset_tick_base() { m_tick_base = 0; }

	void set_rate(attotime const &base, u32 rate, u32 divider = 0)
	{
		// remember the ticks at the old rate at the base time before setting it
		m_tick_base = tick(base);
		m_base = base;

		// configure the new rate
		m_rate = rate;
		if (divider != 0)
			m_divider = divider;
		m_period = (rate == 0) ? subseconds::PER_SECOND : ((subseconds::PER_SECOND + rate - 1) / rate);
	}

	u64 tick(attotime const &time)
	{
		attotime delta = time - m_base;
		u64 count = delta.seconds() * m_rate + delta.frac() / m_period;
		return m_tick_base + ((m_divider == 1) ? count : (count / m_divider));
	}

	attotime tick_time(u64 ticknum)
	{
		// can't get time of ticks prior to the last rate switch
		if (ticknum < m_tick_base)
			return attotime::zero;
		ticknum = (ticknum - m_tick_base) * m_divider;

		u64 seconds = ticknum / m_rate;
		attotime delta(seconds, (ticknum - seconds * m_rate) * m_period);
		return delta + m_base;
	}

private:
	u32 m_rate;					// integral rate in Hz
	u32 m_divider;				// clock divider (or 1 if no divider)
	s64 m_period;				// period, measured in subseconds
	attotime m_base;			// time of last rate change
	u64 m_tick_base;			// ticks at last rate change
};
#endif


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

/** handle addition between two attotimes */
inline attotime operator+(const attotime &left, const attotime &right) noexcept
{
	attotime result;

	// if one of the items is never, return never
	if (left.m_seconds >= attotime::MAX_SECONDS || right.m_seconds >= attotime::MAX_SECONDS)
		return attotime::never;

	// add the seconds and subseconds
	result.m_subseconds = left.m_subseconds + right.m_subseconds;
	result.m_seconds = left.m_seconds + right.m_seconds;

	// normalize and return
	if (result.m_subseconds >= subseconds::PER_SECOND)
	{
		result.m_subseconds -= subseconds::PER_SECOND;
		result.m_seconds++;
	}

	// overflow
	if (result.m_seconds >= attotime::MAX_SECONDS)
		return attotime::never;
	return result;
}

inline attotime &attotime::operator+=(const attotime &right) noexcept
{
	// if one of the items is never, return never
	if (this->m_seconds >= MAX_SECONDS || right.m_seconds >= MAX_SECONDS)
		return *this = never;

	// add the seconds and subseconds
	m_subseconds += right.m_subseconds;
	m_seconds += right.m_seconds;

	// normalize and return
	if (this->m_subseconds >= subseconds::PER_SECOND)
	{
		this->m_subseconds -= subseconds::PER_SECOND;
		this->m_seconds++;
	}

	// overflow
	if (this->m_seconds >= MAX_SECONDS)
		return *this = never;
	return *this;
}


/** handle subtraction between two attotimes */
inline attotime operator-(const attotime &left, const attotime &right) noexcept
{
	attotime result;

	// if time1 is never, return never
	if (left.m_seconds >= attotime::MAX_SECONDS)
		return attotime::never;

	// add the seconds and subseconds
	result.m_subseconds = left.m_subseconds - right.m_subseconds;
	result.m_seconds = left.m_seconds - right.m_seconds;

	// normalize and return
	if (result.m_subseconds < 0)
	{
		result.m_subseconds += subseconds::PER_SECOND;
		result.m_seconds--;
	}
	return result;
}

inline attotime &attotime::operator-=(const attotime &right) noexcept
{
	// if time1 is never, return never
	if (this->m_seconds >= MAX_SECONDS)
		return *this = never;

	// add the seconds and subseconds
	m_subseconds -= right.m_subseconds;
	m_seconds -= right.m_seconds;

	// normalize and return
	if (this->m_subseconds < 0)
	{
		this->m_subseconds += subseconds::PER_SECOND;
		this->m_seconds--;
	}
	return *this;
}

inline attotime operator-(const attotime &left) noexcept
{
	attotime result;

	// if time1 is never, return never
	if (left.m_seconds >= attotime::MAX_SECONDS)
		return attotime::never;

	return attotime(-left.m_seconds - (left.m_subseconds != 0), subseconds::PER_SECOND - left.m_subseconds);
}


/** handle multiplication by an integral factor; defined in terms of the assignment operators */
template<typename T>
inline std::enable_if_t<std::is_integral<T>::value, attotime> operator*(const attotime &left, T factor)
{
	attotime result = left;
	result *= factor;
	return result;
}

template<typename T>
inline std::enable_if_t<std::is_integral<T>::value, attotime> operator*(T factor, const attotime &right)
{
	attotime result = right;
	result *= factor;
	return result;
}

/** handle division by an integral factor; defined in terms of the assignment operators */
template<typename T>
inline std::enable_if_t<std::is_integral<T>::value && sizeof(T) < 8, attotime> operator/(const attotime &left, T factor)
{
	attotime result = left;
	result /= factor;
	return result;
}


/** handle comparisons between attotimes */
inline constexpr bool operator==(const attotime &left, const attotime &right) noexcept
{
	return (left.m_seconds == right.m_seconds && left.m_subseconds == right.m_subseconds);
}

inline constexpr bool operator!=(const attotime &left, const attotime &right) noexcept
{
	return (left.m_seconds != right.m_seconds || left.m_subseconds != right.m_subseconds);
}

inline constexpr bool operator<(const attotime &left, const attotime &right) noexcept
{
	return (left.m_seconds < right.m_seconds || (left.m_seconds == right.m_seconds && left.m_subseconds < right.m_subseconds));
}

inline constexpr bool operator<=(const attotime &left, const attotime &right) noexcept
{
	return (left.m_seconds < right.m_seconds || (left.m_seconds == right.m_seconds && left.m_subseconds <= right.m_subseconds));
}

inline constexpr bool operator>(const attotime &left, const attotime &right) noexcept
{
	return (left.m_seconds > right.m_seconds || (left.m_seconds == right.m_seconds && left.m_subseconds > right.m_subseconds));
}

inline constexpr bool operator>=(const attotime &left, const attotime &right) noexcept
{
	return (left.m_seconds > right.m_seconds || (left.m_seconds == right.m_seconds && left.m_subseconds >= right.m_subseconds));
}


/** Convert to an subseconds value, clamping to 0-1 second */
inline constexpr subseconds attotime::as_subseconds() const noexcept
{
	if (m_seconds >= -2 && m_seconds <= 1)
		return subseconds::from_raw(m_subseconds + subseconds::PER_SECOND * s64(m_seconds));
	else
		return (m_seconds > 0) ? subseconds::max() : subseconds::min();
}


/** Create an attotime from a tick count @ticks at the given frequency @frequency  */
inline attotime attotime::from_ticks(u64 ticks, u32 frequency)
{
	if (frequency > 0)
	{
		subseconds subs_per_tick = subseconds::from_hz(frequency);

		if (ticks < frequency)
			return attotime(ticks * subs_per_tick);

		u32 remainder;
		s32 secs = divu_64x32_rem(ticks, frequency, remainder);
		return attotime(secs, remainder * subs_per_tick);
	}
	else
		return attotime::never;
}

/** Create an attotime from floating point count of seconds @p _time */
inline attotime attotime::from_double(double _time)
{
	s32 secs = floor(_time);
	_time -= double(secs);
	s64 subs = _time * 1e18;
	return attotime(secs, subseconds::from_raw(subs));
}

}



namespace newtime
{

//
// A full time value is represented at a 94-bit integer: 32 bits of whole seconds,
// plus 62 bit of subseconds. The seconds plus top 32 bits of subseconds are held
// in the "coarse" part of the value, while the low 30 bits are held in the "fine"
// part of the value. The low two bits of 'fine' are kept at 0, except for the
// special value "never", which is identified by the lsb being set to 1.
//
// attotime represents the full 94-bit time.
// subseconds reprsents a +/-2 seconds window of time.
//

// ======================> subseconds

// subseconds is a class that represents a fully-accurate value from -2 to +2 seconds
class subseconds
{
	// save_manager needs direct access for save/restore purposes
	friend class save_manager;

	// a divide that rounds up
	static constexpr s64 divide_up(s64 dividend, s64 divisor)
	{
		return (dividend - 1) / divisor + 1;
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

	// constructor
	constexpr subseconds() :
		m_subseconds(0)
	{
	}

	// copy constructor
	subseconds(subseconds const &src) :
		m_subseconds(src.m_subseconds)
	{
	}

	// copy assignment
	subseconds &operator=(subseconds const &src)
	{
		m_subseconds = src.m_subseconds;
		return *this;
	}

	// add to another subseconds value
	subseconds &operator+=(subseconds const &right) noexcept
	{
		m_subseconds += right.m_subseconds;
		return *this;
	}

	// subtract another subseconds value from us
	subseconds &operator-=(subseconds const &right) noexcept
	{
		m_subseconds -= right.m_subseconds;
		return *this;
	}

	// multiply by an integral factor
	template<typename T>
	std::enable_if_t<std::is_integral<T>::value, subseconds> &operator*=(T factor) noexcept
	{
		m_subseconds *= s64(factor);
		return *this;
	}

	// divide by an integral factor
	template<typename T>
	std::enable_if_t<std::is_integral<T>::value, subseconds> &operator/=(T factor)
	{
		m_subseconds /= s64(factor);
		return *this;
	}

	// simple getters
	constexpr bool is_zero() const { return (m_subseconds == 0); }
	constexpr s64 raw() const { return m_subseconds; }

	// constant values
	static constexpr subseconds max() { return subseconds::from_raw(MAX_RAW); }
	static constexpr subseconds min() { return subseconds::from_raw(MIN_RAW); }
	static constexpr subseconds unit() { return subseconds::from_raw(1); }
	static constexpr subseconds zero() { return subseconds::from_raw(0); }

	// static creators
	static constexpr subseconds from_raw(s64 raw) { return subseconds(raw); }
	static constexpr subseconds from_double(double secs) { return subseconds(s64(secs * double(PER_SECOND))); }
	static constexpr subseconds from_msec(s32 msec) { return subseconds(s64(msec) * PER_MILLISECOND); }
	static constexpr subseconds from_usec(s32 usec) { return subseconds(s64(usec) * PER_MICROSECOND); }
	static constexpr subseconds from_nsec(s32 nsec) { return subseconds(s64(nsec) * PER_NANOSECOND); }
	static constexpr subseconds from_hz(double hz)
	{
		s64 const hz_int = s64(hz);
		if (hz == double(hz_int))
			return subseconds(divide_up(PER_SECOND, hz_int));
		return subseconds(s64(double(PER_SECOND) / hz) + 1);
	}
	static constexpr subseconds from_hz(u32 hz) { return subseconds((hz > 0) ? divide_up(PER_SECOND, hz) : MAX_RAW); }
	static constexpr subseconds from_hz(u64 hz) { return subseconds((hz > 0) ? divide_up(PER_SECOND, hz) : MAX_RAW); }
	static constexpr subseconds from_hz(int hz) { return subseconds((hz > 0) ? divide_up(PER_SECOND, hz) : MAX_RAW); }
	static subseconds from_hz(XTAL const &xtal) { return from_hz(xtal.dvalue()); }

	// conversion helpers
	constexpr double as_hz() const { return double(PER_SECOND) / double(m_subseconds); }
	constexpr double as_double() const { return m_subseconds * (1.0 / double(PER_SECOND)); }
	constexpr double as_msec() const { return m_subseconds * (1.0 / double(PER_MILLISECOND)); }
	constexpr double as_usec() const { return m_subseconds * (1.0 / double(PER_MICROSECOND)); }
	constexpr double as_nsec() const { return m_subseconds * (1.0 / double(PER_NANOSECOND)); }
	constexpr s32 as_msec_int() const { return m_subseconds / PER_MILLISECOND; }
	constexpr s32 as_usec_int() const { return m_subseconds / PER_MICROSECOND; }
	constexpr s32 as_nsec_int() const { return m_subseconds / PER_NANOSECOND; }

	// friend functions
	friend subseconds operator+(subseconds const &left, subseconds const &right) noexcept;
	friend subseconds operator-(subseconds const &left) noexcept;
	friend subseconds operator-(subseconds const &left, subseconds const &right) noexcept;
	template<typename T> friend std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(subseconds const &left, T factor) noexcept;
	template<typename T> friend std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(T factor, subseconds const &right) noexcept;
	friend s64 operator/(subseconds const &left, subseconds const &right);
	template<typename T> friend std::enable_if_t<std::is_integral<T>::value, subseconds> operator/(subseconds const &left, T factor);
	friend constexpr bool operator==(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator!=(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator<(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator<=(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator>(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator>=(subseconds const &left, subseconds const &right) noexcept;

private:
	// internal constructor from raw
	constexpr subseconds(s64 raw) :
		m_subseconds(raw)
	{
	}

	// internal state
	s64 m_subseconds;
};

// addition of two subseconds
inline subseconds operator+(subseconds const &left, subseconds const &right) noexcept
{
	return subseconds(left.m_subseconds + right.m_subseconds);
}

// negation of subseconds
inline subseconds operator-(subseconds const &left) noexcept
{
	return subseconds(-left.m_subseconds);
}

// subtactions of two subseconds
inline subseconds operator-(subseconds const &left, subseconds const &right) noexcept
{
	return subseconds(left.m_subseconds - right.m_subseconds);
}

// multiplication of a subseconds value by an integral factor
template<typename T>
inline std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(subseconds const &left, T factor) noexcept
{
	return subseconds(left.m_subseconds * s64(factor));
}

template<typename T>
inline std::enable_if_t<std::is_integral<T>::value, subseconds> operator*(T factor, subseconds const &right) noexcept
{
	return subseconds(s64(factor) * right.m_subseconds);
}

// division of a subseconds value by another subseconds value, producing a raw integer
inline s64 operator/(subseconds const &left, subseconds const &right)
{
	return left.m_subseconds / right.m_subseconds;
}

// division of a subseconds value by an integral factor
template<typename T>
inline std::enable_if_t<std::is_integral<T>::value, subseconds> operator/(subseconds const &left, T factor)
{
	return subseconds(left.m_subseconds / s64(factor));
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
	constexpr attotime(s32 secs) noexcept :
		m_fine(0),
		m_coarse(s64(secs) << 32)
	{
	}

	// construction from subseconds
	constexpr attotime(subseconds subs) noexcept :
		m_fine(u32(subs.raw() << 2)),
		m_coarse(subs.raw() >> 30)
	{
	}

	// construction with integral seconds plus subseconds
	constexpr attotime(s32 secs, subseconds subs) noexcept :
		m_fine(u32(subs.raw() << 2)),
		m_coarse((s64(secs) << 32) + (subs.raw() >> 30))
	{
	}

	// copy constructor
	constexpr attotime(attotime const &src) noexcept :
		m_fine(src.m_fine),
		m_coarse(src.m_coarse)
	{
	}

	// assignment
	attotime &operator=(attotime const &src) noexcept
	{
		m_coarse = src.m_coarse;
		m_fine = src.m_fine;
		return *this;
	}

	// queries
	constexpr bool is_zero() const noexcept { return ((m_coarse | m_fine) == 0); }
	constexpr bool is_never() const noexcept { return is_never(m_fine); }

	// conversion to subseconds
	constexpr subseconds as_subseconds() const noexcept
	{
		s64 value = m_coarse << 30;
		if ((value >> 30) == m_coarse)
			return subseconds::from_raw(value | (m_fine >> 2));
		return (m_coarse >= 0) ? subseconds::max() : subseconds::min();
	}

	// extraction of full seconds and fractional subseconds
	constexpr s32 seconds() const { return s32(m_coarse >> 32); }
	constexpr subseconds frac() const noexcept { return subseconds::from_raw(((m_coarse & 0xffffffffll) << 30) | (m_fine >> 2)); }

	// conversion to double
	constexpr double as_double() const noexcept { return double(m_coarse) * COARSE_FACTOR_INV + double(m_fine >> 2) * FINE_FACTOR_INV; }
	constexpr double as_msec() const { return double(m_coarse) * (COARSE_FACTOR_INV * 1e3) + double(m_fine >> 2) * (FINE_FACTOR_INV * 1e3); }
	constexpr double as_usec() const { return double(m_coarse) * (COARSE_FACTOR_INV * 1e6) + double(m_fine >> 2) * (FINE_FACTOR_INV * 1e6); }
	constexpr double as_nsec() const { return double(m_coarse) * (COARSE_FACTOR_INV * 1e9) + double(m_fine >> 2) * (FINE_FACTOR_INV * 1e9); }

	// conversion to integral values
	s64 as_msec_int() const { return as_ticks(1000); }
	s64 as_usec_int() const { return as_ticks(1000000); }
	s64 as_nsec_int() const { return as_ticks(1000000000); }

	// conversion to frequency
	double as_hz() const noexcept { assert(!is_zero()); return 1.0 / as_double(); }
	double as_khz() const noexcept { assert(!is_zero()); return 1e-3 / as_double(); }
	double as_mhz() const noexcept { assert(!is_zero()); return 1e-6 / as_double(); }

	// conversion to ticks
	s64 as_ticks(subseconds period) const { return *this / period; }
	s64 as_ticks(u32 frequency) const { return s64(seconds()) * s64(frequency) + frac() / subseconds::from_hz(frequency); }
	s64 as_ticks(const XTAL &xtal) const { return as_ticks(xtal.value()); }

	// conversion to string
	const char *as_string(int precision = 9) const;
	std::string to_string(int precision = 9) const;

	// creation from double
	static attotime from_double(double _time)
	{
		if (_time > MAX_SECONDS || _time < MIN_SECONDS)
			return never;
		double upper = floor(_time * COARSE_FACTOR);
		s64 coarse = s64(upper);
		u32 fine = u32((_time - upper) * (FINE_FACTOR / COARSE_FACTOR));
		return attotime(coarse, fine);
	}

	// creation from integral times
	static constexpr attotime from_seconds(s32 seconds) { return attotime(s64(seconds) << 32, 0); }
	static constexpr attotime from_msec(s64 msec) { return attotime(msec / 1000, subseconds::from_msec(msec % 1000)); }
	static constexpr attotime from_usec(s64 usec) { return attotime(usec / 1000000, subseconds::from_usec(usec % 1000000)); }
	static constexpr attotime from_nsec(s64 nsec) { return attotime(nsec / 1000000000, subseconds::from_nsec(nsec % 1000000000)); }

	// creation from frequency
	static constexpr attotime from_hz(u32 frequency) { return (frequency > 0) ? attotime(subseconds::from_hz(frequency)) : never; }
	static constexpr attotime from_hz(int frequency) { return (frequency > 0) ? attotime(subseconds::from_hz(frequency)) : never; }
	static attotime from_hz(const XTAL &xtal) { return (xtal.dvalue() > 1.0) ? attotime(subseconds::from_hz(xtal.dvalue())) : from_hz(xtal.dvalue()); }
	static attotime from_hz(double frequency)
	{
		if (frequency > 1.0)
			return attotime(subseconds::from_hz(frequency));
		else if (frequency > 0.0)
			return from_double(1.0 / frequency);
		else
			return never;
	}

	// creation from ticks at a given frequency
	static attotime from_ticks(u64 ticks, subseconds period);
	static attotime from_ticks(u64 ticks, u32 frequency) { return attotime(ticks / frequency, u32(ticks % frequency) * subseconds::from_hz(frequency)); }
	static attotime from_ticks(u64 ticks, const XTAL &xtal) { return from_ticks(ticks, xtal.value()); }

	// math
	constexpr attotime &operator+=(attotime const &right) noexcept;
	constexpr attotime &operator-=(attotime const &right) noexcept;
	template<typename T> constexpr std::enable_if_t<std::is_integral<T>::value, attotime> &operator*=(T factor);
	template<typename T> constexpr std::enable_if_t<std::is_integral<T>::value && sizeof(T) < 8, attotime> &operator/=(T factor);
	s64 operator/=(subseconds factor);

	// friend math
	friend constexpr attotime operator+(attotime const &left, attotime const &right) noexcept;
	friend constexpr attotime operator-(attotime const &left) noexcept;
	friend constexpr attotime operator-(attotime const &left, attotime const &right) noexcept;
	template<typename T> friend constexpr std::enable_if_t<std::is_integral<T>::value, attotime> operator*(attotime const &left, T factor);
	template<typename T> friend constexpr std::enable_if_t<std::is_integral<T>::value, attotime> operator*(T factor, attotime const &right);
	template<typename T> friend constexpr std::enable_if_t<std::is_integral<T>::value && sizeof(T) < 8, attotime> operator/(attotime const &left, T factor);
	friend s64 operator/(attotime const &left, subseconds factor);

	// friend comparisons
	friend constexpr bool operator==(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator!=(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator<(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator<=(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator>(attotime const &left, attotime const &right) noexcept;
	friend constexpr bool operator>=(attotime const &left, attotime const &right) noexcept;

private:
	// internal helper for creation
	constexpr attotime(s64 coarse, u32 fine) :
		m_fine(fine), m_coarse(coarse)
	{
	}

	// never check
	constexpr static bool is_never(u64 fineval) { return (BIT(fineval, 0, 2) != 0); }

	// members
	u64 m_fine;
	s64 m_coarse;
};

// add two attotimes
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

// negate an attotime
inline constexpr attotime operator-(const attotime &left) noexcept
{
	// check for never
	if (UNEXPECTED(left.is_never()))
		return attotime::never;

	return attotime(-left.m_coarse - (left.m_fine != 0), -left.m_fine);
}

// subtract two attotimes
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

// multiply an attotime by a 32-bit factor
template<typename T, std::enable_if_t<std::is_integral<T>::value, bool>>
inline constexpr attotime operator*(const attotime &left, T factor)
{
	// check for never
	if (UNEXPECTED(left.is_never()))
		return attotime::never;

	// multiply the fine part
	s64 fine = s64(left.m_fine) * s64(factor);

	// compute the coarse portion
	s64 coarse = left.m_coarse * s64(factor) + (fine >> 32);
	return attotime(coarse, u32(fine));
}

template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, attotime> operator*(T factor, const attotime &right)
{
	// multiplication is commutative
	return operator*(right, factor);
}

template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value, attotime> &attotime::operator*=(T factor)
{
	// check for never
	if (UNEXPECTED(is_never()))
		return *this;

	// multiply the fine part
	s64 fine = s64(m_fine) * s64(factor);

	// compute the coarse portion
	m_coarse = m_coarse * s64(factor) + (fine >> 32);
	m_fine = u32(fine);
	return *this;
}

// divide by a 32-bit factor
// TODO: Upgrade to use CPU-native 128/64 divide
template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value && sizeof(T) < 8, attotime> operator/(const attotime &left, T factor)
{
	// check for never
	if (UNEXPECTED(left.is_never()))
		return attotime::never;

	// special case the signed divide
	if (UNEXPECTED(left.m_coarse < 0))
		return -(-left / factor);
	if (std::is_signed<T>::value && UNEXPECTED(factor < 0))
		return -(left / -factor);

	// compute the coarse part
	u64 coarse = u64(left.m_coarse) / u32(factor);

	// compute the fine part by combining the remainder with the fine part
	u64 fine = (((left.m_coarse - coarse * factor) << 32) | left.m_fine) / factor;
	return attotime(coarse, u32(fine) & ~3);
}

template<typename T>
inline constexpr std::enable_if_t<std::is_integral<T>::value && sizeof(T) < 8, attotime> &attotime::operator/=(T factor)
{
	// check for never
	if (UNEXPECTED(is_never()))
		return *this;

	// special case the signed divide
	if (UNEXPECTED(m_coarse < 0))
		return *this = -(-*this / factor);
	if (std::is_signed<T>::value && UNEXPECTED(factor < 0))
		return *this = -(*this / -factor);

	// compute the coarse part
	u64 coarse = u64(m_coarse) / u32(factor);

	// compute the fine part by combining the remainder with the fine part
	u64 fine = (((m_coarse - coarse * factor) << 32) | m_fine) / factor;
	m_coarse = coarse;
	m_fine = u32(fine) & ~3;
	return *this;
}

// divide by subseconds
inline s64 operator/(const attotime &left, subseconds factor)
{
	// check for never
	if (UNEXPECTED(left.is_never()))
		return 0x7fffffffffffffffll;

	// shift the dividend down into a 94-bit value and use the 128-bit divide
	u64 lower = (left.m_fine >> 2) | (left.m_coarse << 30);
	s64 upper = left.m_coarse >> 34;
	return div_128x64(upper, lower, factor.raw());
}

inline s64 attotime::operator/=(subseconds factor)
{
	// check for never
	if (UNEXPECTED(is_never()))
		return 0x7fffffffffffffffll;

	// shift the dividend down into a 94-bit value and use the 128-bit divide
	u64 lower = (m_fine >> 2) | (m_coarse << 30);
	s64 upper = m_coarse >> 34;
	return div_128x64(upper, lower, factor.raw());
}

// equality comparisons
inline constexpr bool operator==(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse == right.m_coarse && left.m_fine == right.m_fine);
}

inline constexpr bool operator!=(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse != right.m_coarse && left.m_fine != right.m_fine);
}

// less than comparisons
inline constexpr bool operator<(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse < right.m_coarse || (left.m_coarse == right.m_coarse && left.m_fine < right.m_fine));
}

inline constexpr bool operator<=(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse < right.m_coarse || (left.m_coarse == right.m_coarse && left.m_fine <= right.m_fine));
}

inline constexpr bool operator>(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse > right.m_coarse || (left.m_coarse == right.m_coarse && left.m_fine > right.m_fine));
}

inline constexpr bool operator>=(const attotime &left, const attotime &right) noexcept
{
	return (left.m_coarse > right.m_coarse || (left.m_coarse == right.m_coarse && left.m_fine >= right.m_fine));
}

}


// select the old implementation for now
using namespace oldtime;

#endif // MAME_EMU_ATTOTIME_H
