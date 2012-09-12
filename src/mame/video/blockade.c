#include "emu.h"
#include "includes/blockade.h"

WRITE8_MEMBER(blockade_state::blockade_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);

	if (ioport("IN3")->read() & 0x80)
	{
		logerror("blockade_videoram_w: scanline %d\n", machine().primary_screen->vpos());
		space.device().execute().spin_until_interrupt();
	}
}

TILE_GET_INFO_MEMBER(blockade_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

VIDEO_START( blockade )
{
	blockade_state *state = machine.driver_data<blockade_state>();
	state->m_bg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(blockade_state::get_bg_tile_info),state), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

SCREEN_UPDATE_IND16( blockade )
{
	blockade_state *state = screen.machine().driver_data<blockade_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
