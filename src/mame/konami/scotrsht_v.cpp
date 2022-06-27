// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
#include "emu.h"
#include "scotrsht.h"


// Similar as Iron Horse
void scotrsht_state::scotrsht_palette(palette_device &palette) const
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

	// characters use colors 0x80-0xff, sprites use colors 0-0x7f
	for (int i = 0; i < 0x200; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			uint8_t const ctabentry = ((~i & 0x100) >> 1) | (j << 4) | (color_prom[i] & 0x0f);
			palette.set_pen_indirect(((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

void scotrsht_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void scotrsht_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void scotrsht_state::charbank_w(uint8_t data)
{
	if (m_charbank != (data & 0x01))
	{
		m_charbank = data & 0x01;
		m_bg_tilemap->mark_all_dirty();
	}

	// other bits unknown
}

void scotrsht_state::palettebank_w(uint8_t data)
{
	if (m_palette_bank != ((data & 0x70) >> 4))
	{
		m_palette_bank = ((data & 0x70) >> 4);
		m_bg_tilemap->mark_all_dirty();
	}

	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);

	// data & 4 unknown
}


TILE_GET_INFO_MEMBER(scotrsht_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + (m_charbank << 9) + ((attr & 0x40) << 2);
	int color = (attr & 0x0f) + m_palette_bank * 16;
	int flag = 0;

	if(attr & 0x10) flag |= TILE_FLIPX;
	if(attr & 0x20) flag |= TILE_FLIPY;

	// data & 0x80 -> tile priority?

	tileinfo.set(0, code, color, flag);
}

/* Same as Jailbreak + palette bank */
void scotrsht_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int i = 0; i < m_spriteram.bytes(); i += 4)
	{
		int attr = m_spriteram[i + 1];    // attributes = ?tyxcccc
		int code = m_spriteram[i] + ((attr & 0x40) << 2);
		int color = (attr & 0x0f) + m_palette_bank * 16;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = m_spriteram[i + 2] - ((attr & 0x80) << 1);
		int sy = m_spriteram[i + 3];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transmask(bitmap,cliprect, code, color, flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, m_palette_bank * 16));
	}
}

void scotrsht_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(scotrsht_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap->set_scroll_cols(64);

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_charbank));
	save_item(NAME(m_palette_bank));
}

uint32_t scotrsht_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int col = 0; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, m_scroll[col]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
