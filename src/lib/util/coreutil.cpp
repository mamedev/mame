// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    coreutil.c

    Miscellaneous utility code

***************************************************************************/

#include "coreutil.h"
#include <cassert>


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
