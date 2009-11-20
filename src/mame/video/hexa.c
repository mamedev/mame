/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "hexa.h"

WRITE8_HANDLER( hexa_videoram_w )
{
	hexa_state *state = (hexa_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE8_HANDLER( hexa_d008_w )
{
	hexa_state *state = (hexa_state *)space->machine->driver_data;

	/* bit 0 = flipx (or y?) */
	if (flip_screen_x_get(space->machine) != (data & 0x01))
	{
		flip_screen_x_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 1 = flipy (or x?) */
	if (flip_screen_y_get(space->machine) != (data & 0x02))
	{
		flip_screen_y_set(space->machine, data & 0x02);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 2 - 3 unknown */

	/* bit 4 could be the ROM bank selector for 8000-bfff (not sure) */
	memory_set_bank(space->machine, 1, ((data & 0x10) >> 4));

	/* bit 5 = char bank */
	if (state->charbank != ((data & 0x20) >> 5))
	{
		state->charbank = (data & 0x20) >> 5;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 6 - 7 unknown */
}

static TILE_GET_INFO( get_bg_tile_info )
{
	hexa_state *state = (hexa_state *)machine->driver_data;
	int offs = tile_index * 2;
	int tile = state->videoram[offs + 1] + ((state->videoram[offs] & 0x07) << 8) + (state->charbank << 11);
	int color = (state->videoram[offs] & 0xf8) >> 3;

	SET_TILE_INFO(0, tile, color, 0);
}

VIDEO_START( hexa )
{
	hexa_state *state = (hexa_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}


VIDEO_UPDATE( hexa )
{
	hexa_state *state = (hexa_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	return 0;
}
