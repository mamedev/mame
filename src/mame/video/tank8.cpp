// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Tank 8 video emulation

***************************************************************************/

#include "emu.h"
#include "includes/tank8.h"


PALETTE_INIT_MEMBER(tank8_state, tank8)
{
	int i;

	palette.set_indirect_color(8, rgb_t(0x00, 0x00, 0x00));
	palette.set_indirect_color(9, rgb_t(0xff, 0xff, 0xff));

	for (i = 0; i < 8; i++)
	{
		palette.set_pen_indirect(2 * i + 0, 8);
		palette.set_pen_indirect(2 * i + 1, i);
	}

	/* walls */
	palette.set_pen_indirect(0x10, 8);
	palette.set_pen_indirect(0x11, 9);

	/* mines */
	palette.set_pen_indirect(0x12, 8);
	palette.set_pen_indirect(0x13, 9);
}


void tank8_state::set_pens()
{
	if (*m_team & 0x01)
	{
		m_palette->set_indirect_color(0, rgb_t(0xff, 0x00, 0x00)); /* red     */
		m_palette->set_indirect_color(1, rgb_t(0x00, 0x00, 0xff)); /* blue    */
		m_palette->set_indirect_color(2, rgb_t(0xff, 0xff, 0x00)); /* yellow  */
		m_palette->set_indirect_color(3, rgb_t(0x00, 0xff, 0x00)); /* green   */
		m_palette->set_indirect_color(4, rgb_t(0xff, 0x00, 0xff)); /* magenta */
		m_palette->set_indirect_color(5, rgb_t(0xe0, 0xc0, 0x70)); /* puce    */
		m_palette->set_indirect_color(6, rgb_t(0x00, 0xff, 0xff)); /* cyan    */
		m_palette->set_indirect_color(7, rgb_t(0xff, 0xaa, 0xaa)); /* pink    */
	}
	else
	{
		m_palette->set_indirect_color(0, rgb_t(0xff, 0x00, 0x00)); /* red     */
		m_palette->set_indirect_color(2, rgb_t(0xff, 0x00, 0x00)); /* red     */
		m_palette->set_indirect_color(4, rgb_t(0xff, 0x00, 0x00)); /* red     */
		m_palette->set_indirect_color(6, rgb_t(0xff, 0x00, 0x00)); /* red     */
		m_palette->set_indirect_color(1, rgb_t(0x00, 0x00, 0xff)); /* blue    */
		m_palette->set_indirect_color(3, rgb_t(0x00, 0x00, 0xff)); /* blue    */
		m_palette->set_indirect_color(5, rgb_t(0x00, 0x00, 0xff)); /* blue    */
		m_palette->set_indirect_color(7, rgb_t(0x00, 0x00, 0xff)); /* blue    */
	}
}


WRITE8_MEMBER(tank8_state::video_ram_w)
{
	m_video_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}



TILE_GET_INFO_MEMBER(tank8_state::get_tile_info)
{
	UINT8 code = m_video_ram[tile_index];

	int color = 0;

	if ((code & 0x38) == 0x28)
	{
		if ((code & 7) != 3)
			color = 8; /* walls */
		else
			color = 9; /* mines */
	}
	else
	{
		if (tile_index & 0x010)
			color |= 1;

		if (code & 0x80)
			color |= 2;

		if (tile_index & 0x200)
			color |= 4;
	}

	SET_TILE_INFO_MEMBER(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}



void tank8_state::video_start()
{
	m_collision_timer = timer_alloc(TIMER_COLLISION);

	m_screen->register_screen_bitmap(m_helper1);
	m_screen->register_screen_bitmap(m_helper2);
	m_screen->register_screen_bitmap(m_helper3);

	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tank8_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	/* VBLANK starts on scanline #256 and ends on scanline #24 */

	m_tilemap->set_scrolly(0, 2 * 24);

	save_item(NAME(m_collision_index));
}


int tank8_state::get_x_pos(int n)
{
	return 498 - m_pos_h_ram[n] - 2 * (m_pos_d_ram[n] & 128); /* ? */
}


int tank8_state::get_y_pos(int n)
{
	return 2 * m_pos_v_ram[n] - 62;
}


void tank8_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		UINT8 code = ~m_pos_d_ram[i];

		int x = get_x_pos(i);
		int y = get_y_pos(i);

		m_gfxdecode->gfx((code & 0x04) ? 2 : 3)->transpen(bitmap,cliprect,
			code & 0x03,
			i,
			code & 0x10,
			code & 0x08,
			x,
			y, 0);
	}
}


void tank8_state::draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		int x = get_x_pos(8 + i);
		int y = get_y_pos(8 + i);

		x -= 4; /* ? */

		rectangle rect(x, x + 3, y, y + 4);
		rect &= cliprect;

		bitmap.fill((i << 1) | 0x01, rect);
	}
}


void tank8_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_COLLISION:
		set_collision(param);
		break;
	default:
		assert_always(FALSE, "Unknown id in tank8_state::device_timer");
	}
}


UINT32 tank8_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);
	draw_bullets(bitmap, cliprect);
	return 0;
}


void tank8_state::screen_eof(screen_device &screen, bool state)
{
	// on falling edge
	if (!state)
	{
		int x;
		int y;
		const rectangle &visarea = m_screen->visible_area();

		m_tilemap->draw(screen, m_helper1, visarea, 0, 0);

		m_helper2.fill(8, visarea);
		m_helper3.fill(8, visarea);

		draw_sprites(m_helper2, visarea);
		draw_bullets(m_helper3, visarea);

		for (y = visarea.min_y; y <= visarea.max_y; y++)
		{
			int _state = 0;

			const UINT16* p1 = &m_helper1.pix16(y);
			const UINT16* p2 = &m_helper2.pix16(y);
			const UINT16* p3 = &m_helper3.pix16(y);

			if ((m_screen->frame_number() ^ y) & 1)
				continue; /* video display is interlaced */

			for (x = visarea.min_x; x <= visarea.max_x; x++)
			{
				UINT8 index;

				/* neither wall nor mine */
				if ((p1[x] != 0x11) && (p1[x] != 0x13))
				{
					_state = 0;
					continue;
				}

				/* neither tank nor bullet */
				if ((p2[x] == 8) && (p3[x] == 8))
				{
					_state = 0;
					continue;
				}

				/* bullets cannot hit mines */
				if ((p3[x] != 8) && (p1[x] == 0x13))
				{
					_state = 0;
					continue;
				}

				if (_state)
					continue;

				if (p3[x] != 8)
				{
					index = ((p3[x] & ~0x01) >> 1) | 0x18;

					if (1)
						index |= 0x20;

					if (0)
						index |= 0x40;

					if (1)
						index |= 0x80;
				}
				else
				{
					int sprite_num = (p2[x] & ~0x01) >> 1;
					index = sprite_num | 0x10;

					if (p1[x] == 0x11)
						index |= 0x20;

					if (y - get_y_pos(sprite_num) >= 8)
						index |= 0x40; /* collision on bottom side */

					if (x - get_x_pos(sprite_num) >= 8)
						index |= 0x80; /* collision on right side */
				}

				m_collision_timer->adjust(screen.time_until_pos(y, x), index);

				_state = 1;
			}
		}
	}
}
