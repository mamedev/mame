/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/hanaawas.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT( hanaawas )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x10);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x10; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* character lookup table.  The 1bpp tiles really only use colors 0-0x0f and the
       3bpp ones 0x10-0x1f */
	for (i = 0; i < 0x100; i++)
	{
		int swapped_i = BITSWAP8(i,2,7,6,5,4,3,1,0);
		UINT8 ctabentry = color_prom[swapped_i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE8_HANDLER( hanaawas_videoram_w )
{
	hanaawas_state *state = (hanaawas_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( hanaawas_colorram_w )
{
	hanaawas_state *state = (hanaawas_state *)space->machine->driver_data;
	state->colorram[offset] = data;

	/* dirty both current and next offsets */
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
	tilemap_mark_tile_dirty(state->bg_tilemap, (offset + (flip_screen_get(space->machine) ? -1 : 1)) & 0x03ff);
}

WRITE8_DEVICE_HANDLER( hanaawas_portB_w )
{
	/* bit 7 is flip screen */
	if (flip_screen_get(device->machine) != (~data & 0x80))
	{
		flip_screen_set(device->machine, ~data & 0x80);
		tilemap_mark_all_tiles_dirty_all(device->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	hanaawas_state *state = (hanaawas_state *)machine->driver_data;
	/* the color is determined by the current color byte, but the bank is via the previous one!!! */
	int offset = (tile_index + (flip_screen_get(machine) ? 1 : -1)) & 0x3ff;
	int attr = state->colorram[offset];
	int gfxbank = (attr & 0x40) >> 6;
	int code = state->videoram[tile_index] + ((attr & 0x20) << 3);
	int color = state->colorram[tile_index] & 0x1f;

	SET_TILE_INFO(gfxbank, code, color, 0);
}

VIDEO_START( hanaawas )
{
	hanaawas_state *state = (hanaawas_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

VIDEO_UPDATE( hanaawas )
{
	hanaawas_state *state = (hanaawas_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	return 0;
}
