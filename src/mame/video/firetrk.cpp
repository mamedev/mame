// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

Atari Fire Truck + Super Bug + Monte Carlo video emulation

***************************************************************************/

#include "emu.h"
#include "includes/firetrk.h"

static const rectangle playfield_window(0x02a, 0x115, 0x000, 0x0ff);


PALETTE_INIT_MEMBER(firetrk_state, firetrk)
{
	int i;

	static const UINT8 colortable_source[] =
	{
		0, 0, 1, 0,
		2, 0, 3, 0,
		3, 3, 2, 3,
		1, 3, 0, 3,
		0, 0, 1, 0,
		2, 0, 0, 3,
		3, 0, 0, 3
	};
	static const rgb_t palette_source[] =
	{
		rgb_t::black,
		rgb_t(0x5b, 0x5b, 0x5b),
		rgb_t(0xa4, 0xa4, 0xa4),
		rgb_t::white
	};

	m_color1_mask = m_color2_mask = 0;

	for (i = 0; i < ARRAY_LENGTH(colortable_source); i++)
	{
		UINT8 color = colortable_source[i];

		if (color == 1)
			m_color1_mask |= 1 << i;
		else if (color == 2)
			m_color2_mask |= 1 << i;

		palette.set_pen_color(i, palette_source[color]);
	}
}


void firetrk_state::prom_to_palette(int number, UINT8 val)
{
	m_palette->set_pen_color(number, rgb_t(pal1bit(val >> 2), pal1bit(val >> 1), pal1bit(val >> 0)));
}


PALETTE_INIT_MEMBER(firetrk_state,montecar)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	static const UINT8 colortable_source[] =
	{
		0x00, 0x00, 0x00, 0x01,
		0x00, 0x02, 0x00, 0x03,
		0x03, 0x03, 0x03, 0x02,
		0x03, 0x01, 0x03, 0x00,
		0x00, 0x00, 0x02, 0x00,
		0x02, 0x01, 0x02, 0x02,
		0x00, 0x10, 0x20, 0x30,
		0x00, 0x04, 0x08, 0x0c,
		0x00, 0x44, 0x48, 0x4c,
		0x00, 0x84, 0x88, 0x8c,
		0x00, 0xc4, 0xc8, 0xcc
	};

	/*
	 * The color PROM is addressed as follows:
	 *
	 *   A0 => PLAYFIELD 1
	 *   A1 => PLAYFIELD 2
	 *   A2 => DRONE 1
	 *   A3 => DRONE 2
	 *   A4 => CAR 1
	 *   A5 => CAR 2
	 *   A6 => DRONE COLOR 1
	 *   A7 => DRONE COLOR 2
	 *   A8 => PLAYFIELD WINDOW
	 *
	 * This driver hard-codes some behavior which actually depends
	 * on the PROM, like priorities, clipping and transparency.
	 *
	 */

	m_color1_mask = m_color2_mask = 0;

	for (i = 0; i < ARRAY_LENGTH(colortable_source); i++)
	{
		UINT8 color = colortable_source[i];

		if (color == 1)
			m_color1_mask |= 1 << i;
		else if (color == 2)
			m_color2_mask |= 1 << i;

		prom_to_palette(i, color_prom[0x100 + colortable_source[i]]);
	}

	palette.set_pen_color(ARRAY_LENGTH(colortable_source) + 0, rgb_t::black);
	palette.set_pen_color(ARRAY_LENGTH(colortable_source) + 1, rgb_t::white);
}


TILE_GET_INFO_MEMBER(firetrk_state::firetrk_get_tile_info1)
{
	int code = m_playfield_ram[tile_index] & 0x3f;
	int color = (m_playfield_ram[tile_index] >> 6) & 0x03;

	if (*m_blink && (code >= 0x04) && (code <= 0x0b))
		color = 0;

	if (m_flash)
		color = color | 0x04;

	SET_TILE_INFO_MEMBER(1, code, color, 0);
}


TILE_GET_INFO_MEMBER(firetrk_state::superbug_get_tile_info1)
{
	int code = m_playfield_ram[tile_index] & 0x3f;
	int color = (m_playfield_ram[tile_index] >> 6) & 0x03;

	if (*m_blink && (code >= 0x08) && (code <= 0x0f))
		color = 0;

	if (m_flash)
		color = color | 0x04;

	SET_TILE_INFO_MEMBER(1, code, color, 0);
}


TILE_GET_INFO_MEMBER(firetrk_state::montecar_get_tile_info1)
{
	int code = m_playfield_ram[tile_index] & 0x3f;
	int color = (m_playfield_ram[tile_index] >> 6) & 0x03;

	if (m_flash)
		color = color | 0x04;

	SET_TILE_INFO_MEMBER(1, code, color, 0);
}


