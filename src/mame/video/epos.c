/***************************************************************************

    Epos games

***************************************************************************/

#include "emu.h"
#include "includes/epos.h"

/***************************************************************************

  These games has one 32 byte palette PROM, connected to the RGB output this way:

  bit 7 -- 240 ohm resistor  -- RED
        -- 510 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
        -- 240 ohm resistor  -- GREEN
        -- 510 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 240 ohm resistor  -- BLUE
  bit 0 -- 510 ohm resistor  -- BLUE

***************************************************************************/

static void get_pens( running_machine *machine, pen_t *pens )
{
	offs_t i;
	const UINT8 *prom = memory_region(machine, "proms");
	int len = memory_region_length(machine, "proms");

	for (i = 0; i < len; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		UINT8 data = prom[i];

		bit0 = (data >> 7) & 0x01;
		bit1 = (data >> 6) & 0x01;
		bit2 = (data >> 5) & 0x01;
		r = 0x92 * bit0 + 0x4a * bit1 + 0x23 * bit2;

		bit0 = (data >> 4) & 0x01;
		bit1 = (data >> 3) & 0x01;
		bit2 = (data >> 2) & 0x01;
		g = 0x92 * bit0 + 0x4a * bit1 + 0x23 * bit2;

		bit0 = (data >> 1) & 0x01;
		bit1 = (data >> 0) & 0x01;
		b = 0xad * bit0 + 0x52 * bit1;

		pens[i] = MAKE_RGB(r, g, b);
	}
}


WRITE8_HANDLER( epos_port_1_w )
{
	epos_state *state = space->machine->driver_data<epos_state>();
	/* D0 - start light #1
       D1 - start light #2
       D2 - coin counter
       D3 - palette select
       D4-D7 - unused
     */

	set_led_status(space->machine, 0, (data >> 0) & 0x01);
	set_led_status(space->machine, 1, (data >> 1) & 0x01);

	coin_counter_w(space->machine, 0, (data >> 2) & 0x01);

	state->palette = (data >> 3) & 0x01;
}


VIDEO_UPDATE( epos )
{
	epos_state *state = screen->machine->driver_data<epos_state>();
	pen_t pens[0x20];
	offs_t offs;

	get_pens(screen->machine, pens);

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		UINT8 data = state->videoram[offs];

		int x = (offs % 136) * 2;
		int y = (offs / 136);

		*BITMAP_ADDR32(bitmap, y, x + 0) = pens[(state->palette << 4) | (data & 0x0f)];
		*BITMAP_ADDR32(bitmap, y, x + 1) = pens[(state->palette << 4) | (data >> 4)];
	}

	return 0;
}
