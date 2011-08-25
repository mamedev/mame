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
	mosaic_state *state = machine.driver_data<mosaic_state>();
	tile_index *= 2;
	SET_TILE_INFO(
			0,
			state->m_fgvideoram[tile_index] + (state->m_fgvideoram[tile_index+1] << 8),
			0,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mosaic_state *state = machine.driver_data<mosaic_state>();
	tile_index *= 2;
	SET_TILE_INFO(
			1,
			state->m_bgvideoram[tile_index] + (state->m_bgvideoram[tile_index+1] << 8),
			0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( mosaic )
{
	mosaic_state *state = machine.driver_data<mosaic_state>();

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->m_fg_tilemap, 0xff);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( mosaic_fgvideoram_w )
{
	mosaic_state *state = space->machine().driver_data<mosaic_state>();

	state->m_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset / 2);
}

WRITE8_HANDLER( mosaic_bgvideoram_w )
{
	mosaic_state *state = space->machine().driver_data<mosaic_state>();

	state->m_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset / 2);
}



SCREEN_UPDATE( mosaic )
{
	mosaic_state *state = screen->machine().driver_data<mosaic_state>();

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);
	return 0;
}
