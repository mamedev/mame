// license:BSD-3-Clause
// copyright-holders:David Haywood, hap
/***************************************************************************

  Galaxia Video HW

  hardware is derived from cvs

***************************************************************************/

#include "emu.h"
#include "includes/galaxia.h"

#define SPRITE_PEN_BASE     (0x10)
#define STAR_PEN            (0x18)
#define BULLET_PEN          (0x19)


// Colors are 3bpp, but how they are generated is a mystery
// there's no color prom on the pcb, nor palette ram

PALETTE_INIT_MEMBER(galaxia_state,galaxia)
{
	// estimated with video/photo references
	const int lut_clr[0x18] = {
		// background
		0, 1, 4, 5,
		0, 3, 6, 2,
		0, 1, 4, 5, // unused?
		0, 3, 1, 7,

		// sprites
		0, 4, 3, 6, 1, 5, 2, 7
	};

	for (int i = 0; i < 0x18; i++)
		palette.set_pen_color(i, pal1bit(lut_clr[i] >> 0), pal1bit(lut_clr[i] >> 1), pal1bit(lut_clr[i] >> 2));

	// stars/bullets
	palette.set_pen_color(STAR_PEN, pal1bit(1), pal1bit(1), pal1bit(1));
	palette.set_pen_color(BULLET_PEN, pal1bit(1), pal1bit(1), pal1bit(0));
}

PALETTE_INIT_MEMBER(galaxia_state,astrowar)
{
	// no reference material available(?), except for Data East astrof
	const int lut_clr[8] = { 7, 3, 5, 1, 4, 2, 6, 7 };

	for (int i = 0; i < 8; i++)
	{
		// background
		palette.set_pen_color(i*2, 0, 0, 0);
		palette.set_pen_color(i*2 + 1, pal1bit(lut_clr[i] >> 0), pal1bit(lut_clr[i] >> 1), pal1bit(lut_clr[i] >> 2));

		// sprites
		palette.set_pen_color(i | SPRITE_PEN_BASE, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}

	// stars/bullets
	palette.set_pen_color(STAR_PEN, pal1bit(1), pal1bit(1), pal1bit(1));
	palette.set_pen_color(BULLET_PEN, pal1bit(1), pal1bit(1), pal1bit(0));
}

TILE_GET_INFO_MEMBER(galaxia_state::get_galaxia_bg_tile_info)
{
	UINT8 code = m_video_ram[tile_index] & 0x7f; // d7 unused
	UINT8 color = m_color_ram[tile_index] & 3; // highest bits unused

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(galaxia_state::get_astrowar_bg_tile_info)
{
	UINT8 code = m_video_ram[tile_index];
	UINT8 color = m_color_ram[tile_index] & 7; // highest bits unused

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void galaxia_state::init_common()
{
	assert((STAR_PEN & 7) == 0);
	cvs_init_stars();
}

VIDEO_START_MEMBER(galaxia_state,galaxia)
{
	init_common();

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galaxia_state::get_galaxia_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(8);

}

VIDEO_START_MEMBER(galaxia_state,astrowar)
{
	init_common();

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(galaxia_state::get_astrowar_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(8);
	m_bg_tilemap->set_scrolldx(8, 8);

	m_screen->register_screen_bitmap(m_temp_bitmap);
}


/********************************************************************************/

UINT32 galaxia_state::screen_update_galaxia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	bitmap_ind16 *s2636_0_bitmap = &m_s2636_0->update(cliprect);
	bitmap_ind16 *s2636_1_bitmap = &m_s2636_1->update(cliprect);
	bitmap_ind16 *s2636_2_bitmap = &m_s2636_2->update(cliprect);

	bitmap.fill(0, cliprect);
	cvs_update_stars(bitmap, cliprect, STAR_PEN, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bool bullet = m_bullet_ram[y] && x == (m_bullet_ram[y] ^ 0xff);
			bool background = (bitmap.pix16(y, x) & 3) != 0;

			// draw bullets (guesswork)
			if (bullet)
			{
				// background vs. bullet collision detection
				if (background) m_collision_register |= 0x80;

				// bullet size/color/priority is guessed
				bitmap.pix16(y, x) = BULLET_PEN;
				if (x) bitmap.pix16(y, x-1) = BULLET_PEN;
			}

			// copy the S2636 images into the main bitmap and check collision
			int pixel0 = s2636_0_bitmap->pix16(y, x);
			int pixel1 = s2636_1_bitmap->pix16(y, x);
			int pixel2 = s2636_2_bitmap->pix16(y, x);

			int pixel = pixel0 | pixel1 | pixel2;

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. S2636 collision detection
				if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel1)) m_collision_register |= 0x01;
				if (S2636_IS_PIXEL_DRAWN(pixel1) && S2636_IS_PIXEL_DRAWN(pixel2)) m_collision_register |= 0x02;
				if (S2636_IS_PIXEL_DRAWN(pixel2) && S2636_IS_PIXEL_DRAWN(pixel0)) m_collision_register |= 0x04;

				// S2636 vs. bullet collision detection
				if (bullet) m_collision_register |= 0x08;

				// S2636 vs. background collision detection
				if (background)
				{
					/* bit4 causes problems on 2nd level
					if (S2636_IS_PIXEL_DRAWN(pixel0)) m_collision_register |= 0x10; */
					if (S2636_IS_PIXEL_DRAWN(pixel1)) m_collision_register |= 0x20;
					if (S2636_IS_PIXEL_DRAWN(pixel2)) m_collision_register |= 0x40;
				}

				bitmap.pix16(y, x) = S2636_PIXEL_COLOR(pixel) | SPRITE_PEN_BASE;
			}
		}
	}

	return 0;
}


UINT32 galaxia_state::screen_update_astrowar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// astrowar has only one S2636
	int x, y;

	bitmap_ind16 &s2636_0_bitmap = m_s2636_0->update(cliprect);

	bitmap.fill(0, cliprect);
	cvs_update_stars(bitmap, cliprect, STAR_PEN, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	copybitmap(m_temp_bitmap, bitmap, 0, 0, 0, 0, cliprect);

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		// draw bullets (guesswork)
		if (m_bullet_ram[y])
		{
			UINT8 pos = m_bullet_ram[y] ^ 0xff;

			// background vs. bullet collision detection
			if (m_temp_bitmap.pix16(y, pos) & 1)
				m_collision_register |= 0x02;

			// bullet size/color/priority is guessed
			bitmap.pix16(y, pos) = BULLET_PEN;
			if (pos) bitmap.pix16(y, pos-1) = BULLET_PEN;
		}

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// NOTE: similar to zac2650.c, the sprite chip runs at a different frequency than the background generator
			// the exact timing ratio is unknown, so we'll have to do with guesswork
			float s_ratio = 256.0f / 196.0f;

			float sx = x * s_ratio;
			if ((int)(sx + 0.5f) > cliprect.max_x)
				break;

			// copy the S2636 bitmap into the main bitmap and check collision
			int pixel = s2636_0_bitmap.pix16(y, x);

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. background collision detection
				if ((m_temp_bitmap.pix16(y, (int)(sx)) | m_temp_bitmap.pix16(y, (int)(sx + 0.5f))) & 1)
					m_collision_register |= 0x01;

				bitmap.pix16(y, (int)(sx)) = S2636_PIXEL_COLOR(pixel) | SPRITE_PEN_BASE;
				bitmap.pix16(y, (int)(sx + 0.5f)) = S2636_PIXEL_COLOR(pixel) | SPRITE_PEN_BASE;
			}
		}
	}

	return 0;
}
