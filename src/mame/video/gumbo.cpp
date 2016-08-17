// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Gumbo video */

#include "emu.h"
#include "includes/gumbo.h"


WRITE16_MEMBER(gumbo_state::gumbo_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(gumbo_state::get_gumbo_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}


WRITE16_MEMBER(gumbo_state::gumbo_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(gumbo_state::get_gumbo_fg_tile_info)
{
	int tileno = m_fg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(1, tileno, 1, 0);
}


void gumbo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gumbo_state::get_gumbo_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gumbo_state::get_gumbo_fg_tile_info),this), TILEMAP_SCAN_ROWS, 4, 4, 128, 64);
	m_fg_tilemap->set_transparent_pen(0xff);
}

UINT32 gumbo_state::screen_update_gumbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
