// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Aaron Giles
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

TILE_GET_INFO_MEMBER(atetris_state::get_tile_info)
{
	int code = m_videoram[tile_index * 2] | ((m_videoram[tile_index * 2 + 1] & 7) << 8);
	int color = (m_videoram[tile_index * 2 + 1] & 0xf0) >> 4;

	tileinfo.set(0, code, color, 0);
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

void atetris_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void atetris_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(atetris_state::get_tile_info)), TILEMAP_SCAN_ROWS,  8,8, 64,32);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t atetris_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
