// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    coreutil.h

    Miscellaneous utility code

***************************************************************************/

#ifndef MAME_UTIL_COREUTIL_H
#define MAME_UTIL_COREUTIL_H

#pragma once

#include "osdcomm.h"


/***************************************************************************
    BINARY CODED DECIMAL HELPERS
***************************************************************************/

int bcd_adjust(int value);
uint32_t dec_2_bcd(uint32_t a);
uint32_t bcd_2_dec(uint32_t a);


/***************************************************************************
    GREGORIAN CALENDAR HELPERS
***************************************************************************/

constexpr bool gregorian_is_leap_year(int year)
{
	return !((year % 100) ? (year % 4) : (year % 400));
}



//-------------------------------------------------
//  gregorian_days_in_month - given a year and a one-counted
//  month, return the amount of days in that month
//-------------------------------------------------

inline int gregorian_days_in_month(int month, int year)
{
	int result;
	switch (month)
	{
	case 4: case 6:
	case 9: case 11:
		// Thirty days have September, April, June, and November.
		result = 30;
		break;

	case 1: case 3:
	case 5: case 7:
	case 8: case 10:
	case 12:
		// All the rest have Thirty One
		result = 31;
		break;

	case 2:
		// No exceptions, but save one:  Twenty Eight hath February
		// in fine, and each leap year Twenty Nine
		result = gregorian_is_leap_year(year) ? 29 : 28;
		break;

	default:
		throw false;
	}
	return result;
}


/***************************************************************************
    MISC
***************************************************************************/

uint32_t core_crc32(uint32_t crc, const uint8_t *buf, uint32_t len);

#endif // MAME_UTIL_COREUTIL_H
