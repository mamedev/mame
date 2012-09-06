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

TILE_GET_INFO_MEMBER(_4enraya_state::get_tile_info)
{

	int code = m_videoram[tile_index * 2] + (m_videoram[tile_index * 2 + 1] << 8);
	SET_TILE_INFO_MEMBER(
		0,
		code,
		0,
		0);
}

VIDEO_START( 4enraya )
{
	_4enraya_state *state = machine.driver_data<_4enraya_state>();

	state->m_bg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(_4enraya_state::get_tile_info),state), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

SCREEN_UPDATE_IND16( 4enraya )
{
	_4enraya_state *state = screen.machine().driver_data<_4enraya_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
