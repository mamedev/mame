/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/trackfld.h"


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

	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

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

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( yiear_videoram_w )
{
	trackfld_state *state = (trackfld_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE8_HANDLER( yiear_control_w )
{
	trackfld_state *state = (trackfld_state *)space->machine->driver_data;
	/* bit 0 flips screen */
	if (flip_screen_get(space->machine) != (data & 0x01))
	{
		flip_screen_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}

	/* bit 1 is NMI enable */
	state->yiear_nmi_enable = data & 0x02;

	/* bit 2 is IRQ enable */
	interrupt_enable_w(space, 0, data & 0x04);

	/* bits 3 and 4 are coin counters */
	coin_counter_w(space->machine, 0, data & 0x08);
	coin_counter_w(space->machine, 1, data & 0x10);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;
	int offs = tile_index * 2;
	int attr = state->videoram[offs];
	int code = state->videoram[offs + 1] | ((attr & 0x10) << 4);
//  int color = (attr & 0xf0) >> 4;
	int flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x40) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, 0, flags);
}

VIDEO_START( yiear )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	UINT8 *spriteram_2 = state->spriteram2;
	int offs;

	for (offs = state->spriteram_size - 2; offs >= 0; offs -= 2)
	{
		int attr = spriteram[offs];
		int code = spriteram_2[offs + 1] + 256 * (attr & 0x01);
		int color = 0;
		int flipx = ~attr & 0x40;
		int flipy = attr & 0x80;
		int sy = 240 - spriteram[offs + 1];
		int sx = spriteram_2[offs];

		if (flip_screen_get(machine))
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		if (offs < 0x26)
		{
			sy++;	/* fix title screen & garbage at the bottom of the screen */
		}

		drawgfx_transpen(bitmap, cliprect,
			machine->gfx[1],
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

VIDEO_UPDATE( yiear )
{
	trackfld_state *state = (trackfld_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
