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

PALETTE_INIT( dribling )
{
	const UINT8 *prom = machine.region("proms")->base() + 0x400;
	int i;

	for (i = 0; i < 256; i++)
	{
		int r = (~prom[i] >> 0) & 1;	// 220
		int g = (~prom[i] >> 1) & 3;	// 820 + 560 (332 max)
		int b = (~prom[i] >> 3) & 1;	// 220

		r *= 0xff;
		g *= 0x55;
		b *= 0xff;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
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

SCREEN_UPDATE_IND16( dribling )
{
	dribling_state *state = screen.machine().driver_data<dribling_state>();
	UINT8 *prombase = screen.machine().region("proms")->base();
	UINT8 *gfxbase = screen.machine().region("gfx1")->base();
	int x, y;

	/* loop over rows */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dst = &bitmap.pix16(y);

		/* loop over columns */
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int b7 = prombase[(x >> 3) | ((y >> 3) << 5)] & 1;
			int b6 = state->m_abca;
			int b5 = (x >> 3) & 1;
			int b4 = (gfxbase[(x >> 3) | (y << 5)] >> (x & 7)) & 1;
			int b3 = (state->m_videoram[(x >> 3) | (y << 5)] >> (x & 7)) & 1;
			int b2_0 = state->m_colorram[(x >> 3) | ((y >> 2) << 7)] & 7;

			/* assemble the various bits into a palette PROM index */
			dst[x] = (b7 << 7) | (b6 << 6) | (b5 << 5) | (b4 << 4) | (b3 << 3) | b2_0;
		}
	}
	return 0;
}
