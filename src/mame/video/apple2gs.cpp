// license:BSD-3-Clause
// copyright-holders:Nathan Woods,R. Belmont
/*********************************************************************

    video/apple2gs.c

    Apple IIgs video code

*********************************************************************/


#include "emu.h"
#include "includes/apple2.h"
#include "includes/apple2gs.h"



VIDEO_START_MEMBER(apple2gs_state,apple2gs)
{
	m_bordercolor = 0;
	apple2_video_start(m_slowmem, m_slowmem+0x10000, 0, 8);
	m_legacy_gfx = std::make_unique<bitmap_ind16>(560, 192);

	save_item(m_bordercolor, "BORDERCLR");
	save_item(NAME(m_fgcolor));
	save_item(NAME(m_bgcolor));
}



UINT32 apple2gs_state::screen_update_apple2gs(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const UINT8 *vram;
	UINT32 *scanline;
	UINT8 scb, b;
	int col, palette;
	UINT32 last_pixel = 0, pixel;
	int beamy;
	UINT16 *a2pixel;

	beamy = cliprect.min_y;

	if (m_newvideo & 0x80)
	{
		// in top or bottom border?
		if ((beamy < BORDER_TOP) || (beamy >= 200+BORDER_TOP))
		{
			// don't draw past the bottom border
			if (beamy >= 231+BORDER_TOP)
			{
				return 0;
			}

			scanline = &bitmap.pix32(beamy);
			for (col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = m_a2_palette[m_bordercolor];
			}
		}
		else    // regular screen area
		{
			int shrline = beamy - BORDER_TOP;

			scb = m_slowmem[0x19D00 + shrline];
			palette = ((scb & 0x0f) << 4);

			vram = &m_slowmem[0x12000 + (shrline * 160)];
			scanline = &bitmap.pix32(beamy);

			// draw left and right borders
			for (col = 0; col < BORDER_LEFT; col++)
			{
				scanline[col] = m_a2_palette[m_bordercolor];
				scanline[col+BORDER_LEFT+640] = m_a2_palette[m_bordercolor];
			}

			if (scb & 0x80) // 640 mode
			{
				for (col = 0; col < 160; col++)
				{
					b = vram[col];
					scanline[col * 4 + 0 + BORDER_LEFT] = m_shr_palette[palette +  0 + ((b >> 6) & 0x03)];
					scanline[col * 4 + 1 + BORDER_LEFT] = m_shr_palette[palette +  4 + ((b >> 4) & 0x03)];
					scanline[col * 4 + 2 + BORDER_LEFT] = m_shr_palette[palette +  8 + ((b >> 2) & 0x03)];
					scanline[col * 4 + 3 + BORDER_LEFT] = m_shr_palette[palette + 12 + ((b >> 0) & 0x03)];
				}
			}
			else        // 320 mode
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
					scanline[col * 4 + 0 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 4 + 1 + BORDER_LEFT] = m_shr_palette[pixel];

					b = vram[col];
					pixel = (b >> 0) & 0x0f;

					if ((scb & 0x20) && !pixel)
						pixel = last_pixel;
					else
						last_pixel = pixel;
					pixel += palette;
					scanline[col * 4 + 2 + BORDER_LEFT] = m_shr_palette[pixel];
					scanline[col * 4 + 3 + BORDER_LEFT] = m_shr_palette[pixel];
				}
			}
		}
	}
	else
	{
		/* call legacy Apple II video rendering at scanline 0 to draw into the off-screen buffer */
		if (beamy == 0)
		{
			// check if DHR should be monochrome 560x192
			if (m_newvideo & 0x20)
			{
				m_monochrome_dhr = true;
			}
			else
			{
				m_monochrome_dhr = false;
			}

			rectangle new_cliprect(0, 559, 0, 191);
			screen_update_apple2(screen, *m_legacy_gfx, new_cliprect);
		}

		if ((beamy < (BORDER_TOP+4)) || (beamy >= (192+4+BORDER_TOP)))
		{
			if (beamy >= (231+BORDER_TOP))
			{
				return 0;
			}

			scanline = &bitmap.pix32(beamy);
			for (col = 0; col < BORDER_LEFT+BORDER_RIGHT+640; col++)
			{
				scanline[col] = m_a2_palette[m_bordercolor];
			}
		}
		else
		{
			scanline = &bitmap.pix32(beamy);

			// draw left and right borders
			for (col = 0; col < BORDER_LEFT + 40; col++)
			{
				scanline[col] = m_a2_palette[m_bordercolor];
				scanline[col+BORDER_LEFT+600] = m_a2_palette[m_bordercolor];
			}

			a2pixel = &m_legacy_gfx->pix16(beamy-(BORDER_TOP+4));
			for (int x = 0; x < 560; x++)
			{
				scanline[40 + BORDER_LEFT + x] = m_a2_palette[*a2pixel++];
			}
		}
	}
	return 0;
}
