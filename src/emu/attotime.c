/***************************************************************************

    attotime.c

    Support functions for working with attotime data.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emucore.h"
#include "eminline.h"
#include "attotime.h"


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

const attotime		attotime_zero = STATIC_ATTOTIME_IN_SEC(0);
const attotime		attotime_never = STATIC_ATTOTIME_IN_SEC(ATTOTIME_MAX_SECONDS);



/***************************************************************************
    CORE MATH FUNCTIONS
***************************************************************************/

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



/***************************************************************************
    MISC UTILITIES
***************************************************************************/

/*-------------------------------------------------
    attotime_string - return a temporary
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
		int temp = precision;
		while (temp < 9)
		{
			upper /= 10;
			temp++;
		}
		sprintf(buffer, "%d.%0*d", _time.seconds, precision, upper);
	}

	/* case 3: more than 9 digits of precision */
	else
	{
		UINT32 lower;
		UINT32 upper = divu_64x32_rem(_time.attoseconds, ATTOSECONDS_PER_SECOND_SQRT, &lower);
		int temp = precision;
		while (temp < 18)
		{
			lower /= 10;
			temp++;
		}
		sprintf(buffer, "%d.%09d%0*d", _time.seconds, upper, precision - 9, lower);
	}
	return buffer;
}
