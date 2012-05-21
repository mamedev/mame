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

	state->m_fg_tilemap->set_transparent_pen(0xff);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(mosaic_state::mosaic_fgvideoram_w)
{

	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(mosaic_state::mosaic_bgvideoram_w)
{

	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}



SCREEN_UPDATE_IND16( mosaic )
{
	mosaic_state *state = screen.machine().driver_data<mosaic_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
