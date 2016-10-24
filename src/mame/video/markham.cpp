// license:BSD-3-Clause
// copyright-holders:Uki
/******************************************************************************

    Markham (c) 1983 Sun Electronics

    Video hardware driver by Uki

    17/Jun/2001 -

******************************************************************************/

#include "emu.h"
#include "includes/markham.h"

void markham_state::palette_init_markham(palette_device &palette)
{
	const uint8_t *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* sprites lookup table */
	for (i = 0; i < 0x400; i++)
	{
		uint8_t ctabentry = color_prom[i];
		palette.set_pen_indirect(i, ctabentry);
	}
}

void markham_state::markham_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void markham_state::markham_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

void markham_state::get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int attr = m_videoram[tile_index * 2];
	int code = m_videoram[(tile_index * 2) + 1] + ((attr & 0x60) << 3);
	int color = (attr & 0x1f) | ((attr & 0x80) >> 2);

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void markham_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(markham_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);
}

void markham_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t *spriteram = m_spriteram;
	int offs;

	for (offs = 0x60; offs < 0x100; offs += 4)
	{
		int chr = spriteram[offs + 1];
		int col = spriteram[offs + 2];

		int fx = flip_screen();
		int fy = flip_screen();

		int x = spriteram[offs + 3];
		int y = spriteram[offs + 0];
		int px, py;
		col &= 0x3f ;

		if (flip_screen() == 0)
		{
			px = x - 2;
			py = 240 - y;
		}
		else
		{
			px = 240 - x;
			py = y;
		}

		px = px & 0xff;

		if (px > 248)
			px = px - 256;

		m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
			chr,
			col,
			fx,fy,
			px,py,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), col, 0));
	}
}

uint32_t markham_state::screen_update_markham(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if ((i > 3) && (i < 16))
			m_bg_tilemap->set_scrollx(i, m_xscroll[0]);
		if (i >= 16)
			m_bg_tilemap->set_scrollx(i, m_xscroll[1]);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
