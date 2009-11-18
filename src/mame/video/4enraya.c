/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/4enraya.h"

WRITE8_HANDLER( fenraya_videoram_w )
{
	_4enraya_state *state = (_4enraya_state *)space->machine->driver_data;

	state->videoram[(offset & 0x3ff) * 2] = data;
	state->videoram[(offset & 0x3ff) * 2 + 1] = (offset & 0xc00) >> 10;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset & 0x3ff);
}

static TILE_GET_INFO( get_tile_info )
{
	_4enraya_state *state = (_4enraya_state *)machine->driver_data;

	int code = state->videoram[tile_index * 2] + (state->videoram[tile_index * 2 + 1] << 8);
	SET_TILE_INFO(
		0,
		code,
		0,
		0);
}

VIDEO_START( 4enraya )
{
	_4enraya_state *state = (_4enraya_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

VIDEO_UPDATE( 4enraya )
{
	_4enraya_state *state = (_4enraya_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	return 0;
}
