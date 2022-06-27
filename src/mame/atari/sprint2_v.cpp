// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Sprint 2 video emulation

***************************************************************************/

#include "emu.h"
#include "sprint2.h"


void sprint2_state::palette(palette_device &palette) const
{
	palette.set_indirect_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_indirect_color(1, rgb_t(0x5b, 0x5b, 0x5b));
	palette.set_indirect_color(2, rgb_t(0xa4, 0xa4, 0xa4));
	palette.set_indirect_color(3, rgb_t(0xff, 0xff, 0xff));

	palette.set_pen_indirect(0x0, 1);   // black playfield
	palette.set_pen_indirect(0x1, 0);
	palette.set_pen_indirect(0x2, 1);   // white playfield
	palette.set_pen_indirect(0x3, 3);

	palette.set_pen_indirect(0x4, 1);   // car #1
	palette.set_pen_indirect(0x5, 3);
	palette.set_pen_indirect(0x6, 1);   // car #2
	palette.set_pen_indirect(0x7, 0);
	palette.set_pen_indirect(0x8, 1);   // car #3
	palette.set_pen_indirect(0x9, 2);
	palette.set_pen_indirect(0xa, 1);   // car #4
	palette.set_pen_indirect(0xb, 2);
}


TILE_GET_INFO_MEMBER(sprint2_state::get_tile_info)
{
	uint8_t code = m_video_ram[tile_index];

	tileinfo.set(0, code & 0x3f, code >> 7, 0);
}


void sprint2_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sprint2_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
}


void sprint2_state::video_ram_w(offs_t offset, uint8_t data)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


uint8_t sprint2_state::collision_check(rectangle& rect)
{
	uint8_t data = 0;

	for (int y = rect.top(); y <= rect.bottom(); y++)
		for (int x = rect.left(); x <= rect.right(); x++)
		{
			uint16_t const a = m_palette->pen_indirect(m_helper.pix(y, x));

			if (a == 0)
				data |= 0x40;

			if (a == 3)
				data |= 0x80;
		}

	return data;
}


inline int sprint2_state::get_sprite_code(int n)
{
	return m_video_ram[0x398 + 2 * n + 1] >> 3;
}
inline int sprint2_state::get_sprite_x(int n)
{
	return 2 * (248 - m_video_ram[0x390 + 1 * n]);
}
inline int sprint2_state::get_sprite_y(int n)
{
	return 1 * (248 - m_video_ram[0x398 + 2 * n]);
}


uint32_t sprint2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(*m_screen, bitmap, cliprect, 0, 0);

	// draw the sprites

	for (int i = 0; i < 4; i++)
	{
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			get_sprite_code(i),
			i,
			0, 0,
			get_sprite_x(i),
			get_sprite_y(i), 0);
	}
	return 0;
}


WRITE_LINE_MEMBER(sprint2_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		const rectangle &visarea = m_screen->visible_area();

		/*
		 * Collisions are detected for both player cars:
		 *
		 * D7 => m_collision w/ white playfield
		 * D6 => m_collision w/ black playfield or another car
		 *
		 */

		for (int i = 0; i < 2; i++)
		{
			rectangle rect(
					get_sprite_x(i),
					get_sprite_x(i) + m_gfxdecode->gfx(1)->width() - 1,
					get_sprite_y(i),
					get_sprite_y(i) + m_gfxdecode->gfx(1)->height() - 1);
			rect &= visarea;

			// check for sprite-tilemap collisions

			m_bg_tilemap->draw(*m_screen, m_helper, rect, 0, 0);

			m_gfxdecode->gfx(1)->transpen(m_helper,rect,
				get_sprite_code(i),
				0,
				0, 0,
				get_sprite_x(i),
				get_sprite_y(i), 1);

			m_collision[i] |= collision_check(rect);

			// check for sprite-sprite collisions

			for (int j = 0; j < 4; j++)
				if (j != i)
				{
					m_gfxdecode->gfx(1)->transpen(m_helper,rect,
						get_sprite_code(j),
						1,
						0, 0,
						get_sprite_x(j),
						get_sprite_y(j), 0);
				}

			m_gfxdecode->gfx(1)->transpen(m_helper,rect,
				get_sprite_code(i),
				0,
				0, 0,
				get_sprite_x(i),
				get_sprite_y(i), 1);

			m_collision[i] |= collision_check(rect);
		}
	}
}
