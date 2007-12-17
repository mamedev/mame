#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"

static int scanline=0;

INTERRUPT_GEN( beezer_interrupt )
{
	scanline = (scanline + 1) % 0x80;
	via_0_ca2_w (0, scanline & 0x10);
	if ((scanline & 0x78) == 0x78)
		cpunum_set_input_line(0, M6809_FIRQ_LINE, ASSERT_LINE);
	else
		cpunum_set_input_line(0, M6809_FIRQ_LINE, CLEAR_LINE);
}

VIDEO_UPDATE( beezer )
{
	int x,y;

	for (y = machine->screen[0].visarea.min_y; y <= machine->screen[0].visarea.max_y; y+=2)
	{
		for (x = machine->screen[0].visarea.min_x; x <= machine->screen[0].visarea.max_x; x++)
		{
			*BITMAP_ADDR16(bitmap, y+1, x) = machine->pens[videoram[0x80*y+x] & 0x0f];
			*BITMAP_ADDR16(bitmap, y,   x) = machine->pens[(videoram[0x80*y+x] >> 4)& 0x0f];
		}
	}

	return 0;
}

WRITE8_HANDLER( beezer_map_w )
{
	/*
      bit 7 -- 330  ohm resistor  -- BLUE
            -- 560  ohm resistor  -- BLUE
            -- 330  ohm resistor  -- GREEN
            -- 560  ohm resistor  -- GREEN
            -- 1.2 kohm resistor  -- GREEN
            -- 330  ohm resistor  -- RED
            -- 560  ohm resistor  -- RED
      bit 0 -- 1.2 kohm resistor  -- RED
    */

	int r, g, b, bit0, bit1, bit2;

	/* red component */
	bit0 = (data >> 0) & 0x01;
	bit1 = (data >> 1) & 0x01;
	bit2 = (data >> 2) & 0x01;
	r = 0x26 * bit0 + 0x50 * bit1 + 0x89 * bit2;
	/* green component */
	bit0 = (data >> 3) & 0x01;
	bit1 = (data >> 4) & 0x01;
	bit2 = (data >> 5) & 0x01;
	g = 0x26 * bit0 + 0x50 * bit1 + 0x89 * bit2;
	/* blue component */
	bit0 = (data >> 6) & 0x01;
	bit1 = (data >> 7) & 0x01;
	b = 0x5f * bit0 + 0xa0 * bit1;

	palette_set_color(Machine, offset, MAKE_RGB(r, g, b));
}

READ8_HANDLER( beezer_line_r )
{
	return (scanline & 0xfe) << 1;
}

