// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Stefan Jokisch
/***************************************************************************

Atari Ultra Tank video emulation

***************************************************************************/

#include "emu.h"
#include "includes/ultratnk.h"
#include "audio/sprint4.h"


void ultratnk_state::ultratnk_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	palette.set_indirect_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_indirect_color(1, rgb_t(0xa4, 0xa4, 0xa4));
	palette.set_indirect_color(2, rgb_t(0x5b, 0x5b, 0x5b));
	palette.set_indirect_color(3, rgb_t(0xff, 0xff, 0xff));

	palette.set_pen_indirect(0, color_prom[0x00] & 3);
	palette.set_pen_indirect(2, color_prom[0x00] & 3);
	palette.set_pen_indirect(4, color_prom[0x00] & 3);
	palette.set_pen_indirect(6, color_prom[0x00] & 3);
	palette.set_pen_indirect(8, color_prom[0x00] & 3);

	palette.set_pen_indirect(1, color_prom[0x01] & 3);
	palette.set_pen_indirect(3, color_prom[0x02] & 3);
	palette.set_pen_indirect(5, color_prom[0x04] & 3);
	palette.set_pen_indirect(7, color_prom[0x08] & 3);
	palette.set_pen_indirect(9, color_prom[0x10] & 3);
}


TILE_GET_INFO_MEMBER(ultratnk_state::tile_info)
{
	uint8_t code = m_videoram[tile_index];

	if (code & 0x20)
		SET_TILE_INFO_MEMBER(0, code, code >> 6, 0);
	else
		SET_TILE_INFO_MEMBER(0, code, 4, 0);
}


void ultratnk_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper);

	m_playfield = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ultratnk_state::tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


uint32_t ultratnk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_playfield->draw(screen, bitmap, cliprect, 0, 0);

	for (int i = 0; i < 4; i++)
	{
		int bank = 0;

		uint8_t horz = m_videoram[0x390 + 2 * i + 0];
		uint8_t attr = m_videoram[0x390 + 2 * i + 1];
		uint8_t vert = m_videoram[0x398 + 2 * i + 0];
		uint8_t code = m_videoram[0x398 + 2 * i + 1];

		if (code & 4)
			bank = 32;

		if (!(attr & 0x80))
		{
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				(code >> 3) | bank,
				i,
				0, 0,
				horz - 15,
				vert - 15, 0);
		}
	}

	return 0;
}


WRITE_LINE_MEMBER(ultratnk_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		uint16_t BG = m_palette->pen_indirect(0);

		/* check for sprite-playfield collisions */

		for (int i = 0; i < 4; i++)
		{
			int bank = 0;

			uint8_t horz = m_videoram[0x390 + 2 * i + 0];
			uint8_t vert = m_videoram[0x398 + 2 * i + 0];
			uint8_t code = m_videoram[0x398 + 2 * i + 1];

			rectangle rect(
					horz - 15,
					horz - 15 + m_gfxdecode->gfx(1)->width() - 1,
					vert - 15,
					vert - 15 + m_gfxdecode->gfx(1)->height() - 1);
			rect &= m_screen->visible_area();

			m_playfield->draw(*m_screen, m_helper, rect, 0, 0);

			if (code & 4)
				bank = 32;

			m_gfxdecode->gfx(1)->transpen(m_helper,rect,
				(code >> 3) | bank,
				4,
				0, 0,
				horz - 15,
				vert - 15, 1);

			for (int y = rect.top(); y <= rect.bottom(); y++)
				for (int x = rect.left(); x <= rect.right(); x++)
					if (m_palette->pen_indirect(m_helper.pix16(y, x)) != BG)
						m_collision[i] = 1;
		}

		/* update sound status */
		m_discrete->write(ULTRATNK_MOTOR_DATA_1, m_videoram[0x391] & 15);
		m_discrete->write(ULTRATNK_MOTOR_DATA_2, m_videoram[0x393] & 15);
	}
}


WRITE8_MEMBER(ultratnk_state::video_ram_w)
{
	m_videoram[offset] = data;
	m_playfield->mark_tile_dirty(offset);
}
