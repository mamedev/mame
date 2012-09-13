/***************************************************************************

        LLC driver by Miodrag Milanovic

        17/04/2009 Preliminary driver.

****************************************************************************/


#include "includes/llc.h"


void llc_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

SCREEN_UPDATE_IND16( llc1 )
{
	llc_state *state = screen.machine().driver_data<llc_state>();
	UINT8 y,ra,chr,gfx,inv;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				inv = (state->m_p_videoram[x] & 0x80) ? 0xff : 0;
				chr = state->m_p_videoram[x] & 0x7f;

				/* get pattern of pixels for that character scanline */
				gfx = state->m_p_chargen[ chr | (ra << 7) ] ^ inv;

				/* Display a scanline of a character (8 pixels) */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}

SCREEN_UPDATE_IND16( llc2 )
{
	llc_state *state = screen.machine().driver_data<llc_state>();
	UINT8 y,ra,chr,gfx,inv, inv1=state->m_rv ? 0xff : 0;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 32; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			inv = 0;
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				chr = state->m_p_videoram[x];
				if (chr==0x11) // inverse on
				{
					inv=0xff;
					chr=0x0f; // must not show
				}
				else
				if (chr==0x10) // inverse off
					inv=0;

				/* get pattern of pixels for that character scanline */
				gfx = state->m_p_chargen[ (chr << 3) | ra ] ^ inv ^ inv1;

				/* Display a scanline of a character (8 pixels) */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}
