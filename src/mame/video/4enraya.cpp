// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Roberto Fresca
/***************************************************************************

  IDSA 4 En Raya

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/4enraya.h"

void _4enraya_state::fenraya_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram[(offset & 0x3ff) * 2] = data;
	m_videoram[(offset & 0x3ff) * 2 + 1] = (offset & 0xc00) >> 10;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void _4enraya_state::get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_videoram[tile_index * 2] + (m_videoram[tile_index * 2 + 1] << 8);
	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void _4enraya_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(_4enraya_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t _4enraya_state::screen_update_4enraya(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
