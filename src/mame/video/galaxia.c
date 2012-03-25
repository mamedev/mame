/***************************************************************************

  Galaxia Video HW

  hardware is derived from cvs

***************************************************************************/

#include "emu.h"
#include "video/s2636.h"
#include "includes/galaxia.h"



SCREEN_UPDATE_IND16( galaxia )
{
	galaxia_state *state = screen.machine().driver_data<galaxia_state>();
	int x, y;

	bitmap_ind16 &s2636_0_bitmap = s2636_update(screen.machine().device("s2636_0"), cliprect);
	bitmap_ind16 &s2636_1_bitmap = s2636_update(screen.machine().device("s2636_1"), cliprect);
	bitmap_ind16 &s2636_2_bitmap = s2636_update(screen.machine().device("s2636_2"), cliprect);

	bitmap.fill(0, cliprect);

	// draw background
	for (x = 0; x < 32; x++)
	{
		// fixed scrolling area
		int y_offs = 0;
		if (x >= 4 && x < 24)
			y_offs = state->m_scroll ^ 0xff;

		for (y = 0; y < 32; y++)
		{
			int tile = state->m_video[y << 5 | x];
			int color = state->m_color[y << 5 | x];
			drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[0], tile, color, 0, 0, x*8, (y_offs + y*8) & 0xff, 0);
		}
	}

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// draw bullets (guesswork)
			bool bullet = state->m_bullet[y] && x == (state->m_bullet[y] ^ 0xff);
			bool background = bitmap.pix16(y, x) != 0;
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

	// draw background (no scroll?)
	for (x = 0; x < 32; x++)
	{
		for (y = 0; y < 32; y++)
		{
			int tile = state->m_video[y << 5 | x];
			int color = state->m_color[y << 5 | x];
			drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[0], tile, color, 0, 0, x*8, y*8, 0);
		}
	}

	copybitmap(state->m_collision_bitmap, bitmap, 0, 0, 0, 0, cliprect);

	// copy the S2636 bitmap into the main bitmap and check collision
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// NOTE: similar to zac2650.c, the sprite chip runs at a different frequency than the background generator
			// the exact timing is unknown, so we'll have to do with guesswork (s_offset and s_ratio)
			int s_offset = 7;
			float s_ratio = 256.0 / 196.0;

			float sx = x * s_ratio;
			if ((int)(sx + 0.5) > cliprect.max_x)
				break;

			int pixel = s2636_0_bitmap.pix16(y, x + s_offset);

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. background collision detection
				if (state->m_collision_bitmap.pix16(y, (int)(sx)) || state->m_collision_bitmap.pix16(y, (int)(sx + 0.5)))
					state->m_collision |= 0x01;

				bitmap.pix16(y, (int)(sx)) = S2636_PIXEL_COLOR(pixel);
				bitmap.pix16(y, (int)(sx + 0.5)) = S2636_PIXEL_COLOR(pixel);
			}
		}
	}

	return 0;
}



/********************************************************************************/

VIDEO_START( galaxia )
{
	galaxia_state *state = machine.driver_data<galaxia_state>();
	state->m_color = auto_alloc_array(machine, UINT8, 0x400);

	machine.primary_screen->register_screen_bitmap(state->m_collision_bitmap);
}
