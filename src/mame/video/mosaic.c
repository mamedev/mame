/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/mosaic.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	mosaic_state *state = (mosaic_state *)machine->driver_data;
	tile_index *= 2;
	SET_TILE_INFO(
			0,
			state->fgvideoram[tile_index] + (state->fgvideoram[tile_index+1] << 8),
			0,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mosaic_state *state = (mosaic_state *)machine->driver_data;
	tile_index *= 2;
	SET_TILE_INFO(
			1,
			state->bgvideoram[tile_index] + (state->bgvideoram[tile_index+1] << 8),
			0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( mosaic )
{
	mosaic_state *state = (mosaic_state *)machine->driver_data;

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0xff);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( mosaic_fgvideoram_w )
{
	mosaic_state *state = (mosaic_state *)space->machine->driver_data;

	state->fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset / 2);
}

WRITE8_HANDLER( mosaic_bgvideoram_w )
{
	mosaic_state *state = (mosaic_state *)space->machine->driver_data;

	state->bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}



VIDEO_UPDATE( mosaic )
{
	mosaic_state *state = (mosaic_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
