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
#include "vsystem_spr.h"
#include "includes/inufuku.h"


/******************************************************************************

    Memory handlers

******************************************************************************/

WRITE16_MEMBER(inufuku_state::inufuku_palettereg_w)
{
	switch (offset)
	{
		case 0x02:  m_bg_palettebank = (data & 0xf000) >> 12;
				m_bg_tilemap->mark_all_dirty();
				break;
		case 0x03:  m_tx_palettebank = (data & 0xf000) >> 12;
				m_tx_tilemap->mark_all_dirty();
				break;
	}
}

WRITE16_MEMBER(inufuku_state::inufuku_scrollreg_w)
{
	switch (offset)
	{
		case 0x00:  m_bg_scrollx = data + 1; break;
		case 0x01:  m_bg_scrolly = data + 0; break;
		case 0x02:  m_tx_scrollx = data - 3; break;
		case 0x03:  m_tx_scrolly = data + 1; break;
		case 0x04:  m_bg_raster = (data & 0x0200) ? 0 : 1; break;
	}
}


/******************************************************************************

    Sprite routines

******************************************************************************/



/******************************************************************************

    Tilemap callbacks

******************************************************************************/

TILE_GET_INFO_MEMBER(inufuku_state::get_inufuku_bg_tile_info)
{
	SET_TILE_INFO_MEMBER(0,
			m_bg_videoram[tile_index],
			m_bg_palettebank,
			0);
}

TILE_GET_INFO_MEMBER(inufuku_state::get_inufuku_tx_tile_info)
{
	SET_TILE_INFO_MEMBER(1,
			m_tx_videoram[tile_index],
			m_tx_palettebank,
			0);
}

READ16_MEMBER(inufuku_state::inufuku_bg_videoram_r)
{
	return m_bg_videoram[offset];
}

WRITE16_MEMBER(inufuku_state::inufuku_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

READ16_MEMBER(inufuku_state::inufuku_tx_videoram_r)
{
	return m_tx_videoram[offset];
}

WRITE16_MEMBER(inufuku_state::inufuku_tx_videoram_w)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


UINT32 inufuku_state::inufuku_tile_callback( UINT32 code )
{
	return ((m_spriteram2[code*2] & 0x0007) << 16) + m_spriteram2[(code*2)+ 1];
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

void inufuku_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(inufuku_state::get_inufuku_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(inufuku_state::get_inufuku_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_bg_tilemap->set_transparent_pen(255);
	m_tx_tilemap->set_transparent_pen(255);

	m_spriteram1_old = make_unique_clear<UINT16[]>(m_spriteram1.bytes()/2);

}


/******************************************************************************

    Display refresh

******************************************************************************/

UINT32 inufuku_state::screen_update_inufuku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0);

	if (m_bg_raster)
	{
		m_bg_tilemap->set_scroll_rows(512);
		for (i = 0; i < 256; i++)
			m_bg_tilemap->set_scrollx((m_bg_scrolly + i) & 0x1ff, m_bg_scrollx+m_bg_rasterram[i]);
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

	m_spr->draw_sprites( m_spriteram1_old.get(), m_spriteram1.bytes(), screen, bitmap, cliprect );
	return 0;
}

void inufuku_state::screen_eof_inufuku(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_spriteram1_old.get(),m_spriteram1,m_spriteram1.bytes());
	}
}
