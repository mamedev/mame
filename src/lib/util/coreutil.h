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
uint32_t dec_2_bcd(uint32_t a);
uint32_t bcd_2_dec(uint32_t a);


/***************************************************************************
    GREGORIAN CALENDAR HELPERS
***************************************************************************/

int gregorian_is_leap_year(int year);
int gregorian_days_in_month(int month, int year);


/***************************************************************************
    MISC
***************************************************************************/

void rand_memory(void *memory, size_t length);

uint32_t core_crc32(uint32_t crc, const uint8_t *buf, uint32_t len);

#endif /* __COREUTIL_H__ */
