/***************************************************************************

    attotime.h

    Support functions for working with attotime data.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#include "mamecore.h"
#include "eminline.h"
#include <math.h>


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ATTOSECONDS_PER_SECOND_SQRT		((attoseconds_t)1000000000)
#define ATTOSECONDS_PER_SECOND			(ATTOSECONDS_PER_SECOND_SQRT * ATTOSECONDS_PER_SECOND_SQRT)
#define ATTOSECONDS_PER_MILLISECOND		(ATTOSECONDS_PER_SECOND / 1000)
#define ATTOSECONDS_PER_MICROSECOND		(ATTOSECONDS_PER_SECOND / 1000000)
#define ATTOSECONDS_PER_NANOSECOND		(ATTOSECONDS_PER_SECOND / 1000000000)

#define ATTOTIME_MAX_SECONDS			((seconds_t)1000000000)



/***************************************************************************
    MACROS
***************************************************************************/

/* convert between a double and attoseconds */
#define ATTOSECONDS_TO_DOUBLE(x)		((double)(x) * 1e-18)
#define DOUBLE_TO_ATTOSECONDS(x)		((attoseconds_t)((x) * 1e18))

/* convert between hertz (as a double) and attoseconds */
#define ATTOSECONDS_TO_HZ(x)			((double)ATTOSECONDS_PER_SECOND / (double)(x))
#define HZ_TO_ATTOSECONDS(x)			((attoseconds_t)(ATTOSECONDS_PER_SECOND / (x)))

/* macros for converting other seconds types to attoseconds */
#define ATTOSECONDS_IN_SEC(x)			((attoseconds_t)(x) * ATTOSECONDS_PER_SECOND)
#define ATTOSECONDS_IN_MSEC(x)			((attoseconds_t)(x) * ATTOSECONDS_PER_MILLISECOND)
#define ATTOSECONDS_IN_USEC(x)			((attoseconds_t)(x) * ATTOSECONDS_PER_MICROSECOND)
#define ATTOSECONDS_IN_NSEC(x)			((attoseconds_t)(x) * ATTOSECONDS_PER_NANOSECOND)

/* macros for building attotimes from other types at runtime */
#define ATTOTIME_IN_HZ(hz)				attotime_make((0), (HZ_TO_ATTOSECONDS(hz)))
#define ATTOTIME_IN_SEC(s)				attotime_make(((s) / 1), ((s) % 1))
#define ATTOTIME_IN_MSEC(ms)			attotime_make(((ms) / 1000), (ATTOSECONDS_IN_MSEC((ms) % 1000)))
#define ATTOTIME_IN_USEC(us)			attotime_make(((us) / 1000000), (ATTOSECONDS_IN_USEC((us) % 1000000)))
#define ATTOTIME_IN_NSEC(ns)			attotime_make(((ns) / 1000000000), (ATTOSECONDS_IN_NSEC((ns) % 1000000000)))

/* macros for building attotimes from other types at compile time */
#define STATIC_ATTOTIME_IN_HZ(hz)		{ (0), (HZ_TO_ATTOSECONDS(hz)) }
#define STATIC_ATTOTIME_IN_SEC(s)		{ ((s) / 1), ((s) % 1) }
#define STATIC_ATTOTIME_IN_MSEC(ms)		{ ((ms) / 1000), (ATTOSECONDS_IN_MSEC((ms) % 1000)) }
#define STATIC_ATTOTIME_IN_USEC(us)		{ ((us) / 1000000), (ATTOSECONDS_IN_USEC((us) % 1000000)) }
#define STATIC_ATTOTIME_IN_NSEC(ns)		{ ((ns) / 1000000000), (ATTOSECONDS_IN_NSEC((ns) % 1000000000)) }

/* macros for building a reduced-resolution attotime for tokenized storage in a UINT64 */
/* this form supports up to 1000 seconds and sacrifices 1/1000 of the full attotime resolution */
#define UINT64_ATTOTIME_IN_HZ(hz)		((UINT64)((ATTOSECONDS_PER_SECOND / 1000) / (hz)))
#define UINT64_ATTOTIME_IN_SEC(s)		((UINT64)(s) * (ATTOSECONDS_PER_SECOND / 1000))
#define UINT64_ATTOTIME_IN_MSEC(ms)		((UINT64)(ms) * (ATTOSECONDS_PER_SECOND / 1000 / 1000))
#define UINT64_ATTOTIME_IN_USEC(us)		((UINT64)(us) * (ATTOSECONDS_PER_SECOND / 1000 / 1000 / 1000))
#define UINT64_ATTOTIME_IN_NSEC(ns)		((UINT64)(ns) * (ATTOSECONDS_PER_SECOND / 1000 / 1000 / 1000 / 1000))

