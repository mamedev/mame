// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Sprint 2 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/sprint2.h"


PALETTE_INIT_MEMBER(sprint2_state, sprint2)
{
	palette.set_indirect_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_indirect_color(1, rgb_t(0x5b, 0x5b, 0x5b));
	palette.set_indirect_color(2, rgb_t(0xa4, 0xa4, 0xa4));
	palette.set_indirect_color(3, rgb_t(0xff, 0xff, 0xff));

	palette.set_pen_indirect(0x0, 1);   /* black playfield */
	palette.set_pen_indirect(0x1, 0);
	palette.set_pen_indirect(0x2, 1);   /* white playfield */
	palette.set_pen_indirect(0x3, 3);

	palette.set_pen_indirect(0x4, 1);   /* car #1 */
	palette.set_pen_indirect(0x5, 3);
	palette.set_pen_indirect(0x6, 1);   /* car #2 */
	palette.set_pen_indirect(0x7, 0);
	palette.set_pen_indirect(0x8, 1);   /* car #3 */
	palette.set_pen_indirect(0x9, 2);
	palette.set_pen_indirect(0xa, 1);   /* car #4 */
	palette.set_pen_indirect(0xb, 2);
}


TILE_GET_INFO_MEMBER(sprint2_state::get_tile_info)
{
	UINT8 code = m_video_ram[tile_index];

	SET_TILE_INFO_MEMBER(0, code & 0x3f, code >> 7, 0);
}


void sprint2_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sprint2_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
}


READ8_MEMBER(sprint2_state::sprint2_collision1_r)
{
	return m_collision[0];
}
READ8_MEMBER(sprint2_state::sprint2_collision2_r)
{
	return m_collision[1];
}


WRITE8_MEMBER(sprint2_state::sprint2_collision_reset1_w)
{
	m_collision[0] = 0;
}
WRITE8_MEMBER(sprint2_state::sprint2_collision_reset2_w)
{
	m_collision[1] = 0;
}


WRITE8_MEMBER(sprint2_state::sprint2_video_ram_w)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


UINT8 sprint2_state::collision_check(rectangle& rect)
{
	UINT8 data = 0;

	int x;
	int y;

	for (y = rect.min_y; y <= rect.max_y; y++)
		for (x = rect.min_x; x <= rect.max_x; x++)
		{
			UINT16 a = m_palette->pen_indirect(m_helper.pix16(y, x));

			if (a == 0)
				data |= 0x40;

			if (a == 3)
				data |= 0x80;
		}

	return data;
}


inline int sprint2_state::get_sprite_code(UINT8 *video_ram, int n)
{
	return video_ram[0x398 + 2 * n + 1] >> 3;
}
inline int sprint2_state::get_sprite_x(UINT8 *video_ram, int n)
{
	return 2 * (248 - video_ram[0x390 + 1 * n]);
}
inline int sprint2_state::get_sprite_y(UINT8 *video_ram, int n)
{
	return 1 * (248 - video_ram[0x398 + 2 * n]);
}


UINT32 sprint2_state::screen_update_sprint2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *video_ram = m_video_ram;
	int i;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the sprites */

	for (i = 0; i < 4; i++)
	{
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			get_sprite_code(video_ram, i),
			i,
			0, 0,
			get_sprite_x(video_ram, i),
			get_sprite_y(video_ram, i), 0);
	}
	return 0;
}


void sprint2_state::screen_eof_sprint2(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		UINT8 *video_ram = m_video_ram;
		int i;
		int j;
		const rectangle &visarea = m_screen->visible_area();

		/*
		 * Collisions are detected for both player cars:
		 *
		 * D7 => m_collision w/ white playfield
		 * D6 => m_collision w/ black playfield or another car
		 *
		 */

		for (i = 0; i < 2; i++)
		{
			rectangle rect;

			rect.min_x = get_sprite_x(video_ram, i);
			rect.min_y = get_sprite_y(video_ram, i);
			rect.max_x = get_sprite_x(video_ram, i) + m_gfxdecode->gfx(1)->width() - 1;
			rect.max_y = get_sprite_y(video_ram, i) + m_gfxdecode->gfx(1)->height() - 1;

			rect &= visarea;

			/* check for sprite-tilemap collisions */

			m_bg_tilemap->draw(screen, m_helper, rect, 0, 0);

			m_gfxdecode->gfx(1)->transpen(m_helper,rect,
				get_sprite_code(video_ram, i),
				0,
				0, 0,
				get_sprite_x(video_ram, i),
				get_sprite_y(video_ram, i), 1);

			m_collision[i] |= collision_check(rect);

			/* check for sprite-sprite collisions */

			for (j = 0; j < 4; j++)
				if (j != i)
				{
					m_gfxdecode->gfx(1)->transpen(m_helper,rect,
						get_sprite_code(video_ram, j),
						1,
						0, 0,
						get_sprite_x(video_ram, j),
						get_sprite_y(video_ram, j), 0);
				}

			m_gfxdecode->gfx(1)->transpen(m_helper,rect,
				get_sprite_code(video_ram, i),
				0,
				0, 0,
				get_sprite_x(video_ram, i),
				get_sprite_y(video_ram, i), 1);

			m_collision[i] |= collision_check(rect);
		}
	}
}
