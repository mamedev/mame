// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/gberet.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Green Beret has a 32 bytes palette PROM and two 256 bytes color lookup table
  PROMs (one for sprites, one for characters).
  The palette PROM is connected to the RGB output, this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void gberet_base_state::palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		// red component
		int bit0 = BIT(color_prom[i], 0);
		int bit1 = BIT(color_prom[i], 1);
		int bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}

	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void gberet_base_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gberet_base_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gberet_state::scroll_w(offs_t offset, uint8_t data)
{
	m_scrollram[offset] = data;

	int scroll = m_scrollram[offset & 0x1f] | (m_scrollram[offset | 0x20] << 8);
	m_bg_tilemap->set_scrollx(offset & 0x1f, scroll);
}

void gberet_state::sprite_bank_w(uint8_t data)
{
	m_spritebank = data;
}

TILE_GET_INFO_MEMBER(gberet_base_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0x40) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	tileinfo.group = color;
	tileinfo.category = (attr & 0x80) >> 7;

	tileinfo.set(0, code, color, flags);
}

void gberet_base_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gberet_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap->configure_groups(*m_gfxdecode->gfx(0), 0x10);
	m_bg_tilemap->set_scroll_rows(32);
}

void gberet_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t *sr;

	if (m_spritebank & 0x08)
		sr = m_spriteram2;
	else
		sr = m_spriteram;

	for (int offs = 0; offs < 0xc0; offs += 4)
	{
		if (sr[offs + 3])
		{
			int attr = sr[offs + 1];
			int code = sr[offs + 0] + ((attr & 0x40) << 2);
			int color = attr & 0x0f;
			int sx = sr[offs + 2] - 2 * (attr & 0x80);
			int sy = sr[offs + 3];
			int flipx = attr & 0x10;
			int flipy = attr & 0x20;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(1)->transmask(bitmap,cliprect, code, color, flipx, flipy, sx, sy,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
		}
	}
}

uint32_t gberet_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

// Green Beret (bootleg)

void gberetb_state::scroll_w(offs_t offset, uint8_t data)
{
	int scroll = data;

	if (offset)
		scroll |= 0x100;

	for (offset = 6; offset < 29; offset++)
		m_bg_tilemap->set_scrollx(offset, scroll + 64 - 8);
}

void gberetb_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		if (m_spriteram[offs + 1])
		{
			int attr = m_spriteram[offs + 3];
			int code = m_spriteram[offs] + ((attr & 0x40) << 2);
			int color = attr & 0x0f;
			int sx = m_spriteram[offs + 2] - 2 * (attr & 0x80);
			int sy = 240 - m_spriteram[offs + 1];
			int flipx = attr & 0x10;
			int flipy = attr & 0x20;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(1)->transmask(bitmap,cliprect, code, color, flipx, flipy, sx, sy,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
		}
	}
}

uint32_t gberetb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
