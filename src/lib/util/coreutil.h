/***************************************************************************

    coreutil.h

    Miscellaneous utility code

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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


#endif /* __COREUTIL_H__ */
