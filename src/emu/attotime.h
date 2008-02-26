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

/* ----- conversion helpers ----- */

/* convert an attotime to attoseconds, clamping to maximum positive/negative values */
attoseconds_t attotime_to_attoseconds(attotime _time);


/* ----- core math functions ----- */

/* add two attotimes */
attotime attotime_add(attotime _time1, attotime _time2);

/* add attoseconds to an attotime */
attotime attotime_add_attoseconds(attotime _time1, attoseconds_t _attoseconds);

/* subtract two attotimes */
attotime attotime_sub(attotime _time1, attotime _time2);

/* subtract attoseconds from an attotime */
attotime attotime_sub_attoseconds(attotime _time1, attoseconds_t _attoseconds);

/* multiply an attotime by a constant */
attotime attotime_mul(attotime _time1, UINT32 factor);

/* divide an attotime by a constant */
attotime attotime_div(attotime _time1, UINT32 factor);

/* compare two attotimes */
int attotime_compare(attotime _time1, attotime _time2);

/* return the minimum of two attotimes */
attotime attotime_min(attotime _time1, attotime _time2);

/* return the maximum of two attotimes */
attotime attotime_max(attotime _time1, attotime _time2);


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


#endif	/* __ATTOTIME_H__ */
