// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Video emulation for Omori Battle Cross

***************************************************************************/

#include "emu.h"
#include "includes/battlex.h"


void battlex_state::battlex_palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_palette->set_pen_color(offset, pal1bit(data >> 0), pal1bit(data >> 2), pal1bit(data >> 1));
}

void battlex_state::battlex_scroll_x_lsb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_scroll_lsb = data;
}

void battlex_state::battlex_scroll_x_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_scroll_msb = data;
}

void battlex_state::battlex_scroll_starfield_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

void battlex_state::battlex_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void battlex_state::battlex_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_starfield_enabled = data & 0x10;

	if (flip_screen() != (data >> 7))
	{
		flip_screen_set(data & 0x80);
		machine().tilemap().mark_all_dirty();
	}
}


void battlex_state::get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tile = m_videoram[tile_index * 2] | (((m_videoram[tile_index * 2 + 1] & 0x01)) << 8);
	int color = (m_videoram[tile_index * 2 + 1] & 0x0e) >> 1; // high bits unused

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

void battlex_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(battlex_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

void battlex_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	uint8_t *source = m_spriteram;
	uint8_t *finish = m_spriteram + 0x200;

	while (source < finish)
	{
		int sx = (source[0] & 0x7f) * 2 - (source[0] & 0x80) * 2;
		int sy = source[3];
		int tile = source[2] & 0x7f;
		int color = source[1] & 0x07;   /* bits 3,4,5 also used during explosions */
		int flipy = source[1] & 0x80;
		int flipx = source[1] & 0x40;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		gfx->transpen(bitmap,cliprect, tile, color, flipx, flipy, sx, sy, 0);
		source += 4;
	}

}


uint32_t battlex_state::screen_update_battlex(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!flip_screen())
		m_bg_tilemap->set_scrollx(0, m_scroll_lsb | (m_scroll_msb << 8));
	else
		m_bg_tilemap->set_scrollx(0, m_scroll_lsb | m_scroll_msb);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	return 0;
}
