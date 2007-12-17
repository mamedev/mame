/***************************************************************************

    World Rally

    Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
    I/O ports)

***************************************************************************/

#include "driver.h"

UINT16 *wrally_encr_table[2];

/***************************************************************************

    World Rally Video RAM encryption

***************************************************************************/

static int subxor1(int data,int res)
{
	res ^= 0x0002;
	if (BIT(data,0) ^ !BIT(data,5))
	{
		res ^= 0x0800;
		if (BIT(data,2) ^ BIT(data,3)) res ^= 0x0001;
		if (BIT(data,6) && !BIT(data,11) && BIT(data,5)) res ^= 0x0001;
	}

	return res;
}

static int subxor2(int data,int res)
{
	res ^= 0x0020;
	if (BIT(data,8) ^ !BIT(data,5))
	{
		res ^= 0x0008;
		if (BIT(data,4) ^ BIT(data,3)) res ^= 0x0100;
		if (BIT(data,6) && !BIT(data,11) && BIT(data,5)) res ^= 0x0100;
	}

	return res;
}

static int wrally_decode_vram(int data)
{
	int res = 0;


	res = BITSWAP16(data,5,7,9,12,2,14,13,15,3,6,8,11,4,10,0,1);

	res ^= 0x0062;

	if (BIT(data,9))  res ^= 0x5004;

	if ( BIT(data,5) ^ (!BIT(data,9) &&  BIT(data,7)))  res ^= 0x0200;
	if (!BIT(data,5) ^ (!BIT(data,9) &&  BIT(data,12))) res ^= 0x0400;

	if (BIT(data,3) ^ BIT(data,6))  res ^= 0x0101;

	if (BIT(data,3) ^ BIT(data,5))
	{
		res ^= 0x0808;
		if (BIT(data,2) ^ BIT(data,3)) res ^= 0x0001;
		if (BIT(data,4) ^ BIT(data,3)) res ^= 0x0100;
	}

	if ((BIT(data,9) && !BIT(data,10)) || BIT(data,5))
	{
		res ^= 0x0010;

		if (!BIT(data,11)) res ^= 0x0141;

		if (BIT(data,6) && !BIT(data,11))
		{
			res ^= 0x0888;
			if (!BIT(data,2)) res ^= 0x0001;
			if (!BIT(data,4)) res ^= 0x0100;
		}
	}

	if (BIT(data,9) && !BIT(data,10))
	{
		res ^= 0x8600;
		if (!BIT(data,14)) res = subxor1(data,res);
		if (!BIT(data,5))  res = subxor2(data,res);

		if (BIT(data,6) && !BIT(data,11))
		{
			if (BIT(data,0) && !BIT(data,5)) res ^= 0x0001;
			if (BIT(data,8) && !BIT(data,5)) res ^= 0x0100;
		}
	}

	if (BIT(data,5) ^  BIT(data,14))
	{
		if ( BIT(data,14))                 res = subxor1(data,res);
		if (!BIT(data,9) &&  BIT(data,12)) res = subxor1(data,res);
	}

	if (BIT(data,5) ^ !BIT(data,13))
	{
		if ( BIT(data,5))                  res = subxor2(data,res);
		if ( BIT(data,9) &&  BIT(data,10)) res = subxor2(data,res);
		if (!BIT(data,9) && !BIT(data,7))  res = subxor2(data,res);
	}

	return res;
}

/***************************************************************************

    World Rally memory handlers

***************************************************************************/

WRITE16_HANDLER( OKIM6295_bankswitch_w )
{
	UINT8 *RAM = memory_region(REGION_SOUND1);

	if (ACCESSING_LSB){
		memcpy(&RAM[0x30000], &RAM[0x40000 + (data & 0x0f)*0x10000], 0x10000);
	}
}

WRITE16_HANDLER( wrally_coin_counter_w )
{
	coin_counter_w( (offset >> 3) & 0x01, data & 0x01);
}

WRITE16_HANDLER( wrally_coin_lockout_w )
{
	coin_lockout_w( (offset >> 3) & 0x01, ~data & 0x01);
}

/***************************************************************************

    World Rally init machine

***************************************************************************/

DRIVER_INIT( wrally )
{
	int i;

	/* recreate encryption tables on start up */
	wrally_encr_table[0] = (UINT16 *)auto_malloc(0x10000*2);
	wrally_encr_table[1] = (UINT16 *)auto_malloc(0x10000*2);
	for (i = 0; i < 0x10000; i++){
		wrally_encr_table[0][i] = wrally_decode_vram(i);
		wrally_encr_table[1][i] = wrally_decode_vram(i);
	}
}
