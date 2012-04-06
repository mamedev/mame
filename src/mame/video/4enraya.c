/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/4enraya.h"

WRITE8_MEMBER(_4enraya_state::fenraya_videoram_w)
{

	m_videoram[(offset & 0x3ff) * 2] = data;
	m_videoram[(offset & 0x3ff) * 2 + 1] = (offset & 0xc00) >> 10;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
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

SCREEN_UPDATE_IND16( 4enraya )
{
	_4enraya_state *state = screen.machine().driver_data<_4enraya_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
