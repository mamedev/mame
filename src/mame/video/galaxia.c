/***************************************************************************

  Galaxia Video HW

  hardware is derived from cvs

***************************************************************************/

#include "emu.h"
#include "video/s2636.h"
#include "includes/galaxia.h"


PALETTE_INIT( galaxia )
{
	for (int i = 0; i < 0x100 ; i++)
	{
		// 1bpp is correct, but there should be more permutations
		palette_set_color_rgb(machine, i, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	galaxia_state *state = machine.driver_data<galaxia_state>();
	UINT8 code = state->m_video[tile_index];
	UINT8 attr = state->m_color[tile_index];
	UINT8 color = attr;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( galaxia )
{
	galaxia_state *state = machine.driver_data<galaxia_state>();
	state->m_color = auto_alloc_array(machine, UINT8, 0x400);
	
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_scroll_cols(8);

	machine.primary_screen->register_screen_bitmap(state->m_temp_bitmap);
}


/********************************************************************************/

SCREEN_UPDATE_IND16( galaxia )
{
	galaxia_state *state = screen.machine().driver_data<galaxia_state>();
	int x, y;

	bitmap_ind16 &s2636_0_bitmap = s2636_update(screen.machine().device("s2636_0"), cliprect);
	bitmap_ind16 &s2636_1_bitmap = s2636_update(screen.machine().device("s2636_1"), cliprect);
	bitmap_ind16 &s2636_2_bitmap = s2636_update(screen.machine().device("s2636_2"), cliprect);

	bitmap.fill(0, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bool bullet = state->m_bullet[y] && x == (state->m_bullet[y] ^ 0xff);
			bool background = (bitmap.pix16(y, x) & 7) != 0;

			// draw bullets (guesswork)
			if (bullet)
			{
				// background vs. bullet collision detection
				if (background) state->m_collision |= 0x80;

				// bullet size/color/priority is guessed
				bitmap.pix16(y, x) = 7;
				if (x) bitmap.pix16(y, x-1) = 7;
			}

			// copy the S2636 images into the main bitmap and check collision
			int pixel0 = s2636_0_bitmap.pix16(y, x);
			int pixel1 = s2636_1_bitmap.pix16(y, x);
			int pixel2 = s2636_2_bitmap.pix16(y, x);

			int pixel = pixel0 | pixel1 | pixel2;

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. S2636 collision detection
				if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel1)) state->m_collision |= 0x01;
				if (S2636_IS_PIXEL_DRAWN(pixel1) && S2636_IS_PIXEL_DRAWN(pixel2)) state->m_collision |= 0x02;
				if (S2636_IS_PIXEL_DRAWN(pixel2) && S2636_IS_PIXEL_DRAWN(pixel0)) state->m_collision |= 0x04;

				// S2636 vs. bullet collision detection
				if (bullet) state->m_collision |= 0x08;

				// S2636 vs. background collision detection
				if (background)
				{
					/* bit4 causes problems on 2nd level
                    if (S2636_IS_PIXEL_DRAWN(pixel0)) state->m_collision |= 0x10; */
					if (S2636_IS_PIXEL_DRAWN(pixel1)) state->m_collision |= 0x20;
					if (S2636_IS_PIXEL_DRAWN(pixel2)) state->m_collision |= 0x40;
				}

				bitmap.pix16(y, x) = S2636_PIXEL_COLOR(pixel);
			}
		}
	}

	return 0;
}


SCREEN_UPDATE_IND16( astrowar )
{
	// astrowar has only one S2636
	galaxia_state *state = screen.machine().driver_data<galaxia_state>();
	int x, y;

	bitmap_ind16 &s2636_0_bitmap = s2636_update(screen.machine().device("s2636_0"), cliprect);

	bitmap.fill(0, cliprect);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	copybitmap(state->m_temp_bitmap, bitmap, 0, 0, 0, 0, cliprect);

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		// draw bullets (guesswork)
		if (state->m_bullet[y])
		{
			UINT8 pos = (state->m_bullet[y] ^ 0xff) - 8;

			// background vs. bullet collision detection
			if (state->m_temp_bitmap.pix16(y, pos) & 7)
				state->m_collision |= 0x02;

			// bullet size/color/priority is guessed
			bitmap.pix16(y, pos) = 7;
			if (pos) bitmap.pix16(y, pos-1) = 7;
		}

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// NOTE: similar to zac2650.c, the sprite chip runs at a different frequency than the background generator
			// the exact timing is unknown, so we'll have to do with guesswork (s_offset and s_ratio)
			int s_offset = 7;
			float s_ratio = 256.0 / 196.0;

			float sx = x * s_ratio;
			if ((int)(sx + 0.5) > cliprect.max_x)
				break;

			// copy the S2636 bitmap into the main bitmap and check collision
			int pixel = s2636_0_bitmap.pix16(y, x + s_offset);

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. background collision detection
				if ((state->m_temp_bitmap.pix16(y, (int)(sx)) | state->m_temp_bitmap.pix16(y, (int)(sx + 0.5))) & 7)
					state->m_collision |= 0x01;

				bitmap.pix16(y, (int)(sx)) = S2636_PIXEL_COLOR(pixel);
				bitmap.pix16(y, (int)(sx + 0.5)) = S2636_PIXEL_COLOR(pixel);
			}
		}
	}

	return 0;
}
