// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/seicross.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Seicross has two 32x8 palette PROMs, connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
void seicross_state::seicross_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

WRITE8_MEMBER(seicross_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(seicross_state::colorram_w)
{
	/* bit 5 of the address is not used for color memory. There is just */
	/* 512k of memory; every two consecutive rows share the same memory */
	/* region. */

	offset &= 0xffdf;

	m_colorram[offset] = data;
	m_colorram[offset + 0x20] = data;

	m_bg_tilemap->mark_tile_dirty(offset);
	m_bg_tilemap->mark_tile_dirty(offset + 0x20);
}

TILE_GET_INFO_MEMBER(seicross_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x10) << 4);
	int color = m_colorram[tile_index] & 0x0f;
	int flags = ((m_colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((m_colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void seicross_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(seicross_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_bg_tilemap->set_scroll_cols(32);
}

void seicross_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int x = m_spriteram[offs + 3];
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				(m_spriteram[offs] & 0x3f) + ((m_spriteram[offs + 1] & 0x10) << 2) + 128,
				m_spriteram[offs + 1] & 0x0f,
				m_spriteram[offs] & 0x40,m_spriteram[offs] & 0x80,
				x,240-m_spriteram[offs + 2],0);
		if(x>0xf0)
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					(m_spriteram[offs] & 0x3f) + ((m_spriteram[offs + 1] & 0x10) << 2) + 128,
					m_spriteram[offs + 1] & 0x0f,
					m_spriteram[offs] & 0x40,m_spriteram[offs] & 0x80,
					x-256,240-m_spriteram[offs + 2],0);
	}

	for (offs = m_spriteram2.bytes() - 4; offs >= 0; offs -= 4)
	{
		int x = m_spriteram2[offs + 3];
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				(m_spriteram2[offs] & 0x3f) + ((m_spriteram2[offs + 1] & 0x10) << 2),
				m_spriteram2[offs + 1] & 0x0f,
				m_spriteram2[offs] & 0x40,m_spriteram2[offs] & 0x80,
				x,240-m_spriteram2[offs + 2],0);
		if(x>0xf0)
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					(m_spriteram2[offs] & 0x3f) + ((m_spriteram2[offs + 1] & 0x10) << 2),
					m_spriteram2[offs + 1] & 0x0f,
					m_spriteram2[offs] & 0x40,m_spriteram2[offs] & 0x80,
					x-256,240-m_spriteram2[offs + 2],0);
	}
}

uint32_t seicross_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int col;

	for (col = 0; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, m_row_scroll[col]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
