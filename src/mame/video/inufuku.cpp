// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Video Hardware for Video System Games.

    Quiz & Variety Sukusuku Inufuku
    (c)1998 Video System Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2003/08/09 -

    based on other Video System drivers

******************************************************************************/

#include "emu.h"
#include "includes/inufuku.h"
#include "screen.h"


/******************************************************************************

    Memory handlers

******************************************************************************/

void inufuku_state::palettereg_w(offs_t offset, u16 data)
{
	switch (offset)
	{
		case 0x02:
			m_bg_palettebank = (data & 0xf000) >> 12;
			m_bg_tilemap->mark_all_dirty();
			break;
		case 0x03:
			m_tx_palettebank = (data & 0xf000) >> 12;
			m_tx_tilemap->mark_all_dirty();
			break;
	}
}

void inufuku_state::scrollreg_w(offs_t offset, u16 data)
{
	switch (offset)
	{
		case 0x00:  m_bg_scrollx = data + 1; break;
		case 0x01:  m_bg_scrolly = data + 0; break;
		case 0x02:  m_tx_scrollx = data - 3; break;
		case 0x03:  m_tx_scrolly = data + 1; break;
		case 0x04:  m_bg_raster = BIT(~data, 9); break;
	}
}


/******************************************************************************

    Sprite routines

******************************************************************************/



/******************************************************************************

    Tilemap callbacks

******************************************************************************/

TILE_GET_INFO_MEMBER(inufuku_state::get_bg_tile_info)
{
	tileinfo.set(0,
			m_bg_videoram[tile_index],
			m_bg_palettebank,
			0);
}

TILE_GET_INFO_MEMBER(inufuku_state::get_tx_tile_info)
{
	tileinfo.set(1,
			m_tx_videoram[tile_index],
			m_tx_palettebank,
			0);
}

void inufuku_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void inufuku_state::tx_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


u32 inufuku_state::tile_callback( u32 code )
{
	return ((m_sprtileram[code * 2] & 0x0007) << 16) + m_sprtileram[(code * 2) + 1];
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

void inufuku_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(inufuku_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(inufuku_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_bg_tilemap->set_transparent_pen(255);
	m_tx_tilemap->set_transparent_pen(255);
}


/******************************************************************************

    Display refresh

******************************************************************************/

u32 inufuku_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0);

	if (m_bg_raster)
	{
		m_bg_tilemap->set_scroll_rows(512);
		for (int i = cliprect.min_y; i <= cliprect.max_y; i++)
			m_bg_tilemap->set_scrollx((m_bg_scrolly + i) & 0x1ff, m_bg_scrollx + m_bg_rasterram[i]);
	}
	else
	{
		m_bg_tilemap->set_scroll_rows(1);
		m_bg_tilemap->set_scrollx(0, m_bg_scrollx);
	}
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_tx_tilemap->set_scrollx(0, m_tx_scrollx);
	m_tx_tilemap->set_scrolly(0, m_tx_scrolly);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	m_spr->draw_sprites( m_sprattrram->buffer(), m_sprattrram->bytes(), screen, bitmap, cliprect );
	return 0;
}
