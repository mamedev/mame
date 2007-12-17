/***************************************************************************

    coreutil.c

    Miscellaneous utility code

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************/

#include "coreutil.h"


/***************************************************************************
    BINARY CODED DECIMAL HELPERS
***************************************************************************/

int bcd_adjust(int value)
{
	if ((value & 0xf) >= 0xa)
		value = value + 0x10 - 0xa;
	if ((value & 0xf0) >= 0xa0)
		value = value - 0xa0 + 0x100;
	return value;
}


UINT32 dec_2_bcd(UINT32 a)
{
	UINT32 result = 0;
	int shift = 0;

	while (a != 0)
	{
		result |= (a % 10) << shift;
		a /= 10;
		shift += 4;
	}
	return result;
}


UINT32 bcd_2_dec(UINT32 a)
{
	UINT32 result = 0;
	UINT32 scale = 1;

	while (a != 0)
	{
		result += (a & 0x0f) * scale;
		a >>= 4;
		scale *= 10;
	}
	return result;
}



/***************************************************************************
    GREGORIAN CALENDAR HELPERS
***************************************************************************/

int gregorian_is_leap_year(int year)
{
	return !(year % 100 ? year % 4 : year % 400);
}


/* months are one counted */
int gregorian_days_in_month(int month, int year)
{
	if (month == 2)
		return gregorian_is_leap_year(year) ? 29 : 28;
	else if (month == 4 || month == 6 || month == 9 || month == 11)
		return 30;
	else
		return 31;
}


/***************************************************************************
    MISC
***************************************************************************/

void rand_memory(void *memory, size_t length)
{
	UINT8 *bytes = (UINT8 *) memory;
	UINT32 currand;
	size_t i;

	currand = rand();
	currand <<= 15;
	currand ^= rand();

	for (i = 0; i < length; i++)
	{
		currand = currand * 214013 + 2531011;
		bytes[i] = (UINT8) (currand >> 16);
	}
}

