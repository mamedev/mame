/***************************************************************************

    attotime.c

    Support functions for working with attotime data.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "mamecore.h"
#include "attotime.h"
#include "eminline.h"


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

const attotime 		attotime_zero = STATIC_ATTOTIME_IN_SEC(0);
const attotime 		attotime_never = STATIC_ATTOTIME_IN_SEC(ATTOTIME_MAX_SECONDS);



/***************************************************************************
    CONVERSION HELPERS
***************************************************************************/

/*-------------------------------------------------
    attotime_to_attoseconds - convert a attotime
    to attoseconds, clamping to maximum positive/
    negative values
-------------------------------------------------*/

attoseconds_t attotime_to_attoseconds(attotime _time)
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



/***************************************************************************
    CORE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    attotime_add - add two attotimes
-------------------------------------------------*/

attotime attotime_add(attotime _time1, attotime _time2)
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

attotime attotime_add_attoseconds(attotime _time1, attoseconds_t _attoseconds)
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

attotime attotime_sub(attotime _time1, attotime _time2)
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

attotime attotime_sub_attoseconds(attotime _time1, attoseconds_t _attoseconds)
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
    attotime_mul - multiply an attotime by
    a constant
-------------------------------------------------*/

attotime attotime_mul(attotime _time1, UINT32 factor)
{
	UINT32 attolo, attohi, reslo, reshi;
	UINT64 temp;

	/* if one of the items is attotime_never, return attotime_never */
	if (_time1.seconds >= ATTOTIME_MAX_SECONDS)
		return attotime_never;

	/* 0 times anything is zero */
	if (factor == 0)
		return attotime_zero;

	/* split attoseconds into upper and lower halves which fit into 32 bits */
	attohi = divu_64x32_rem(_time1.attoseconds, ATTOSECONDS_PER_SECOND_SQRT, &attolo);

	/* scale the lower half, then split into high/low parts */
	temp = mulu_32x32(attolo, factor);
	temp = divu_64x32_rem(temp, ATTOSECONDS_PER_SECOND_SQRT, &reslo);

	/* scale the upper half, then split into high/low parts */
	temp += mulu_32x32(attohi, factor);
	temp = divu_64x32_rem(temp, ATTOSECONDS_PER_SECOND_SQRT, &reshi);

	/* scale the seconds */
	temp += mulu_32x32(_time1.seconds, factor);
	if (temp >= ATTOTIME_MAX_SECONDS)
		return attotime_never;

	/* build the result */
	return attotime_make(temp, (attoseconds_t)reslo + mul_32x32(reshi, ATTOSECONDS_PER_SECOND_SQRT));
}


/*-------------------------------------------------
    attotime_div - divide an attotime by
    a constant
-------------------------------------------------*/

attotime attotime_div(attotime _time1, UINT32 factor)
{
	UINT32 attolo, attohi, reshi, reslo, remainder;
	attotime result;
	UINT64 temp;

	/* if one of the items is attotime_never, return attotime_never */
	if (_time1.seconds >= ATTOTIME_MAX_SECONDS)
		return attotime_never;

	/* ignore divide by zero */
	if (factor == 0)
		return _time1;

	/* split attoseconds into upper and lower halves which fit into 32 bits */
	attohi = divu_64x32_rem(_time1.attoseconds, ATTOSECONDS_PER_SECOND_SQRT, &attolo);

	/* divide the seconds and get the remainder */
	result.seconds = divu_64x32_rem(_time1.seconds, factor, &remainder);

	/* combine the upper half of attoseconds with the remainder and divide that */
	temp = (INT64)attohi + mulu_32x32(remainder, ATTOSECONDS_PER_SECOND_SQRT);
	reshi = divu_64x32_rem(temp, factor, &remainder);

	/* combine the lower half of attoseconds with the remainder and divide that */
	temp = attolo + mulu_32x32(remainder, ATTOSECONDS_PER_SECOND_SQRT);
	reslo = divu_64x32_rem(temp, factor, &remainder);

	/* round based on the remainder */
	result.attoseconds = (attoseconds_t)reslo + mulu_32x32(reshi, ATTOSECONDS_PER_SECOND_SQRT);
	if (remainder >= factor / 2)
		if (++result.attoseconds >= ATTOSECONDS_PER_SECOND)
		{
			result.attoseconds = 0;
			result.seconds++;
		}
	return result;
}


/*-------------------------------------------------
    attotime_compare - compare two attotimes
-------------------------------------------------*/

int attotime_compare(attotime _time1, attotime _time2)
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

attotime attotime_min(attotime _time1, attotime _time2)
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

attotime attotime_max(attotime _time1, attotime _time2)
{
	if (_time1.seconds > _time2.seconds)
		return _time1;
	if (_time1.seconds < _time2.seconds)
		return _time2;
	if (_time1.attoseconds > _time2.attoseconds)
		return _time1;
	return _time2;
}



/***************************************************************************
    MISC UTILITIES
***************************************************************************/

/*-------------------------------------------------
    attotime_compare - return a temporary
    printable string describing an attotime
-------------------------------------------------*/

const char *attotime_string(attotime _time, int precision)
{
	static char buffers[8][30];
	static int nextbuf;
	char *buffer = &buffers[nextbuf++ % 8][0];

	/* case 1: we want no precision; seconds only */
	if (precision == 0)
		sprintf(buffer, "%d", _time.seconds);

	/* case 2: we want 9 or fewer digits of precision */
	else if (precision <= 9)
	{
		UINT32 upper = _time.attoseconds / ATTOSECONDS_PER_SECOND_SQRT;
		sprintf(buffer, "%d.%0*d", _time.seconds, precision, upper);
	}

	/* case 3: more than 9 digits of precision */
	else
	{
		UINT32 lower;
		UINT32 upper = divu_64x32_rem(_time.attoseconds, ATTOSECONDS_PER_SECOND_SQRT, &lower);
		sprintf(buffer, "%d.%09d%0*d", _time.seconds, upper, precision - 9, lower);
	}
	return buffer;
}
