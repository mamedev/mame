// license:BSD-3-Clause
// copyright-holders:David Haywood
/* video/pass.c - see drivers/pass.c for more info */

#include "emu.h"
#include "pass.h"

/* background tilemap stuff */

TILE_GET_INFO_MEMBER(pass_state::get_pass_bg_tile_info)
{
	int tileno, fx;

	tileno = m_bg_videoram[tile_index] & 0x1fff;
	fx = (m_bg_videoram[tile_index] & 0xc000) >> 14;
	tileinfo.set(1, tileno, 0, TILE_FLIPYX(fx));

}

void pass_state::pass_bg_videoram_w(offs_t offset, uint16_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

/* foreground 'sprites' tilemap stuff */

TILE_GET_INFO_MEMBER(pass_state::get_pass_fg_tile_info)
{
	int tileno, flip;

	tileno = m_fg_videoram[tile_index] & 0x3fff;
	flip = (m_fg_videoram[tile_index] & 0xc000) >>14;

	tileinfo.set(0, tileno, 0, TILE_FLIPYX(flip));

}

void pass_state::pass_fg_videoram_w(offs_t offset, uint16_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

/* video update / start */

void pass_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pass_state::get_pass_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8,  64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pass_state::get_pass_fg_tile_info)), TILEMAP_SCAN_ROWS, 4, 4, 128, 64);

	m_fg_tilemap->set_transparent_pen(255);
}

uint32_t pass_state::screen_update_pass(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
