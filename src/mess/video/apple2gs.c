/*********************************************************************

    video/apple2gs.c

    Apple IIgs video code

*********************************************************************/


#include "emu.h"
#include "includes/apple2.h"
#include "includes/apple2gs.h"



VIDEO_START( apple2gs )
{
	apple2gs_state *state = machine.driver_data<apple2gs_state>();
	state->m_bordercolor = 0;
	apple2_video_start(machine, state->m_slowmem, 0x20000, 0, 8);
	state->m_legacy_gfx = auto_bitmap_ind16_alloc(machine, 560, 192);

	state_save_register_item(machine, "BORDERCLR", NULL, 0, state->m_bordercolor);
}



SCREEN_UPDATE_IND16( apple2gs )
{
	apple2gs_state *state = screen.machine().driver_data<apple2gs_state>();
	const UINT8 *vram;
	UINT16 *scanline;
	UINT8 scb, b;
	int col, palette;
	UINT16 last_pixel = 0, pixel;
	int beamy;

	beamy = cliprect.min_y;

	if (state->m_newvideo & 0x80)
	{
		// in top or bottom border?
		if ((beamy < BORDER_TOP) || (beamy >= 200+BORDER_TOP))
		{
			// don't draw past the bottom border
			if (beamy >= 231+BORDER_TOP)
			{
				return 0;
			}

			scanline = &bitmap.pix16(beamy);
			for (col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = state->m_bordercolor;
			}
		}
		else	// regular screen area
		{
			int shrline = beamy - BORDER_TOP;

			scb = state->m_slowmem[0x19D00 + shrline];
			palette = ((scb & 0x0f) << 4) + 16;

			vram = &state->m_slowmem[0x12000 + (shrline * 160)];
			scanline = &bitmap.pix16(beamy);

			// draw left and right borders
			for (col = 0; col < BORDER_LEFT; col++)
			{
				scanline[col] = state->m_bordercolor;
				scanline[col+BORDER_LEFT+640] = state->m_bordercolor;
			}

			if (scb & 0x80)	// 640 mode
			{
				for (col = 0; col < 160; col++)
				{
					b = vram[col];
					scanline[col * 4 + 0 + BORDER_LEFT] = palette +  0 + ((b >> 6) & 0x03);
					scanline[col * 4 + 1 + BORDER_LEFT] = palette +  4 + ((b >> 4) & 0x03);
					scanline[col * 4 + 2 + BORDER_LEFT] = palette +  8 + ((b >> 2) & 0x03);
					scanline[col * 4 + 3 + BORDER_LEFT] = palette + 12 + ((b >> 0) & 0x03);
				}
			}
			else		// 320 mode
			{
				for (col = 0; col < 160; col++)
				{
					b = vram[col];
					pixel = (b >> 4) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 4 + 0 + BORDER_LEFT] = pixel;
					scanline[col * 4 + 1 + BORDER_LEFT] = pixel;

					b = vram[col];
					pixel = (b >> 0) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 4 + 2 + BORDER_LEFT] = pixel;
					scanline[col * 4 + 3 + BORDER_LEFT] = pixel;
				}
			}
		}
	}
	else
	{
		/* call legacy Apple II video rendering at scanline 0 to draw into the off-screen buffer */
		if (beamy == 0)
		{
			apple2_state *a2state = screen.machine().driver_data<apple2_state>();

			// check if DHR should be monochrome 560x192
			if (state->m_newvideo & 0x20)
			{
				a2state->m_monochrome_dhr = true;
			}
			else
			{
				a2state->m_monochrome_dhr = false;
			}

			rectangle new_cliprect(0, 559, 0, 191);
			SCREEN_UPDATE_NAME(apple2)(NULL, screen, *state->m_legacy_gfx, new_cliprect);
		}

		if ((beamy < (BORDER_TOP+4)) || (beamy >= (192+4+BORDER_TOP)))
		{
			if (beamy >= (231+BORDER_TOP))
			{
				return 0;
			}

			scanline = &bitmap.pix16(beamy);
			for (col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = state->m_bordercolor;
			}
		}
		else
		{
			scanline = &bitmap.pix16(beamy);

			// draw left and right borders
			for (col = 0; col < BORDER_LEFT + 40; col++)
			{
				scanline[col] = state->m_bordercolor;
				scanline[col+BORDER_LEFT+600] = state->m_bordercolor;
			}

			memcpy(scanline + 40 + BORDER_LEFT, &state->m_legacy_gfx->pix16(beamy-(BORDER_TOP+4)), 560 * sizeof(UINT16));
		}
	}
	return 0;
}

