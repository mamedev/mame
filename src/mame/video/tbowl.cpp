// license:BSD-3-Clause
// copyright-holders:David Haywood
/* video/tbowl.c */

/* see drivers/tbowl.c for more info */

#include "emu.h"
#include "includes/tbowl.h"


/* Foreground Layer (tx) Tilemap */

TILE_GET_INFO_MEMBER(tbowl_state::get_tx_tile_info)
{
	int tileno;
	int col;

	tileno = m_txvideoram[tile_index] | ((m_txvideoram[tile_index+0x800] & 0x07) << 8);
	col = (m_txvideoram[tile_index+0x800] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(0,tileno,col,0);
}

WRITE8_MEMBER(tbowl_state::txvideoram_w)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}

/* Bottom BG Layer (bg) Tilemap */

TILE_GET_INFO_MEMBER(tbowl_state::get_bg_tile_info)
{
	int tileno;
	int col;

	tileno = m_bgvideoram[tile_index] | ((m_bgvideoram[tile_index+0x1000] & 0x0f) << 8);
	col = (m_bgvideoram[tile_index+0x1000] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(1,tileno,col,0);
}

WRITE8_MEMBER(tbowl_state::bg2videoram_w)
{
	m_bg2videoram[offset] = data;
	m_bg2_tilemap->mark_tile_dirty(offset & 0xfff);
}

WRITE8_MEMBER(tbowl_state::bgxscroll_lo)
{
	m_xscroll = (m_xscroll & 0xff00) | data;
}

WRITE8_MEMBER(tbowl_state::bgxscroll_hi)
{
	m_xscroll = (m_xscroll & 0x00ff) | (data << 8);
}

WRITE8_MEMBER(tbowl_state::bgyscroll_lo)
{
	m_yscroll = (m_yscroll & 0xff00) | data;
}

WRITE8_MEMBER(tbowl_state::bgyscroll_hi)
{
	m_yscroll = (m_yscroll & 0x00ff) | (data << 8);
}

/* Middle BG Layer (bg2) Tilemaps */

TILE_GET_INFO_MEMBER(tbowl_state::get_bg2_tile_info)
{
	int tileno;
	int col;

	tileno = m_bg2videoram[tile_index] | ((m_bg2videoram[tile_index+0x1000] & 0x0f) << 8);
	tileno ^= 0x400;
	col = (m_bg2videoram[tile_index+0x1000] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(2,tileno,col,0);
}

WRITE8_MEMBER(tbowl_state::bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0xfff);
}

WRITE8_MEMBER(tbowl_state::bg2xscroll_lo)
{
	m_bg2xscroll = (m_bg2xscroll & 0xff00) | data;
}

WRITE8_MEMBER(tbowl_state::bg2xscroll_hi)
{
	m_bg2xscroll = (m_bg2xscroll & 0x00ff) | (data << 8);
}

WRITE8_MEMBER(tbowl_state::bg2yscroll_lo)
{
	m_bg2yscroll = (m_bg2yscroll & 0xff00) | data;
}

WRITE8_MEMBER(tbowl_state::bg2yscroll_hi)
{
	m_bg2yscroll = (m_bg2yscroll & 0x00ff) | (data << 8);
}


/*** Video Start / Update ***/

void tbowl_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tbowl_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tbowl_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16,128,32);
	m_bg2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tbowl_state::get_bg2_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16,128,32);

	m_tx_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg2_tilemap->set_transparent_pen(0);

	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	save_item(NAME(m_bg2xscroll));
	save_item(NAME(m_bg2yscroll));
}



UINT32 tbowl_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_xscroll );
	m_bg_tilemap->set_scrolly(0, m_yscroll );
	m_bg2_tilemap->set_scrollx(0, m_bg2xscroll );
	m_bg2_tilemap->set_scrolly(0, m_bg2yscroll );
	m_tx_tilemap->set_scrollx(0, 0 );
	m_tx_tilemap->set_scrolly(0, 0 );

	bitmap.fill(0x100, cliprect); /* is there a register controling the colour? looks odd when screen is blank */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_sprgen->tbowl_draw_sprites(bitmap,cliprect, m_gfxdecode, 0, m_spriteram);
	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}

UINT32 tbowl_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_xscroll+32*8 );
	m_bg_tilemap->set_scrolly(0, m_yscroll );
	m_bg2_tilemap->set_scrollx(0, m_bg2xscroll+32*8 );
	m_bg2_tilemap->set_scrolly(0, m_bg2yscroll );
	m_tx_tilemap->set_scrollx(0, 32*8 );
	m_tx_tilemap->set_scrolly(0, 0 );

	bitmap.fill(0x100, cliprect); /* is there a register controling the colour? looks odd when screen is blank */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_sprgen->tbowl_draw_sprites(bitmap,cliprect, m_gfxdecode, 32*8, m_spriteram);
	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}
