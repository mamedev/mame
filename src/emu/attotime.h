// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    attotime.h

    Support functions for working with attotime data.

****************************************************************************

    Attotime is an attosecond-accurate timing system implemented as
    96-bit integers.

        1 second      = 1e0 seconds
        1 millisecond = 1e-3 seconds
        1 microsecond = 1e-6 seconds
        1 nanosecond  = 1e-9 seconds
        1 picosecond  = 1e-12 seconds
        1 femtosecond = 1e-15 seconds
        1 attosecond  = 1e-18 seconds

    This may seem insanely accurate, but it has its uses when multiple
    clocks in the system are run by independent crystals. It is also
    useful to compute the attotime for something small, say 1 clock tick,
    and still have it be accurate and useful for scaling.

    Attotime consists of a 32-bit seconds count and a 64-bit attoseconds
    count. Because the lower bits are kept as attoseconds and not as a
    full 64-bit value, there is headroom to make some operations simpler.

***************************************************************************/

#pragma once

#ifndef __ATTOTIME_H__
#define __ATTOTIME_H__

#include <math.h>
#undef min
#undef max

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// core components of the attotime structure
typedef INT64 attoseconds_t;
typedef INT32 seconds_t;

// core definitions
const attoseconds_t ATTOSECONDS_PER_SECOND_SQRT = 1000000000;
const attoseconds_t ATTOSECONDS_PER_SECOND = ATTOSECONDS_PER_SECOND_SQRT * ATTOSECONDS_PER_SECOND_SQRT;
const attoseconds_t ATTOSECONDS_PER_MILLISECOND = ATTOSECONDS_PER_SECOND / 1000;
const attoseconds_t ATTOSECONDS_PER_MICROSECOND = ATTOSECONDS_PER_SECOND / 1000000;
const attoseconds_t ATTOSECONDS_PER_NANOSECOND = ATTOSECONDS_PER_SECOND / 1000000000;

const seconds_t ATTOTIME_MAX_SECONDS = 1000000000;



//**************************************************************************
//  MACROS
//**************************************************************************

// convert between a double and attoseconds
#define ATTOSECONDS_TO_DOUBLE(x)        ((double)(x) * 1e-18)
#define DOUBLE_TO_ATTOSECONDS(x)        ((attoseconds_t)((x) * 1e18))

// convert between hertz (as a double) and attoseconds
#define ATTOSECONDS_TO_HZ(x)            ((double)ATTOSECONDS_PER_SECOND / (double)(x))
#define HZ_TO_ATTOSECONDS(x)            ((attoseconds_t)(ATTOSECONDS_PER_SECOND / (x)))

// macros for converting other seconds types to attoseconds
#define ATTOSECONDS_IN_SEC(x)           ((attoseconds_t)(x) * ATTOSECONDS_PER_SECOND)
#define ATTOSECONDS_IN_MSEC(x)          ((attoseconds_t)(x) * ATTOSECONDS_PER_MILLISECOND)
#define ATTOSECONDS_IN_USEC(x)          ((attoseconds_t)(x) * ATTOSECONDS_PER_MICROSECOND)
#define ATTOSECONDS_IN_NSEC(x)          ((attoseconds_t)(x) * ATTOSECONDS_PER_NANOSECOND)



//**************************************************************************
//  TYPE DEFINITIONS
//***************************************************************************/

// the attotime structure itself
class attotime
{
public:
	// construction/destruction
	attotime()
		: m_seconds(0),
			m_attoseconds(0) { }

	attotime(seconds_t secs, attoseconds_t attos)
		: m_seconds(secs),
			m_attoseconds(attos) { }

	attotime(const attotime& that)
		: m_seconds(that.m_seconds),
			m_attoseconds(that.m_attoseconds) { }

	// assignment
	attotime& operator=(const attotime& that)
	{
		this->m_seconds = that.m_seconds;
		this->m_attoseconds = that.m_attoseconds;
		return *this;
	}

	// queries
	bool is_zero() const { return (m_seconds == 0 && m_attoseconds == 0); }
	bool is_never() const { return (m_seconds >= ATTOTIME_MAX_SECONDS); }

	// conversion to other forms
	double as_double() const { return double(m_seconds) + ATTOSECONDS_TO_DOUBLE(m_attoseconds); }
	attoseconds_t as_attoseconds() const;
	UINT64 as_ticks(UINT32 frequency) const;
	const char *as_string(int precision = 9) const;

	// Needed by gba.c FIXME: this shouldn't be necessary?

	void normalize()
	{
			while (m_attoseconds >= ATTOSECONDS_PER_SECOND)
			{
				m_seconds++;
				m_attoseconds -= ATTOSECONDS_PER_SECOND;
			}
	}

	attoseconds_t attoseconds() const { return m_attoseconds; }
	seconds_t seconds() const { return m_seconds; }

	// conversion from other forms
	static attotime from_double(double _time);
	static attotime from_ticks(UINT64 ticks, UINT32 frequency);
	static attotime from_seconds(INT32 seconds) { return attotime(seconds, 0); }
	static attotime from_msec(INT64 msec) { return attotime(msec / 1000, (msec % 1000) * (ATTOSECONDS_PER_SECOND / 1000)); }
	static attotime from_usec(INT64 usec) { return attotime(usec / 1000000, (usec % 1000000) * (ATTOSECONDS_PER_SECOND / 1000000)); }
	static attotime from_nsec(INT64 nsec) { return attotime(nsec / 1000000000, (nsec % 1000000000) * (ATTOSECONDS_PER_SECOND / 1000000000)); }
	static attotime from_hz(double frequency) { assert(frequency > 0); double d = 1 / frequency; return attotime(floor(d), modf(d, &d) * ATTOSECONDS_PER_SECOND); }

