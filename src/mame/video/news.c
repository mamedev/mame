// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "includes/news.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(news_state::get_fg_tile_info)
{
	int code = (m_fgram[tile_index * 2] << 8) | m_fgram[tile_index * 2 + 1];
	SET_TILE_INFO_MEMBER(0,
			code & 0x0fff,
			(code & 0xf000) >> 12,
			0);
}

TILE_GET_INFO_MEMBER(news_state::get_bg_tile_info)
{
	int code = (m_bgram[tile_index * 2] << 8) | m_bgram[tile_index * 2 + 1];
	int color = (code & 0xf000) >> 12;

	code &= 0x0fff;
	if ((code & 0x0e00) == 0x0e00)
		code = (code & 0x1ff) | (m_bgpic << 9);

	SET_TILE_INFO_MEMBER(0,
			code,
			color,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void news_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(news_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(news_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(news_state::news_fgram_w)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(news_state::news_bgram_w)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(news_state::news_bgpic_w)
{
	if (m_bgpic != data)
	{
		m_bgpic = data;
		m_bg_tilemap->mark_all_dirty();
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 news_state::screen_update_news(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
