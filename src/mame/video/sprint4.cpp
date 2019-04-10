// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Sprint 4 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/sprint4.h"
#include "audio/sprint4.h"
#include "screen.h"


void sprint4_state::sprint4_palette(palette_device &palette) const
{
	palette.set_indirect_color(0, rgb_t(0x00, 0x00, 0x00)); // black
	palette.set_indirect_color(1, rgb_t(0xfc, 0xdf, 0x80)); // peach
	palette.set_indirect_color(2, rgb_t(0xf0, 0x00, 0xf0)); // violet
	palette.set_indirect_color(3, rgb_t(0x00, 0xf0, 0x0f)); // green
	palette.set_indirect_color(4, rgb_t(0x30, 0x4f, 0xff)); // blue
	palette.set_indirect_color(5, rgb_t(0xff, 0xff, 0xff)); // white

	palette.set_pen_indirect(0, 0);
	palette.set_pen_indirect(2, 0);
	palette.set_pen_indirect(4, 0);
	palette.set_pen_indirect(6, 0);
	palette.set_pen_indirect(8, 0);

	palette.set_pen_indirect(1, 1);
	palette.set_pen_indirect(3, 2);
	palette.set_pen_indirect(5, 3);
	palette.set_pen_indirect(7, 4);
	palette.set_pen_indirect(9, 5);
}


TILE_GET_INFO_MEMBER(sprint4_state::tile_info)
{
	uint8_t code = m_videoram[tile_index];

	if ((code & 0x30) == 0x30)
		SET_TILE_INFO_MEMBER(0, code & ~0x40, (code >> 6) ^ 3, 0);
	else
		SET_TILE_INFO_MEMBER(0, code, 4, 0);
}


void sprint4_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper);

	m_playfield = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(sprint4_state::tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


uint32_t sprint4_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_playfield->draw(screen, bitmap, cliprect, 0, 0);

	for (int i = 0; i < 4; i++)
	{
		int bank = 0;

		uint8_t horz = m_videoram[0x390 + 2 * i + 0];
		uint8_t attr = m_videoram[0x390 + 2 * i + 1];
		uint8_t vert = m_videoram[0x398 + 2 * i + 0];
		uint8_t code = m_videoram[0x398 + 2 * i + 1];

		if (i & 1)
			bank = 32;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			(code >> 3) | bank,
			(attr & 0x80) ? 4 : i,
			0, 0,
			horz - 15,
			vert - 15, 0);
	}
	return 0;
}


WRITE_LINE_MEMBER(sprint4_state::screen_vblank)
{
	// rising edge
	if (state)
	{
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

			if (i & 1)
				bank = 32;

			m_gfxdecode->gfx(1)->transpen(m_helper,rect,
				(code >> 3) | bank,
				4,
				0, 0,
				horz - 15,
				vert - 15, 1);

			for (int y = rect.top(); y <= rect.bottom(); y++)
				for (int x = rect.left(); x <= rect.right(); x++)
					if (m_palette->pen_indirect(m_helper.pix16(y, x)) != 0)
						m_collision[i] = 1;
		}

		/* update sound status */

		m_discrete->write(SPRINT4_MOTOR_DATA_1, m_videoram[0x391] & 15);
		m_discrete->write(SPRINT4_MOTOR_DATA_2, m_videoram[0x393] & 15);
		m_discrete->write(SPRINT4_MOTOR_DATA_3, m_videoram[0x395] & 15);
		m_discrete->write(SPRINT4_MOTOR_DATA_4, m_videoram[0x397] & 15);
	}
}


WRITE8_MEMBER(sprint4_state::video_ram_w)
{
	m_videoram[offset] = data;
	m_playfield->mark_tile_dirty(offset);
}
