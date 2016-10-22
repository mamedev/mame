// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    coreutil.c

    Miscellaneous utility code

***************************************************************************/

#include "coreutil.h"
#include <assert.h>
#include <zlib.h>


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


uint32_t dec_2_bcd(uint32_t a)
{
	uint32_t result = 0;
	int shift = 0;

	while (a != 0)
	{
		result |= (a % 10) << shift;
		a /= 10;
		shift += 4;
	}
	return result;
}


uint32_t bcd_2_dec(uint32_t a)
{
	uint32_t result = 0;
	uint32_t scale = 1;

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
	return !((year % 100) ? (year % 4) : (year % 400));
}


/* months are one counted */

/**
 * @fn  int gregorian_days_in_month(int month, int year)
 *
 * @brief   Gregorian days in month.
 *
 * @param   month   The month.
 * @param   year    The year.
 *
 * @return  An int.
 */

int gregorian_days_in_month(int month, int year)
{
	assert(month >= 1 && month <= 12);

	int days[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	days[1] += gregorian_is_leap_year(year) ? 1 : 0;
	return days[month-1];
}


/***************************************************************************
    MISC
***************************************************************************/

/**
 * @fn  void rand_memory(void *memory, size_t length)
 *
 * @brief   Random memory.
 *
 * @param [in,out]  memory  If non-null, the memory.
 * @param   length          The length.
 */

void rand_memory(void *memory, size_t length)
{
	static uint32_t seed = 0;
	uint8_t *bytes = (uint8_t *) memory;
	size_t i;

	for (i = 0; i < length; i++)
	{
		seed = seed * 214013 + 2531011;
		bytes[i] = (uint8_t) (seed >> 16);
	}
}


uint32_t core_crc32(uint32_t crc, const uint8_t *buf, uint32_t len)
{
	return crc32(crc, buf, len);
}
