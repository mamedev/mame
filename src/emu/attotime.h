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

class subseconds
{
	// save_manager needs direct access for save/restore purposes
	friend class save_manager;

public:
	// core definitions
	static constexpr u64 PER_SECOND_SQRT = 1'000'000'000;
	static constexpr u64 PER_SECOND = PER_SECOND_SQRT * PER_SECOND_SQRT;
	static constexpr u64 PER_MILLISECOND = PER_SECOND / 1'000;
	static constexpr u64 PER_MICROSECOND = PER_SECOND / 1'000'000;
	static constexpr u64 PER_NANOSECOND = PER_SECOND / 1'000'000'000;

	static constexpr u64 MAX_RAW = PER_SECOND;
//	static constexpr subseconds MAX = subseconds(MAX_RAW);

	constexpr subseconds() :
		m_subseconds(0)
	{
	}

	subseconds(subseconds const &src) :
		m_subseconds(src.m_subseconds)
	{
	}

	subseconds &operator=(subseconds const &src)
	{
		m_subseconds = src.m_subseconds;
		return *this;
	}

	// math
	subseconds &operator+=(subseconds const &right) noexcept
	{
		m_subseconds += right.m_subseconds;
		return *this;
	}

	subseconds &operator-=(subseconds const &right) noexcept
	{
		m_subseconds -= right.m_subseconds;
		return *this;
	}

	subseconds &operator*=(u32 factor) noexcept
	{
		m_subseconds *= factor;
		return *this;
	}

	subseconds &operator/=(u32 factor)
	{
		m_subseconds /= factor;
		return *this;
	}

	constexpr bool is_zero() const { return (m_subseconds == 0); }
	constexpr u64 raw() const { return m_subseconds; }

	// constants
	static constexpr subseconds max() { return subseconds::from_raw(PER_SECOND); }
	static constexpr subseconds min() { return subseconds::from_raw(1); }
	static constexpr subseconds zero() { return subseconds::from_raw(0); }

	// static creators
	static constexpr subseconds from_raw(u64 raw) { return subseconds(raw); }
	static constexpr subseconds from_double(double secs) { return u64(secs * double(PER_SECOND)); }
	static constexpr subseconds from_msec(u64 msec) { return subseconds(msec * PER_MILLISECOND); }
	static constexpr subseconds from_usec(u64 usec) { return subseconds(usec * PER_MICROSECOND); }
	static constexpr subseconds from_nsec(u64 nsec) { return subseconds(nsec * PER_NANOSECOND); }
	static constexpr subseconds from_hz(double hz)
	{
		u64 const hz_int = u64(hz);
		if (hz == double(hz_int))
			return subseconds(MAX_RAW / s64(hz));
		return subseconds(u64(2e64 / hz));
	}
	static constexpr subseconds from_hz(u32 hz) { return subseconds((hz > 1) ? (MAX_RAW / hz) : MAX_RAW); }
	static constexpr subseconds from_hz(u64 hz) { return subseconds((hz > 1) ? (MAX_RAW / hz) : MAX_RAW); }
	static constexpr subseconds from_hz(int hz) { return subseconds((hz > 1) ? (MAX_RAW / hz) : MAX_RAW); }
	static subseconds from_hz(XTAL const &xtal) { return from_hz(xtal.dvalue()); }

	// conversion helpers
	constexpr double as_hz() const { return double(PER_SECOND) / double(m_subseconds); }
	constexpr double as_double() const { return m_subseconds * (1.0 / double(PER_SECOND)); }
	constexpr double as_msec() const { return m_subseconds * (1.0 / double(PER_MILLISECOND)); }
	constexpr double as_usec() const { return m_subseconds * (1.0 / double(PER_MICROSECOND)); }
	constexpr double as_nsec() const { return m_subseconds * (1.0 / double(PER_NANOSECOND)); }
	constexpr u32 as_msec_int() const { return m_subseconds / PER_MILLISECOND; }
	constexpr u32 as_usec_int() const { return m_subseconds / PER_MICROSECOND; }
	constexpr u32 as_nsec_int() const { return m_subseconds / PER_NANOSECOND; }

