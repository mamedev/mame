// license:BSD-3-Clause
// copyright-holders:Aaron Giles,smf
/***************************************************************************

    timehelp.h

    Assorted shared functionality between timekeeping chips and RTCs.

***************************************************************************/

#pragma once

#ifndef SRC_DEVICES_MACHINE_TIMEHELP_H
#define SRC_DEVICES_MACHINE_TIMEHELP_H

class time_helper
{
public:
	static inline uint8_t make_bcd(uint8_t data)
	{
		return (((data / 10) % 10) << 4) + (data % 10);
	}

	static inline uint8_t from_bcd(uint8_t data)
	{
		return (((data >> 4) & 15) * 10) + (data & 15);
	}

	static int inc_bcd(uint8_t *data, int mask, int min, int max, bool *tens_carry = nullptr)
	{
		int bcd = (*data + 1) & mask;
		int carry = 0;

		if ((bcd & 0x0f) > 9)
		{
			if (tens_carry)
				*tens_carry = true;
			bcd &= 0xf0;
			bcd += 0x10;
		}
		else if (tens_carry)
		{
			*tens_carry = false;
		}

		if (bcd > max)
		{
			bcd = min;
			carry = 1;
		}

		*data = (*data & ~mask) | (bcd & mask);
		return carry;
	}

};

#endif // SRC_DEVICES_MACHINE_TIMEHELP_H
