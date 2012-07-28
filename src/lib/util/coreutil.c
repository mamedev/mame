/***************************************************************************

    coreutil.c

    Miscellaneous utility code

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

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
	return !((year % 100) ? (year % 4) : (year % 400));
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
	static UINT32 seed = 0;
	UINT8 *bytes = (UINT8 *) memory;
	size_t i;

	for (i = 0; i < length; i++)
	{
		seed = seed * 214013 + 2531011;
		bytes[i] = (UINT8) (seed >> 16);
	}
}
