// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

  kchamp.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "kchamp.h"


void kchamp_state::kchamp_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < palette.entries(); i++)
	{
		int const red = color_prom[i];
		int const green = color_prom[palette.entries() + i];
		int const blue = color_prom[2 * palette.entries() + i];

		palette.set_pen_color(i, pal4bit(red), pal4bit(green), pal4bit(blue));
	}
}

void kchamp_state::kchamp_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void kchamp_state::kchamp_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(kchamp_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 7) << 8);
	int color = (m_colorram[tile_index] >> 3) & 0x1f;

	tileinfo.set(0, code, color, 0);
}

void kchamp_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kchamp_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

/*
        Sprites
        -------
        Offset        Encoding
             0        YYYYYYYY
             1        TTTTTTTT
             2        FGGTCCCC
             3        XXXXXXXX
*/

void kchamp_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int dx, int dy)
{
	for (int offs = 0; offs < 0x100; offs += 4)
	{
		int attr = m_spriteram[offs + 2];
		int bank = 1 + ((attr & 0x60) >> 5);
		int code = m_spriteram[offs + 1] + ((attr & 0x10) << 4);
		int color = attr & 0x0f;
		int flipx = 0;
		int flipy = attr & 0x80;
		int sx = m_spriteram[offs + 3] + dx;
		int sy = 240 - m_spriteram[offs] + dy;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(bank)->transpen(bitmap,cliprect, code, color, flipx, flipy, sx, sy, 0);
	}
}


uint32_t kchamp_state::screen_update_kchamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, -8, 7);
	return 0;
}

uint32_t kchamp_state::screen_update_kchampvs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 0, 0);
	return 0;
}
