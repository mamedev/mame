// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Poolshark video emulation

***************************************************************************/

#include "emu.h"
#include "includes/poolshrk.h"




TILE_GET_INFO_MEMBER(poolshrk_state::get_tile_info)
{
	SET_TILE_INFO_MEMBER(1, m_playfield_ram[tile_index] & 0x3f, 0, 0);
}


void poolshrk_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(poolshrk_state::get_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
}


UINT32 poolshrk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->mark_all_dirty();

	bitmap.fill(0, cliprect);

	/* draw sprites */

	for (int i = 0; i < 16; i++)
	{
		int hpos = m_hpos_ram[i];
		int vpos = m_vpos_ram[i];

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, i, (i == 0) ? 0 : 1, 0, 0,
			248 - hpos, vpos - 15, 0);
	}

	/* draw playfield */

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
