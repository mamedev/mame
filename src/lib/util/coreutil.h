// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    coreutil.h

    Miscellaneous utility code

***************************************************************************/

#pragma once

#ifndef __COREUTIL_H__
#define __COREUTIL_H__

#include "osdcomm.h"


/***************************************************************************
    BINARY CODED DECIMAL HELPERS
***************************************************************************/

int bcd_adjust(int value);
UINT32 dec_2_bcd(UINT32 a);
UINT32 bcd_2_dec(UINT32 a);


/***************************************************************************
    GREGORIAN CALENDAR HELPERS
***************************************************************************/

int gregorian_is_leap_year(int year);
int gregorian_days_in_month(int month, int year);


/***************************************************************************
    MISC
***************************************************************************/

void rand_memory(void *memory, size_t length);

UINT32 core_crc32(UINT32 crc, const UINT8 *buf, UINT32 len);

#endif /* __COREUTIL_H__ */
