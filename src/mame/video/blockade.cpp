// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
#include "emu.h"
#include "includes/blockade.h"

WRITE8_MEMBER(blockade_state::blockade_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);

	if (ioport("IN3")->read() & 0x80)
	{
		logerror("blockade_videoram_w: scanline %d\n", m_screen->vpos());
		space.device().execute().spin_until_interrupt();
	}
}

TILE_GET_INFO_MEMBER(blockade_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void blockade_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(blockade_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

UINT32 blockade_state::screen_update_blockade(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