/* macros for converting a UINT64 attotime to a full attotime */
#define UINT64_ATTOTIME_TO_ATTOTIME(v)	attotime_make((v) / (ATTOSECONDS_PER_SECOND / 1000), ((v) % (ATTOSECONDS_PER_SECOND / 1000)) * 1000)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* core components of the attotime structure */
typedef INT64 attoseconds_t;
typedef INT32 seconds_t;


/* the attotime structure itself */
typedef struct _attotime attotime;
struct _attotime
{
	seconds_t		seconds;
	attoseconds_t	attoseconds;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* constant times for zero and never */
extern const attotime 		attotime_zero;
extern const attotime 		attotime_never;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- core math functions ----- */

/* multiply an attotime by a constant */
attotime attotime_mul(attotime _time1, UINT32 factor);

/* divide an attotime by a constant */
attotime attotime_div(attotime _time1, UINT32 factor);


/* ----- misc utilities ----- */

/* return a temporary printable string describing an attotime */
const char *attotime_string(attotime _time, int precision);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    attotime_make - assemble an attotime from
    seconds and attoseconds components
-------------------------------------------------*/

INLINE attotime attotime_make(seconds_t _secs, attoseconds_t _subsecs)
{
	attotime result;
	result.seconds = _secs;
	result.attoseconds = _subsecs;
	return result;
}


/*-------------------------------------------------
    attotime_to_double - convert from attotime
    to double
-------------------------------------------------*/

INLINE double attotime_to_double(attotime _time)
{
	return (double)_time.seconds + ATTOSECONDS_TO_DOUBLE(_time.attoseconds);
}


/*-------------------------------------------------
    double_to_attotime - convert from double to
    attotime
-------------------------------------------------*/

INLINE attotime double_to_attotime(double _time)
{
	attotime result;

	/* set seconds to the integral part */
	result.seconds = floor(_time);

	/* set attoseconds to the fractional part */
	_time -= (double)result.seconds;
	result.attoseconds = DOUBLE_TO_ATTOSECONDS(_time);
	return result;
}


/*-------------------------------------------------
    attotime_to_attoseconds - convert a attotime
    to attoseconds, clamping to maximum positive/
    negative values
-------------------------------------------------*/

INLINE attoseconds_t attotime_to_attoseconds(attotime _time)
{
	/* positive values between 0 and 1 second */
	if (_time.seconds == 0)
		return _time.attoseconds;

	/* negative values between -1 and 0 seconds */
	else if (_time.seconds == -1)
		return _time.attoseconds - ATTOSECONDS_PER_SECOND;

	/* out-of-range positive values */
	else if (_time.seconds > 0)
		return ATTOSECONDS_PER_SECOND;

	/* out-of-range negative values */
	else
		return -ATTOSECONDS_PER_SECOND;
}


/*-------------------------------------------------
    attotime_to_ticks - convert an attotime to
    clock ticks at the given frequency
-------------------------------------------------*/

INLINE UINT64 attotime_to_ticks(attotime _time, UINT32 frequency)
{
	UINT32 fracticks = attotime_mul(attotime_make(0, _time.attoseconds), frequency).seconds;
	return mulu_32x32(_time.seconds, frequency) + fracticks;
}


/*-------------------------------------------------
    ticks_to_attotime - convert clock ticks at
    the given frequency to an attotime
-------------------------------------------------*/

INLINE attotime ticks_to_attotime(UINT64 ticks, UINT32 frequency)
{
	attoseconds_t attos_per_tick = HZ_TO_ATTOSECONDS(frequency);
	attotime result;

	if (ticks < frequency)
	{
		result.seconds = 0;
		result.attoseconds = ticks * attos_per_tick;
	}
	else
	{
		UINT32 remainder;
		result.seconds = divu_64x32_rem(ticks, frequency, &remainder);
		result.attoseconds = (UINT64)remainder * attos_per_tick;
	}
	return result;
}


/*-------------------------------------------------
    attotime_add - add two attotimes
-------------------------------------------------*/

INLINE attotime attotime_add(attotime _time1, attotime _time2)
{
	attotime result;

	/* if one of the items is attotime_never, return attotime_never */
	if (_time1.seconds >= ATTOTIME_MAX_SECONDS || _time2.seconds >= ATTOTIME_MAX_SECONDS)
		return attotime_never;

	/* add the seconds and attoseconds */
	result.attoseconds = _time1.attoseconds + _time2.attoseconds;
	result.seconds = _time1.seconds + _time2.seconds;

	/* normalize and return */
	if (result.attoseconds >= ATTOSECONDS_PER_SECOND)
	{
		result.attoseconds -= ATTOSECONDS_PER_SECOND;
		result.seconds++;
	}

	/* overflow */
	if (result.seconds >= ATTOTIME_MAX_SECONDS)
		return attotime_never;
	return result;
}


/*-------------------------------------------------
    attotime_add_attoseconds - add attoseconds
    to a attotime
-------------------------------------------------*/

INLINE attotime attotime_add_attoseconds(attotime _time1, attoseconds_t _attoseconds)
{
	attotime result;

	/* if one of the items is attotime_never, return attotime_never */
	if (_time1.seconds >= ATTOTIME_MAX_SECONDS)
		return attotime_never;

	/* add the seconds and attoseconds */
	result.attoseconds = _time1.attoseconds + _attoseconds;
	result.seconds = _time1.seconds;

	/* normalize and return */
	if (result.attoseconds >= ATTOSECONDS_PER_SECOND)
	{
		result.attoseconds -= ATTOSECONDS_PER_SECOND;
		result.seconds++;
	}

	/* overflow */
	if (result.seconds >= ATTOTIME_MAX_SECONDS)
		return attotime_never;
	return result;
}


/*-------------------------------------------------
    attotime_sub - subtract two attotimes
-------------------------------------------------*/

INLINE attotime attotime_sub(attotime _time1, attotime _time2)
{
	attotime result;

	/* if time1 is attotime_never, return attotime_never */
	if (_time1.seconds >= ATTOTIME_MAX_SECONDS)
		return attotime_never;

	/* add the seconds and attoseconds */
	result.attoseconds = _time1.attoseconds - _time2.attoseconds;
	result.seconds = _time1.seconds - _time2.seconds;

	/* normalize and return */
	if (result.attoseconds < 0)
	{
		result.attoseconds += ATTOSECONDS_PER_SECOND;
		result.seconds--;
	}
	return result;
}


/*-------------------------------------------------
    attotime_sub_attoseconds - subtract
    attoseconds from a attotime
-------------------------------------------------*/

INLINE attotime attotime_sub_attoseconds(attotime _time1, attoseconds_t _attoseconds)
{
	attotime result;

	/* if time1 is attotime_never, return attotime_never */
	if (_time1.seconds >= ATTOTIME_MAX_SECONDS)
		return attotime_never;

	/* add the seconds and attoseconds */
	result.attoseconds = _time1.attoseconds - _attoseconds;
	result.seconds = _time1.seconds;

	/* normalize and return */
	if (result.attoseconds < 0)
	{
		result.attoseconds += ATTOSECONDS_PER_SECOND;
		result.seconds--;
	}
	return result;
}


/*-------------------------------------------------
    attotime_compare - compare two attotimes
-------------------------------------------------*/

INLINE int attotime_compare(attotime _time1, attotime _time2)
{
	if (_time1.seconds > _time2.seconds)
		return 1;
	if (_time1.seconds < _time2.seconds)
		return -1;
	if (_time1.attoseconds > _time2.attoseconds)
		return 1;
	if (_time1.attoseconds < _time2.attoseconds)
		return -1;
	return 0;
}


/*-------------------------------------------------
    attotime_min - return the minimum of two
    attotimes
-------------------------------------------------*/

INLINE attotime attotime_min(attotime _time1, attotime _time2)
{
	if (_time1.seconds > _time2.seconds)
		return _time2;
	if (_time1.seconds < _time2.seconds)
		return _time1;
	if (_time1.attoseconds > _time2.attoseconds)
		return _time2;
	return _time1;
}


/*-------------------------------------------------
    attotime_max - return the maximum of two
    attotimes
-------------------------------------------------*/

INLINE attotime attotime_max(attotime _time1, attotime _time2)
{
	if (_time1.seconds > _time2.seconds)
		return _time1;
	if (_time1.seconds < _time2.seconds)
		return _time2;
	if (_time1.attoseconds > _time2.attoseconds)
		return _time1;
	return _time2;
}


/*-------------------------------------------------
    attotime_is_never - return whether or not an
    attotime is attotime_never
-------------------------------------------------*/

INLINE int attotime_is_never(attotime _time)
{
	return (_time.seconds >= ATTOTIME_MAX_SECONDS);
}


#endif	/* __ATTOTIME_H__ */
