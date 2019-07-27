// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Roberto Fresca
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/truco.h"

void truco_state::truco_palette(palette_device &palette) const
{
	for (int i = 0; i < palette.entries(); i++)
	{
		int r = (i & 0x8) ? 0xff : 0x00;
		int g = (i & 0x4) ? 0xff : 0x00;
		int b = (i & 0x2) ? 0xff : 0x00;

		int const dim = (i & 0x1);

		if (dim)
		{
			r >>= 1;
			g >>= 1;
			b >>= 1;
		}

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}

uint32_t truco_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *videoram = m_videoram;

	for (int y = 0; y < 192; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			int const pixel = (videoram[x >> 1] >> ((x & 1) ? 0 : 4)) & 0x0f;

			bitmap.pix32(y, x) = m_palette->pen(pixel);
		}

		videoram += 0x80;
	}
	return 0;
}
