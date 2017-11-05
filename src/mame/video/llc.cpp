// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        LLC driver by Miodrag Milanovic

        17/04/2009 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/llc.h"


uint32_t llc_state::screen_update_llc1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx,inv;
	uint16_t sy=0,ma=0,x;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				inv = (m_p_videoram[x] & 0x80) ? 0xff : 0;
				chr = m_p_videoram[x] & 0x7f;

				/* get pattern of pixels for that character scanline */
				gfx = m_p_chargen[ chr | (ra << 7) ] ^ inv;

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

uint32_t llc_state::screen_update_llc2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx,inv, inv1=m_rv ? 0xff : 0;
	uint16_t sy=0,ma=0,x;

	for (y = 0; y < 32; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			inv = 0;
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				chr = m_p_videoram[x];
				if (chr==0x11) // inverse on
				{
					inv=0xff;
					chr=0x0f; // must not show
				}
				else
				if (chr==0x10) // inverse off
					inv=0;

				/* get pattern of pixels for that character scanline */
				gfx = m_p_chargen[ (chr << 3) | ra ] ^ inv ^ inv1;

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
