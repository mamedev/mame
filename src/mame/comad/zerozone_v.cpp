// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  video/zerozone.cpp

***************************************************************************/

#include "emu.h"
#include "zerozone.h"

void zerozone_state::tilemap_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
	m_zz_tilemap->mark_tile_dirty(offset);
}


void zerozone_state::tilebank_w(uint8_t data)
{
//  popmessage ("Data %04x",data);
	m_tilebank = data & 0x07;
	m_zz_tilemap->mark_all_dirty();
}

TILE_GET_INFO_MEMBER(zerozone_state::get_zerozone_tile_info)
{
	int tileno = m_vram[tile_index] & 0x07ff;
	int colour = m_vram[tile_index] & 0xf000;

	if (m_vram[tile_index] & 0x0800)
		tileno += m_tilebank * 0x800;

	tileinfo.set(0, tileno, colour >> 12, 0);
}

void zerozone_state::video_start()
{
	// I'm not 100% sure it should be opaque, pink title screen looks strange in Las Vegas Girls
	// but if it's transparent other things look incorrect
	m_zz_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(zerozone_state::get_zerozone_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);
}

uint32_t zerozone_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_zz_tilemap->draw(screen, bitmap, cliprect);
	return 0;
}
