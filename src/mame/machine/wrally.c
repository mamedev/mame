/***************************************************************************

    World Rally

    Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
    I/O ports)

***************************************************************************/

#include "driver.h"
#include "cpu/mcs51/mcs51.h"
#include "gaelcrpt.h"
#include "includes/wrally.h"

/***************************************************************************

    World Rally memory handlers

***************************************************************************/

WRITE16_HANDLER( wrally_vram_w )
{
	data = gaelco_decrypt(space, offset, data, 0x1f, 0x522a);
	COMBINE_DATA(&wrally_videoram[offset]);

	tilemap_mark_tile_dirty(wrally_pant[(offset & 0x1fff) >> 12], ((offset << 1) & 0x1fff) >> 2);
}

WRITE16_HANDLER( wrally_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x01);
}

WRITE16_HANDLER( OKIM6295_bankswitch_w )
{
	UINT8 *RAM = memory_region(space->machine, "oki");

	if (ACCESSING_BITS_0_7){
		memcpy(&RAM[0x30000], &RAM[0x40000 + (data & 0x0f)*0x10000], 0x10000);
	}
}

WRITE16_HANDLER( wrally_coin_counter_w )
{
	coin_counter_w( space->machine, (offset >> 3) & 0x01, data & 0x01);
}

WRITE16_HANDLER( wrally_coin_lockout_w )
{
	coin_lockout_w( space->machine, (offset >> 3) & 0x01, ~data & 0x01);
}

