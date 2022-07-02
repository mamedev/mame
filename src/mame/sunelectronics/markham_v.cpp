// license:BSD-3-Clause
// copyright-holders:Uki
/******************************************************************************

    Markham (c) 1983 Sun Electronics
    Strength & Skill (c) 1984 Sun Electronics

    Video hardware driver by Uki

******************************************************************************/

#include "emu.h"
#include "markham.h"

void markham_state::markham_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i | 0x000]);
		int const g = pal4bit(color_prom[i | 0x100]);
		int const b = pal4bit(color_prom[i | 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// sprites lookup table
	for (int i = 0; i < 0x400; i++)
	{
		uint8_t const ctabentry = color_prom[i];
		palette.set_pen_indirect(i, ctabentry);
	}
}

void markham_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(markham_state::get_bg_tile_info)
{
	int attr = m_videoram[tile_index * 2];
	int code = m_videoram[(tile_index * 2) + 1] + ((attr & 0x60) << 3);
	int color = (attr & 0x1f) | ((attr & 0x80) >> 2);

	tileinfo.set(0, code, color, 0);
}

void markham_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(markham_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);
}

VIDEO_START_MEMBER(markham_state, strnskil)
{
	video_start();

	m_bg_tilemap->set_scroll_rows(32);
	m_irq_scanline_start = 109;
	m_irq_scanline_end = 240;

	save_item(NAME(m_irq_source));
	save_item(NAME(m_irq_scanline_start));
	save_item(NAME(m_irq_scanline_end));
	save_item(NAME(m_scroll_ctrl));
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
		col &= 0x3f;

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
	int row;

	for (row = 0; row < 32; row++)
	{
		if ((row > 3) && (row < 16))
			m_bg_tilemap->set_scrollx(row, m_xscroll[0]);
		if (row >= 16)
			m_bg_tilemap->set_scrollx(row, m_xscroll[1]);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

uint32_t markham_state::screen_update_strnskil(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint8_t *scroll_data = (const uint8_t *)memregion("scroll_prom")->base();

	int row;

	for (row = 0; row < 32; row++)
	{
		switch (scroll_data[m_scroll_ctrl * 32 + row])
		{
			case 2:
				m_bg_tilemap->set_scrollx(row, -~m_xscroll[1]);
				break;
			case 4:
				m_bg_tilemap->set_scrollx(row, -~m_xscroll[0]);
				break;
			default:
				// case 6 and 0
				m_bg_tilemap->set_scrollx(row, 0);
				break;
		}
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