	// math
	attotime &operator+=(const attotime &right);
	attotime &operator-=(const attotime &right);
	attotime &operator*=(UINT32 factor);
	attotime &operator/=(UINT32 factor);

	// members
	seconds_t       m_seconds;
	attoseconds_t   m_attoseconds;

	// constants
	static const attotime never;
	static const attotime zero;
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  operator+ - handle addition between two
//  attotimes
//-------------------------------------------------

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


//-------------------------------------------------
//  operator- - handle subtraction between two
//  attotimes
//-------------------------------------------------

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


//-------------------------------------------------
//  operator* - handle multiplication/division by
//  an integral factor; defined in terms of the
//  assignment operators
//-------------------------------------------------

inline attotime operator*(const attotime &left, UINT32 factor)
{
	attotime result = left;
	result *= factor;
	return result;
}

inline attotime operator*(UINT32 factor, const attotime &right)
{
	attotime result = right;
	result *= factor;
	return result;
}

inline attotime operator/(const attotime &left, UINT32 factor)
{
	attotime result = left;
	result /= factor;
	return result;
}


//-------------------------------------------------
//  operator== - handle comparisons between
//  attotimes
//-------------------------------------------------

inline bool operator==(const attotime &left, const attotime &right)
{
	return (left.m_seconds == right.m_seconds && left.m_attoseconds == right.m_attoseconds);
}

inline bool operator!=(const attotime &left, const attotime &right)
{
	return (left.m_seconds != right.m_seconds || left.m_attoseconds != right.m_attoseconds);
}

inline bool operator<(const attotime &left, const attotime &right)
{
	return (left.m_seconds < right.m_seconds || (left.m_seconds == right.m_seconds && left.m_attoseconds < right.m_attoseconds));
}

inline bool operator<=(const attotime &left, const attotime &right)
{
	return (left.m_seconds < right.m_seconds || (left.m_seconds == right.m_seconds && left.m_attoseconds <= right.m_attoseconds));
}

inline bool operator>(const attotime &left, const attotime &right)
{
	return (left.m_seconds > right.m_seconds || (left.m_seconds == right.m_seconds && left.m_attoseconds > right.m_attoseconds));
}

inline bool operator>=(const attotime &left, const attotime &right)
{
	return (left.m_seconds > right.m_seconds || (left.m_seconds == right.m_seconds && left.m_attoseconds >= right.m_attoseconds));
}


//-------------------------------------------------
//  min - return the minimum of two attotimes
//-------------------------------------------------

inline attotime min(const attotime &left, const attotime &right)
{
	if (left.m_seconds > right.m_seconds)
		return right;
	if (left.m_seconds < right.m_seconds)
		return left;
	if (left.m_attoseconds > right.m_attoseconds)
		return right;
	return left;
}


//-------------------------------------------------
//  max - return the maximum of two attotimes
//-------------------------------------------------

inline attotime max(const attotime &left, const attotime &right)
{
	if (left.m_seconds > right.m_seconds)
		return left;
	if (left.m_seconds < right.m_seconds)
		return right;
	if (left.m_attoseconds > right.m_attoseconds)
		return left;
	return right;
}


//-------------------------------------------------
//  as_attoseconds - convert to an attoseconds
//  value, clamping to +/- 1 second
//-------------------------------------------------

inline attoseconds_t attotime::as_attoseconds() const
{
	// positive values between 0 and 1 second
	if (m_seconds == 0)
		return m_attoseconds;

	// negative values between -1 and 0 seconds
	else if (m_seconds == -1)
		return m_attoseconds - ATTOSECONDS_PER_SECOND;

	// out-of-range positive values
	else if (m_seconds > 0)
		return ATTOSECONDS_PER_SECOND;

	// out-of-range negative values
	else
		return -ATTOSECONDS_PER_SECOND;
}


//-------------------------------------------------
//  as_ticks - convert to ticks at the given
//  frequency
//-------------------------------------------------

inline UINT64 attotime::as_ticks(UINT32 frequency) const
{
	UINT32 fracticks = (attotime(0, m_attoseconds) * frequency).m_seconds;
	return mulu_32x32(m_seconds, frequency) + fracticks;
}


//-------------------------------------------------
//  from_ticks - create an attotime from a tick
//  count at the given frequency
//-------------------------------------------------

inline attotime attotime::from_ticks(UINT64 ticks, UINT32 frequency)
{
	attoseconds_t attos_per_tick = HZ_TO_ATTOSECONDS(frequency);

	if (ticks < frequency)
		return attotime(0, ticks * attos_per_tick);

	UINT32 remainder;
	INT32 secs = divu_64x32_rem(ticks, frequency, &remainder);
	return attotime(secs, (UINT64)remainder * attos_per_tick);
}


//-------------------------------------------------
//  from_double - create an attotime from floating
//  point count of seconds
//-------------------------------------------------

inline attotime attotime::from_double(double _time)
{
	seconds_t secs = floor(_time);
	_time -= double(secs);
	attoseconds_t attos = DOUBLE_TO_ATTOSECONDS(_time);
	return attotime(secs, attos);
}


#endif  // __ATTOTIME_H__
