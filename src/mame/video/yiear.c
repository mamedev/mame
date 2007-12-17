/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

int yiear_nmi_enable;

static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Yie Ar Kung-Fu has one 32x8 palette PROM, connected to the RGB output this
  way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( yiear )
{
	int i;


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( yiear_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( yiear_control_w )
{
	/* bit 0 flips screen */

	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	/* bit 1 is NMI enable */

	yiear_nmi_enable = data & 0x02;

	/* bit 2 is IRQ enable */

	interrupt_enable_w(0, data & 0x04);

	/* bits 3 and 4 are coin counters */

	coin_counter_w(0, data & 0x08);
	coin_counter_w(1, data & 0x10);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index * 2;
	int attr = videoram[offs];
	int code = videoram[offs + 1] | ((attr & 0x10) << 4);
//  int color = (attr & 0xf0) >> 4;
	int flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x40) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, 0, flags);
}

VIDEO_START( yiear )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size - 2;offs >= 0;offs -= 2)
	{
		int attr = spriteram[offs];
		int code = spriteram_2[offs + 1] + 256 * (attr & 0x01);
		int color = 0;
		int flipx = ~attr & 0x40;
		int flipy = attr & 0x80;
		int sy = 240 - spriteram[offs + 1];
		int sx = spriteram_2[offs];

		if (flip_screen)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		if (offs < 0x26)
		{
			sy++;	/* fix title screen & garbage at the bottom of the screen */
		}

		drawgfx(bitmap, machine->gfx[1],
			code, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( yiear )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
