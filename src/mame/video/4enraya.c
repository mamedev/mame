/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/4enraya.h"

WRITE8_HANDLER( fenraya_videoram_w )
{
	_4enraya_state *state = space->machine().driver_data<_4enraya_state>();

	state->m_videoram[(offset & 0x3ff) * 2] = data;
	state->m_videoram[(offset & 0x3ff) * 2 + 1] = (offset & 0xc00) >> 10;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset & 0x3ff);
}

static TILE_GET_INFO( get_tile_info )
{
	_4enraya_state *state = machine.driver_data<_4enraya_state>();

	int code = state->m_videoram[tile_index * 2] + (state->m_videoram[tile_index * 2 + 1] << 8);
	SET_TILE_INFO(
		0,
		code,
		0,
		0);
}

VIDEO_START( 4enraya )
{
	_4enraya_state *state = machine.driver_data<_4enraya_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

SCREEN_UPDATE( 4enraya )
{
	_4enraya_state *state = screen->machine().driver_data<_4enraya_state>();

	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	return 0;
}