TILE_GET_INFO_MEMBER(firetrk_state::firetrk_get_tile_info2)
{
	UINT8 code = m_playfield_ram[tile_index] & 0x3f;
	int color = 0;

	/* palette 1 for crash and palette 2 for skid */
	if (((code & 0x30) != 0x00) || ((code & 0x0c) == 0x00))
		color = 1;   /* palette 0, 1 */

	if ((code & 0x3c) == 0x0c)
		color = 2;   /* palette 0, 2 */

	SET_TILE_INFO_MEMBER(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(firetrk_state::superbug_get_tile_info2)
{
	UINT8 code = m_playfield_ram[tile_index] & 0x3f;
	int color = 0;

	/* palette 1 for crash and palette 2 for skid */
	if ((code & 0x30) != 0x00)
		color = 1;   /* palette 0, 1 */

	if ((code & 0x38) == 0x00)
		color = 2;   /* palette 0, 2 */

	SET_TILE_INFO_MEMBER(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(firetrk_state::montecar_get_tile_info2)
{
	UINT8 code = m_playfield_ram[tile_index];
	int color = 0;

	/* palette 1 for crash and palette 2 for skid */
	if (((code & 0xc0) == 0x40) || ((code & 0xc0) == 0x80))
		color = 2;   /* palette 2, 1 */

	if ((code & 0xc0) == 0xc0)
		color = 1;   /* palette 2, 0 */

	if ((code & 0xc0) == 0x00)
		color = 3;   /* palette 2, 2 */

	if ((code & 0x30) == 0x30)
		color = 0;   /* palette 0, 0 */

	SET_TILE_INFO_MEMBER(2, code & 0x3f, color, 0);
}


void firetrk_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper1);
	m_screen->register_screen_bitmap(m_helper2);

	m_tilemap1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(firetrk_state::firetrk_get_tile_info1),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_tilemap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(firetrk_state::firetrk_get_tile_info2),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
}


VIDEO_START_MEMBER(firetrk_state,superbug)
{
	m_screen->register_screen_bitmap(m_helper1);
	m_screen->register_screen_bitmap(m_helper2);

	m_tilemap1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(firetrk_state::superbug_get_tile_info1),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_tilemap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(firetrk_state::superbug_get_tile_info2),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
}


VIDEO_START_MEMBER(firetrk_state,montecar)
{
	m_screen->register_screen_bitmap(m_helper1);
	m_screen->register_screen_bitmap(m_helper2);

	m_tilemap1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(firetrk_state::montecar_get_tile_info1),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_tilemap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(firetrk_state::montecar_get_tile_info2),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
}


void firetrk_state::firetrk_draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flash)
{
	int gfx_bank, code, color, flip_x, flip_y, x, y;

	if (which)
	{
		gfx_bank = 5;
		code = *m_drone_rot & 0x07;
		color = flash ? 1 : 0;
		flip_x = *m_drone_rot & 0x08;
		flip_y = *m_drone_rot & 0x10;
		x = (flip_x ? *m_drone_x - 63 : 192 - *m_drone_x) + 36;
		y =  flip_y ? *m_drone_y - 63 : 192 - *m_drone_y;
	}
	else
	{
		gfx_bank = (*m_car_rot & 0x10) ? 4 : 3;
		code = *m_car_rot & 0x03;
		color = flash ? 1 : 0;
		flip_x = *m_car_rot & 0x04;
		flip_y = *m_car_rot & 0x08;
		x = 144;
		y = 104;
	}

		m_gfxdecode->gfx(gfx_bank)->transpen(bitmap,cliprect, code, color, flip_x, flip_y, x, y, 0);
}


void firetrk_state::superbug_draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int flash)
{
	int gfx_bank = (*m_car_rot & 0x10) ? 4 : 3;
	int code = ~*m_car_rot & 0x03;
	int color = flash ? 1 : 0;
	int flip_x = *m_car_rot & 0x04;
	int flip_y = *m_car_rot & 0x08;

		m_gfxdecode->gfx(gfx_bank)->transpen(bitmap,cliprect, code, color, flip_x, flip_y, 144, 104, 0);
}


