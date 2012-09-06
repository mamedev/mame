/***************************************************************************

Atari Poolshark video emulation

***************************************************************************/

#include "emu.h"
#include "includes/poolshrk.h"




TILE_GET_INFO_MEMBER(poolshrk_state::get_tile_info)
{
	SET_TILE_INFO_MEMBER(1, m_playfield_ram[tile_index] & 0x3f, 0, 0);
}


VIDEO_START( poolshrk )
{
	poolshrk_state *state = machine.driver_data<poolshrk_state>();
	state->m_bg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(poolshrk_state::get_tile_info),state), TILEMAP_SCAN_ROWS,
		 8, 8, 32, 32);

	state->m_bg_tilemap->set_transparent_pen(0);
}


SCREEN_UPDATE_IND16( poolshrk )
{
	poolshrk_state *state = screen.machine().driver_data<poolshrk_state>();
	int i;

	state->m_bg_tilemap->mark_all_dirty();

	bitmap.fill(0, cliprect);

	/* draw sprites */

	for (i = 0; i < 16; i++)
	{
		int hpos = state->m_hpos_ram[i];
		int vpos = state->m_vpos_ram[i];

		drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[0], i, (i == 0) ? 0 : 1, 0, 0,
			248 - hpos, vpos - 15, 0);
	}

	/* draw playfield */

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
