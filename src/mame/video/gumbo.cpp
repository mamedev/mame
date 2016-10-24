// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Gumbo video */

#include "emu.h"
#include "includes/gumbo.h"


void gumbo_state::gumbo_bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gumbo_state::get_gumbo_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tileno = m_bg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}


void gumbo_state::gumbo_fg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

void gumbo_state::get_gumbo_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tileno = m_fg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(1, tileno, 1, 0);
}


void gumbo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(gumbo_state::get_gumbo_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(gumbo_state::get_gumbo_fg_tile_info),this), TILEMAP_SCAN_ROWS, 4, 4, 128, 64);
	m_fg_tilemap->set_transparent_pen(0xff);
}

uint32_t gumbo_state::screen_update_gumbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
