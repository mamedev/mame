// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Model Racing Dribbling hardware

***************************************************************************/

#include "emu.h"
#include "dribling.h"


/*************************************
 *
 *  Convert the palette PROM into
 *  a real palette
 *
 *************************************/

void dribling_state::palette(palette_device &palette) const
{
	uint8_t const *const prom = memregion("proms")->base() + 0x400;

	for (int i = 0; i < 256; i++)
	{
		int r = (~prom[i] >> 0) & 1;    // 220
		int g = (~prom[i] >> 1) & 3;    // 820 + 560 (332 max)
		int b = (~prom[i] >> 3) & 1;    // 220

		r *= 0xff;
		g *= 0x55;
		b *= 0xff;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}



/*************************************
 *
 *  Color control writes
 *
 *************************************/

void dribling_state::colorram_w(offs_t offset, uint8_t data)
{
	// it is very important that we mask off the two bits here
	m_colorram[offset & 0x1f9f] = data;
}



/*************************************
 *
 *  Video update routine
 *
 *************************************/

uint32_t dribling_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// loop over rows
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t *const dst = &bitmap.pix(y);

		// loop over columns
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int b7 = m_proms[(x >> 3) | ((y >> 3) << 5)] & 1;
			int b6 = m_abca;
			int b5 = (x >> 3) & 1;
			int b4 = (m_gfxroms[(x >> 3) | (y << 5)] >> (x & 7)) & 1;
			int b3 = (m_videoram[(x >> 3) | (y << 5)] >> (x & 7)) & 1;
			int b2_0 = m_colorram[(x >> 3) | ((y >> 2) << 7)] & 7;

			// assemble the various bits into a palette PROM index
			dst[x] = (b7 << 7) | (b6 << 6) | (b5 << 5) | (b4 << 4) | (b3 << 3) | b2_0;
		}
	}
	return 0;
}
