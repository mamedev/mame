// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Robbbert
//***************************************************************************

#include "emu.h"
#include "includes/trs80.h"
#include "screen.h"

/* Bit assignment for "m_mode"
    d1 7 or 8 bit video (1=requires 7-bit, 0=don't care)
    d0 80/64 or 40/32 characters per line (1=32) */


/* 7 or 8-bit video, 32/64 characters per line = trs80, trs80l2, sys80 */
uint32_t trs80_state::screen_update_trs80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=0;
	uint8_t const cols = BIT(m_mode, 0) ? 32 : 64;
	uint8_t const skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 12; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 64; x+=skip)
			{
				uint8_t chr = m_p_videoram[x];

				if (chr & 0x80)
				{
					uint8_t gfxbit = (ra & 0x0c)>>1;
					/* Display one line of a lores character (6 pixels) */
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					gfxbit++;
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
				}
				else
				{
					if (BIT(m_mode, 1) & (chr < 32)) chr+=64;

					// if g,j,p,q,y; lower the descender
					uint8_t gfx;
					if ((chr==0x2c)||(chr==0x3b)||(chr==0x67)||(chr==0x6a)||(chr==0x70)||(chr==0x71)||(chr==0x79))
					{
						if ((ra < 10) && (ra > 1))
							gfx = m_p_chargen[(chr<<3) | (ra-2) ];
						else
							gfx = 0;
					}
					else
					{
						if (ra < 8)
							gfx = m_p_chargen[(chr<<3) | ra ];
						else
							gfx = 0;
					}

					/* Display a scanline of a character (6 pixels) */
					*p++ = BIT(gfx, 5);
					*p++ = BIT(gfx, 4);
					*p++ = BIT(gfx, 3);
					*p++ = BIT(gfx, 2);
					*p++ = BIT(gfx, 1);
					*p++ = BIT(gfx, 0);
				}
			}
		}
		ma+=64;
	}
	return 0;
}


/* 7 or 8-bit video, 64/32 characters per line = ht1080z, ht1080z2, ht108064 */
uint32_t trs80_state::screen_update_ht1080z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=0;
	uint8_t const cols = BIT(m_mode, 0) ? 32 : 64;
	uint8_t const skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 12; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 64; x+=skip)
			{
				uint8_t chr = m_p_videoram[x];

				if (chr & 0x80)
				{
					uint8_t gfxbit = (ra & 0x0c)>>1;
					/* Display one line of a lores character (6 pixels) */
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					gfxbit++;
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
				}
				else
				{
					if (BIT(m_mode, 1) && (chr < 32)) chr+=64;

					/* get pattern of pixels for that character scanline */
					uint8_t gfx = m_p_chargen[(chr<<4) | ra ];

					/* Display a scanline of a character (6 pixels) */
					*p++ = BIT(gfx, 7);
					*p++ = BIT(gfx, 6);
					*p++ = BIT(gfx, 5);
					*p++ = BIT(gfx, 4);
					*p++ = BIT(gfx, 3);
					*p++ = 0;   /* fix for ht108064 */
				}
			}
		}
		ma+=64;
	}
	return 0;
}