	// friend functions
	friend subseconds operator+(subseconds const &left, subseconds const &right) noexcept;
	friend subseconds operator-(subseconds const &left, subseconds const &right) noexcept;
	friend subseconds operator*(subseconds const &left, u32 factor) noexcept;
	friend subseconds operator*(u32 factor, subseconds const &right) noexcept;
	friend u64 operator/(subseconds const &left, subseconds const &right);
	friend subseconds operator/(subseconds const &left, u32 factor);
	friend constexpr bool operator==(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator!=(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator<(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator<=(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator>(subseconds const &left, subseconds const &right) noexcept;
	friend constexpr bool operator>=(subseconds const &left, subseconds const &right) noexcept;

private:
	constexpr subseconds(u64 raw) : m_subseconds(raw) { }

	u64 m_subseconds;
};

inline subseconds operator+(subseconds const &left, subseconds const &right) noexcept
{ return subseconds(left.m_subseconds + right.m_subseconds); }
inline subseconds operator-(subseconds const &left, subseconds const &right) noexcept
{ return subseconds(left.m_subseconds - right.m_subseconds); }
inline subseconds operator*(subseconds const &left, u32 factor) noexcept
{ return subseconds(left.m_subseconds * factor); }
inline subseconds operator*(u32 factor, subseconds const &right) noexcept
{ return subseconds(factor * right.m_subseconds); }
inline u64 operator/(subseconds const &left, subseconds const &right)
{ return left.m_subseconds / right.m_subseconds; }
inline subseconds operator/(subseconds const &left, u32 factor)
{ return subseconds(left.m_subseconds / factor); }
inline constexpr bool operator==(subseconds const &left, subseconds const &right) noexcept
{ return left.m_subseconds == right.m_subseconds; }
inline constexpr bool operator!=(subseconds const &left, subseconds const &right) noexcept
{ return left.m_subseconds != right.m_subseconds; }
inline constexpr bool operator<(subseconds const &left, subseconds const &right) noexcept
{ return left.m_subseconds < right.m_subseconds; }
inline constexpr bool operator<=(subseconds const &left, subseconds const &right) noexcept
{ return left.m_subseconds <= right.m_subseconds; }
inline constexpr bool operator>(subseconds const &left, subseconds const &right) noexcept
{ return left.m_subseconds > right.m_subseconds; }
inline constexpr bool operator>=(subseconds const &left, subseconds const &right) noexcept
{ return left.m_subseconds >= right.m_subseconds; }


// the attotime structure itself
class attotime
{
	// save_manager needs direct access for save/restore purposes
	friend class save_manager;

public:
	static constexpr s32 MAX_SECONDS = 1'000'000'000;

	// construction/destruction
	constexpr attotime() noexcept : m_subseconds(0), m_seconds(0) { }

	/** Constructs with @p secs seconds and @p subs subseconds. */
	constexpr attotime(s32 secs, subseconds subs) noexcept : m_subseconds(subs.raw()), m_seconds(secs) { }
	constexpr attotime(s32 secs) noexcept : m_subseconds(0), m_seconds(secs) { }
	constexpr attotime(subseconds subs) noexcept : m_subseconds(subs.raw()), m_seconds(0) { }

	constexpr attotime(const attotime& that) noexcept : m_subseconds(that.m_subseconds), m_seconds(that.m_seconds) { }

	// assignment
	attotime &operator=(const attotime& that) noexcept
	{
		this->m_seconds = that.m_seconds;
		this->m_subseconds = that.m_subseconds;
		return *this;
	}

	// queries
	constexpr bool is_zero() const noexcept { return (m_seconds == 0 && m_subseconds == 0); }
	/** Test if value is above @ref MAX_SECONDS (considered an overflow) */
	constexpr bool is_never() const noexcept { return (m_seconds >= MAX_SECONDS); }

	// conversion to other forms
	constexpr double as_double() const noexcept { return double(m_seconds) + double(m_subseconds) * 1e-18; }
	constexpr subseconds as_subseconds() const noexcept;
	double as_hz() const noexcept { assert(!is_zero()); return m_seconds == 0 ? (double(subseconds::PER_SECOND) / double(m_subseconds)) : is_never() ? 0.0 : 1.0 / as_double(); }
	double as_khz() const noexcept { assert(!is_zero()); return m_seconds == 0 ? double(subseconds::PER_MILLISECOND) / double(m_subseconds) : is_never() ? 0.0 : 1e-3 / as_double(); }
	double as_mhz() const noexcept { assert(!is_zero()); return m_seconds == 0 ? double(subseconds::PER_MICROSECOND) / double(m_subseconds) : is_never() ? 0.0 : 1e-6 / as_double(); }
	u64 as_ticks(u32 frequency) const;
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
	constexpr subseconds raw_subseconds() const noexcept { return subseconds::from_raw(m_subseconds); }
	/** @return the seconds portion. */
	constexpr s32 seconds() const noexcept { return m_seconds; }

	void set_seconds(s32 seconds) { m_seconds = seconds;}
	void set_subseconds(subseconds subseconds) { m_subseconds = subseconds.raw(); }

	static attotime from_double(double _time);
	static attotime from_ticks(u64 ticks, u32 frequency);
	static attotime from_ticks(u64 ticks, const XTAL &xtal) { return from_ticks(ticks, xtal.value()); }
	/** Create an attotime from a integer count of seconds @seconds */
	static constexpr attotime from_seconds(s32 seconds) { return attotime(seconds); }
	/** Create an attotime from a integer count of milliseconds @msec */
	static constexpr attotime from_msec(s64 msec) { return attotime(msec / 1000, subseconds::from_raw((msec % 1000) * (subseconds::PER_SECOND / 1000))); }
	/** Create an attotime from a integer count of microseconds @usec */
	static constexpr attotime from_usec(s64 usec) { return attotime(usec / 1000000, subseconds::from_raw((usec % 1000000) * (subseconds::PER_SECOND / 1000000))); }
	/** Create an attotime from a integer count of nanoseconds @nsec */
	static constexpr attotime from_nsec(s64 nsec) { return attotime(nsec / 1000000000, subseconds::from_raw((nsec % 1000000000) * (subseconds::PER_SECOND / 1000000000))); }
	/** Create an attotime from at the given frequency @frequency */
	static constexpr attotime from_hz(u32 frequency) { return (frequency > 1) ? attotime(0, subseconds::from_raw(s64(subseconds::PER_SECOND / frequency))) : (frequency == 1) ? attotime::from_seconds(1) : attotime::never; }
	static constexpr attotime from_hz(int frequency) { return (frequency > 0) ? from_hz(u32(frequency)) : attotime::never; }
	static attotime from_hz(const XTAL &xtal) { return (xtal.dvalue() > 1.0) ? attotime(0, subseconds::from_raw(s64(subseconds::PER_SECOND / xtal))) : from_hz(xtal.dvalue()); }
	static attotime from_hz(double frequency)
	{
		if (frequency > 1.0)
			return attotime(0, subseconds::from_raw(s64(subseconds::PER_SECOND / frequency)));
		else if (frequency > 0.0)
		{
			double i, f = modf(1.0 / frequency, &i);
			return attotime(i, subseconds::from_raw(f * subseconds::PER_SECOND));
		}
		else
			return attotime::never;
	}

	// math
	attotime &operator+=(const attotime &right) noexcept;
	attotime &operator-=(const attotime &right) noexcept;
	attotime &operator*=(u32 factor);
	attotime &operator/=(u32 factor);

	// constants
	static const attotime never;
	static const attotime zero;

	// friend functions
	friend attotime operator+(const attotime &left, const attotime &right) noexcept;
	friend attotime operator-(const attotime &left, const attotime &right) noexcept;
	friend attotime operator*(const attotime &left, u32 factor);
	friend attotime operator*(u32 factor, const attotime &right);
	friend attotime operator/(const attotime &left, u32 factor);
	friend constexpr bool operator==(const attotime &left, const attotime &right) noexcept;
	friend constexpr bool operator!=(const attotime &left, const attotime &right) noexcept;
	friend constexpr bool operator<(const attotime &left, const attotime &right) noexcept;
	friend constexpr bool operator<=(const attotime &left, const attotime &right) noexcept;
	friend constexpr bool operator>(const attotime &left, const attotime &right) noexcept;
	friend constexpr bool operator>=(const attotime &left, const attotime &right) noexcept;

private:
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
		u64 count = delta.seconds() * m_rate + delta.raw_subseconds() / m_period;
		return m_tick_base + ((m_divider == 1) ? count : (count / m_divider));
	}

	attotime tick_time(u64 ticknum)
	{
		// can't get time of ticks prior to the last rate switch
		if (ticknum < m_tick_base)
			return attotime::zero;
		ticknum = (ticknum - m_tick_base) * m_divider;

		u64 seconds = ticknum / m_rate;
		attotime delta(seconds, subseconds::from_raw((ticknum - seconds * m_rate) * m_period));
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
	return subseconds::from_raw(
			(m_seconds == 0) ? m_subseconds :                  // positive values between 0 and 1 second
			(m_seconds < 0) ? 0 :                               // negative values between -1 and 0 seconds
			subseconds::MAX_RAW);                                // out-of-range negative values
}


/** as_ticks - convert to ticks at @p frequency */
inline u64 attotime::as_ticks(u32 frequency) const
{
	u32 fracticks = (attotime(subseconds::from_raw(m_subseconds)) * frequency).m_seconds;
	return mulu_32x32(m_seconds, frequency) + fracticks;
}


/** Create an attotime from a tick count @ticks at the given frequency @frequency  */
inline attotime attotime::from_ticks(u64 ticks, u32 frequency)
{
	if (frequency > 0)
	{
		s64 attos_per_tick = s64(subseconds::PER_SECOND / frequency);

		if (ticks < frequency)
			return attotime(subseconds::from_raw(ticks * attos_per_tick));

		u32 remainder;
		s32 secs = divu_64x32_rem(ticks, frequency, remainder);
		return attotime(secs, subseconds::from_raw(u64(remainder) * attos_per_tick));
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


#endif // MAME_EMU_ATTOTIME_H
