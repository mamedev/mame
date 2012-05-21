/***************************************************************************

    Atari Tetris hardware

***************************************************************************/

#include "emu.h"
#include "includes/atetris.h"


/*************************************
 *
 *  Tilemap callback
 *
 *************************************/

static TILE_GET_INFO( get_tile_info )
{
	atetris_state *state = machine.driver_data<atetris_state>();
	UINT8 *videoram = state->m_videoram;
	int code = videoram[tile_index * 2] | ((videoram[tile_index * 2 + 1] & 7) << 8);
	int color = (videoram[tile_index * 2 + 1] & 0xf0) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

WRITE8_MEMBER(atetris_state::atetris_videoram_w)
{
	UINT8 *videoram = m_videoram;

	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atetris )
{
	atetris_state *state = machine.driver_data<atetris_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows,  8,8, 64,32);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

SCREEN_UPDATE_IND16( atetris )
{
	atetris_state *state = screen.machine().driver_data<atetris_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}
