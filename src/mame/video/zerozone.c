// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  video/zerozone.c

***************************************************************************/

#include "emu.h"
#include "includes/zerozone.h"

WRITE16_MEMBER( zerozone_state::tilemap_w )
{
	COMBINE_DATA(&m_vram[offset]);
	m_zz_tilemap->mark_tile_dirty(offset);
}


WRITE16_MEMBER( zerozone_state::tilebank_w )
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

	SET_TILE_INFO_MEMBER(0, tileno, colour >> 12, 0);
}

void zerozone_state::video_start()
{
	// i'm not 100% sure it should be opaque, pink title screen looks strange in las vegas girls
	// but if its transparent other things look incorrect
	m_zz_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(zerozone_state::get_zerozone_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 64, 32);
}

UINT32 zerozone_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_zz_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