void firetrk_state::montecar_draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int is_collision_detection)
{
	int gfx_bank, code, color, flip_x, flip_y, x, y;

	if (which)
	{
		gfx_bank = 4;
		code = *m_drone_rot & 0x07;
		color = is_collision_detection ? 0 : (((*m_car_rot & 0x80) >> 6) | ((*m_drone_rot & 0x80) >> 7));
		flip_x = *m_drone_rot & 0x10;
		flip_y = *m_drone_rot & 0x08;
		x = (flip_x ? *m_drone_x - 31 : 224 - *m_drone_x) + 34;
		y =  flip_y ? *m_drone_y - 31 : 224 - *m_drone_y;
	}
	else
	{
		gfx_bank = 3;
		code = *m_car_rot & 0x07;
		color = 0;
		flip_x = *m_car_rot & 0x10;
		flip_y = *m_car_rot & 0x08;
		x = 144;
		y = 104;
	}

		m_gfxdecode->gfx(gfx_bank)->transpen(bitmap,cliprect, code, color, flip_x, flip_y, x, y, 0);
}


void firetrk_state::draw_text(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *alpha_ram,
						int x, int count, int height)
{
	int i;

	for (i = 0; i < count; i++)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, alpha_ram[i], 0, 0, 0, x, i * height);
}


void firetrk_state::check_collision(int which)
{
	int y, x;

	for (y = playfield_window.min_y; y <= playfield_window.max_y; y++)
		for (x = playfield_window.min_x; x <= playfield_window.max_x; x++)
		{
			pen_t a = m_helper1.pix16(y, x);
			pen_t b = m_helper2.pix16(y, x);

			if (b != 0xff && (m_color1_mask >> a) & 1)
				m_crash[which] = 1;

			if (b != 0xff && (m_color2_mask >> a) & 1)
				m_skid[which] = 1;
		}
}


UINT32 firetrk_state::screen_update_firetrk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().mark_all_dirty();
	m_tilemap1->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap2->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap1->set_scrolly(0, *m_scroll_y);
	m_tilemap2->set_scrolly(0, *m_scroll_y);

	bitmap.fill(0, cliprect);
	m_tilemap1->draw(screen, bitmap, playfield_window, 0, 0);
	firetrk_draw_car(bitmap, playfield_window, 0, m_flash);
	firetrk_draw_car(bitmap, playfield_window, 1, m_flash);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x00, 296, 0x10, 0x10);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x10,   8, 0x10, 0x10);

	if (cliprect.max_y == screen.visible_area().max_y)
	{
		m_tilemap2->draw(screen, m_helper1, playfield_window, 0, 0);

		m_helper2.fill(0xff, playfield_window);
		firetrk_draw_car(m_helper2, playfield_window, 0, FALSE);
		check_collision(0);

		m_helper2.fill(0xff, playfield_window);
		firetrk_draw_car(m_helper2, playfield_window, 1, FALSE);
		check_collision(1);

		*m_blink = FALSE;
	}

	return 0;
}


UINT32 firetrk_state::screen_update_superbug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().mark_all_dirty();
	m_tilemap1->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap2->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap1->set_scrolly(0, *m_scroll_y);
	m_tilemap2->set_scrolly(0, *m_scroll_y);

	bitmap.fill(0, cliprect);
	m_tilemap1->draw(screen, bitmap, playfield_window, 0, 0);
	superbug_draw_car(bitmap, playfield_window, m_flash);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x00, 296, 0x10, 0x10);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x10,   8, 0x10, 0x10);

	if (cliprect.max_y == screen.visible_area().max_y)
	{
		m_tilemap2->draw(screen, m_helper1, playfield_window, 0, 0);

		m_helper2.fill(0xff, playfield_window);
		superbug_draw_car(m_helper2, playfield_window, FALSE);
		check_collision(0);

		*m_blink = FALSE;
	}

	return 0;
}


UINT32 firetrk_state::screen_update_montecar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().mark_all_dirty();
	m_tilemap1->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap2->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap1->set_scrolly(0, *m_scroll_y);
	m_tilemap2->set_scrolly(0, *m_scroll_y);

	bitmap.fill(0x2c, cliprect);
	m_tilemap1->draw(screen, bitmap, playfield_window, 0, 0);
	montecar_draw_car(bitmap, playfield_window, 0, FALSE);
	montecar_draw_car(bitmap, playfield_window, 1, FALSE);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x00, 24, 0x20, 0x08);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x20, 16, 0x20, 0x08);

	if (cliprect.max_y == screen.visible_area().max_y)
	{
		m_tilemap2->draw(screen, m_helper1, playfield_window, 0, 0);

		m_helper2.fill(0xff, playfield_window);
		montecar_draw_car(m_helper2, playfield_window, 0, TRUE);
		check_collision(0);

		m_helper2.fill(0xff, playfield_window);
		montecar_draw_car(m_helper2, playfield_window, 1, TRUE);
		check_collision(1);
	}

	return 0;
}
