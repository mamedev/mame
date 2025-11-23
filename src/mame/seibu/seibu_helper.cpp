// license:BSD-3-Clause
// copyright-holders:Ville Linde, Nicola Salmoria
/*
    Helper for common decryption method in Seibu Kaihatsu hardwares.
*/

#include "emu.h"
#include "seibu_helper.h"

// add two numbers generating carry from one bit to the next only if
// the corresponding bit in carry_mask is 1
u32 seibu_partial_carry_sum(u32 add1, u32 add2, u32 carry_mask, int bits)
{
	int res = 0;
	int carry = 0;
	for (int i = 0; i < bits; i++)
	{
		const int bit = BIT(add1, i) + BIT(add2, i) + carry;

		res += BIT(bit, 0) << i;

		// generate carry only if the corresponding bit in carry_mask is 1
		if (BIT(carry_mask, i))
			carry = bit >> 1;
		else
			carry = 0;
	}

	// wrap around carry from top bit to bit 0
	if (carry)
		res ^= 1;

	return res;
}

u32 seibu_partial_carry_sum32(u32 add1, u32 add2, u32 carry_mask)
{
	return seibu_partial_carry_sum(add1, add2, carry_mask, 32);
}

u32 seibu_partial_carry_sum24(u32 add1, u32 add2, u32 carry_mask)
{
	return seibu_partial_carry_sum(add1, add2, carry_mask, 24);
}
