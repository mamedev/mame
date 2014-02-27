// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Model Racing Dribbling hardware

***************************************************************************/

#include "emu.h"
#include "includes/dribling.h"


/*************************************
 *
 *  Convert the palette PROM into
 *  a real palette
 *
 *************************************/

PALETTE_INIT_MEMBER(dribling_state, dribling)
{
	const UINT8 *prom = memregion("proms")->base() + 0x400;
	int i;

	for (i = 0; i < 256; i++)
	{
		int r = (~prom[i] >> 0) & 1;    // 220
		int g = (~prom[i] >> 1) & 3;    // 820 + 560 (332 max)
		int b = (~prom[i] >> 3) & 1;    // 220

		r *= 0xff;
		g *= 0x55;
		b *= 0xff;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}



/*************************************
 *
 *  Color control writes
 *
 *************************************/

WRITE8_MEMBER(dribling_state::dribling_colorram_w)
{
	/* it is very important that we mask off the two bits here */
	m_colorram[offset & 0x1f9f] = data;
}



/*************************************
 *
 *  Video update routine
 *
 *************************************/

UINT32 dribling_state::screen_update_dribling(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *prombase = memregion("proms")->base();
	UINT8 *gfxbase = memregion("gfx1")->base();
	int x, y;

	/* loop over rows */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dst = &bitmap.pix16(y);

		/* loop over columns */
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int b7 = prombase[(x >> 3) | ((y >> 3) << 5)] & 1;
			int b6 = m_abca;
			int b5 = (x >> 3) & 1;
			int b4 = (gfxbase[(x >> 3) | (y << 5)] >> (x & 7)) & 1;
			int b3 = (m_videoram[(x >> 3) | (y << 5)] >> (x & 7)) & 1;
			int b2_0 = m_colorram[(x >> 3) | ((y >> 2) << 7)] & 7;

			/* assemble the various bits into a palette PROM index */
			dst[x] = (b7 << 7) | (b6 << 6) | (b5 << 5) | (b4 << 4) | (b3 << 3) | b2_0;
		}
	}
	return 0;
}
