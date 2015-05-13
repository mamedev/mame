// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Blue Print

***************************************************************************/

#include "emu.h"
#include "includes/blueprnt.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Blue Print doesn't have color PROMs. For sprites, the ROM data is directly
  converted into colors; for characters, it is converted through the color
  code (bits 0-2 = RBG for 01 pixels, bits 3-5 = RBG for 10 pixels, 00 pixels
  always black, 11 pixels use the OR of bits 0-2 and 3-5. Bit 6 is intensity
  control)

***************************************************************************/

PALETTE_INIT_MEMBER(blueprnt_state, blueprnt)
{
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		UINT8 pen;
		int r, g, b;

		if (i < 0x200)
			/* characters */
			pen = ((i & 0x100) >> 5) |
					((i & 0x002) ? ((i & 0x0e0) >> 5) : 0) |
					((i & 0x001) ? ((i & 0x01c) >> 2) : 0);
		else
			/* sprites */
			pen = i - 0x200;

		r = ((pen >> 0) & 1) * ((pen & 0x08) ? 0xbf : 0xff);
		g = ((pen >> 2) & 1) * ((pen & 0x08) ? 0xbf : 0xff);
		b = ((pen >> 1) & 1) * ((pen & 0x08) ? 0xbf : 0xff);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

WRITE8_MEMBER(blueprnt_state::blueprnt_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(blueprnt_state::blueprnt_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);

	offset-=32;
	offset &=0x3ff;
	m_bg_tilemap->mark_tile_dirty(offset);

	offset+=64;
	offset &=0x3ff;
	m_bg_tilemap->mark_tile_dirty(offset);


}

WRITE8_MEMBER(blueprnt_state::blueprnt_flipscreen_w)
{
	flip_screen_set(~data & 0x02);

	if (m_gfx_bank != ((data & 0x04) >> 2))
	{
		m_gfx_bank = ((data & 0x04) >> 2);
		machine().tilemap().mark_all_dirty();
	}
}



TILE_GET_INFO_MEMBER(blueprnt_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int bank;

	// It looks like the upper bank attribute bit (at least) comes from the previous tile read.
	// Obviously if the screen is flipped the previous tile the hardware would read is different
	// to the previous tile when it's not flipped hence the if (flip_screen()) logic
	//
	// note, one line still ends up darkened in the cocktail mode of grasspin, but on the real
	// hardware there was no observable brightness difference between any part of the screen so
	// I'm not convinced the brightness implementation is correct anyway, it might simply be
	// tied to the use of upper / lower tiles or priority instead?
	if (flip_screen())
	{
		bank = m_colorram[(tile_index+32)&0x3ff] & 0x40;
	}
	else
	{
		bank = m_colorram[(tile_index-32)&0x3ff] & 0x40;
	}

	int code = m_videoram[tile_index];
	int color = attr & 0x7f;

	tileinfo.category = (attr & 0x80) ? 1 : 0;
	if (bank) code += m_gfx_bank * 0x100;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}



VIDEO_START_MEMBER(blueprnt_state,blueprnt)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(blueprnt_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS_FLIP_X, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(32);

	save_item(NAME(m_gfx_bank));
}


void blueprnt_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int code = m_spriteram[offs + 1];
		int sx = m_spriteram[offs + 3];
		int sy = 240 - m_spriteram[offs];
		int flipx = m_spriteram[offs + 2] & 0x40;
		int flipy = m_spriteram[offs + 2 - 4] & 0x80;    // -4? Awkward, isn't it?

		if (flip_screen())
		{
			sx = 248 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		// sprites are slightly misplaced, regardless of the screen flip
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, 0, flipx, flipy, 2 + sx, sy - 1, 0);
	}
}

UINT32 blueprnt_state::screen_update_blueprnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	if (flip_screen())
		for (i = 0; i < 32; i++)
			m_bg_tilemap->set_scrolly(i, m_scrollram[32 - i]);
	else
		for (i = 0; i < 32; i++)
			m_bg_tilemap->set_scrolly(i, m_scrollram[30 - i]);

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	return 0;
}
