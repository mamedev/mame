// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Sprint 4 video emulation

***************************************************************************/

#include "emu.h"
#include "audio/sprint4.h"
#include "includes/sprint4.h"


PALETTE_INIT_MEMBER(sprint4_state, sprint4)
{
	palette.set_indirect_color(0, rgb_t(0x00, 0x00, 0x00)); /* black  */
	palette.set_indirect_color(1, rgb_t(0xfc, 0xdf, 0x80)); /* peach  */
	palette.set_indirect_color(2, rgb_t(0xf0, 0x00, 0xf0)); /* violet */
	palette.set_indirect_color(3, rgb_t(0x00, 0xf0, 0x0f)); /* green  */
	palette.set_indirect_color(4, rgb_t(0x30, 0x4f, 0xff)); /* blue   */
	palette.set_indirect_color(5, rgb_t(0xff, 0xff, 0xff)); /* white  */

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


TILE_GET_INFO_MEMBER(sprint4_state::sprint4_tile_info)
{
	UINT8 *videoram = m_videoram;
	UINT8 code = videoram[tile_index];

	if ((code & 0x30) == 0x30)
		SET_TILE_INFO_MEMBER(0, code & ~0x40, (code >> 6) ^ 3, 0);
	else
		SET_TILE_INFO_MEMBER(0, code, 4, 0);
}


void sprint4_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper);

	m_playfield = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sprint4_state::sprint4_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


UINT32 sprint4_state::screen_update_sprint4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int i;

	m_playfield->draw(screen, bitmap, cliprect, 0, 0);

	for (i = 0; i < 4; i++)
	{
		int bank = 0;

		UINT8 horz = videoram[0x390 + 2 * i + 0];
		UINT8 attr = videoram[0x390 + 2 * i + 1];
		UINT8 vert = videoram[0x398 + 2 * i + 0];
		UINT8 code = videoram[0x398 + 2 * i + 1];

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


void sprint4_state::screen_eof_sprint4(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		UINT8 *videoram = m_videoram;
		int i;

		/* check for sprite-playfield collisions */

		for (i = 0; i < 4; i++)
		{
			rectangle rect;

			int x;
			int y;

			int bank = 0;

			UINT8 horz = videoram[0x390 + 2 * i + 0];
			UINT8 vert = videoram[0x398 + 2 * i + 0];
			UINT8 code = videoram[0x398 + 2 * i + 1];

			rect.min_x = horz - 15;
			rect.min_y = vert - 15;
			rect.max_x = horz - 15 + m_gfxdecode->gfx(1)->width() - 1;
			rect.max_y = vert - 15 + m_gfxdecode->gfx(1)->height() - 1;

			rect &= m_screen->visible_area();

			m_playfield->draw(screen, m_helper, rect, 0, 0);

			if (i & 1)
				bank = 32;

			m_gfxdecode->gfx(1)->transpen(m_helper,rect,
				(code >> 3) | bank,
				4,
				0, 0,
				horz - 15,
				vert - 15, 1);

			for (y = rect.min_y; y <= rect.max_y; y++)
				for (x = rect.min_x; x <= rect.max_x; x++)
					if (m_palette->pen_indirect(m_helper.pix16(y, x)) != 0)
						m_collision[i] = 1;
		}

		/* update sound status */

		address_space &space = machine().driver_data()->generic_space();
		m_discrete->write(space, SPRINT4_MOTOR_DATA_1, videoram[0x391] & 15);
		m_discrete->write(space, SPRINT4_MOTOR_DATA_2, videoram[0x393] & 15);
		m_discrete->write(space, SPRINT4_MOTOR_DATA_3, videoram[0x395] & 15);
		m_discrete->write(space, SPRINT4_MOTOR_DATA_4, videoram[0x397] & 15);
	}
}


WRITE8_MEMBER(sprint4_state::sprint4_video_ram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_playfield->mark_tile_dirty(offset);
}
