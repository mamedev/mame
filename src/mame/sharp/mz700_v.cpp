// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Dirk Best
/***************************************************************************
 *  Sharp MZ700
 *
 *  video hardware
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Reference: https://original.sharpmz.org/
 *
 ***************************************************************************/

#include "emu.h"
#include "mz700.h"


uint32_t mz_state::screen_update_mz700(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t bg=0,fg=0,oldcol=0x7e;
	uint16_t sy=0,ma=0;

	for (uint8_t y = 0; y < 25; y++)
	{
		for (uint8_t ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 40; x++)
			{
				uint8_t col = m_colorram[x];
				if (col != oldcol)
				{
					oldcol = col;
					col = bitswap<8>(col, 7, 3, 4, 6, 5, 0, 2, 1); // turn BRG into RGB
					bg = col & 7;
					fg = (col >> 3) & 7;
				}
				uint16_t chr = m_videoram[x] | (BIT(col, 7)<<8);
				uint8_t gfx = m_p_chargen[(chr<<3) | ra ];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 0) ? fg : bg;
				*p++ = BIT(gfx, 1) ? fg : bg;
				*p++ = BIT(gfx, 2) ? fg : bg;
				*p++ = BIT(gfx, 3) ? fg : bg;
				*p++ = BIT(gfx, 4) ? fg : bg;
				*p++ = BIT(gfx, 5) ? fg : bg;
				*p++ = BIT(gfx, 6) ? fg : bg;
				*p++ = BIT(gfx, 7) ? fg : bg;
			}
		}
		ma+=40;
	}
	return 0;
}


/***************************************************************************
    MZ-800
    Not working.
***************************************************************************/

uint32_t mz800_state::screen_update_mz800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_mz700_mode)
		return screen_update_mz700(screen, bitmap, cliprect);
	else
	{
		if (m_hires_mode)
		{
		}
		else
		{
			for (int x = 0; x < 40; x++)
			{
				for (int y = 0; y < 200; y++)
				{
					bitmap.pix(y, x * 8 + 0) = BIT(m_videoram[x * 8 + y], 0) ? 7 : 0;
					bitmap.pix(y, x * 8 + 1) = BIT(m_videoram[x * 8 + y], 1) ? 7 : 0;
					bitmap.pix(y, x * 8 + 2) = BIT(m_videoram[x * 8 + y], 2) ? 7 : 0;
					bitmap.pix(y, x * 8 + 3) = BIT(m_videoram[x * 8 + y], 3) ? 7 : 0;
					bitmap.pix(y, x * 8 + 4) = BIT(m_videoram[x * 8 + y], 4) ? 7 : 0;
					bitmap.pix(y, x * 8 + 5) = BIT(m_videoram[x * 8 + y], 5) ? 7 : 0;
					bitmap.pix(y, x * 8 + 6) = BIT(m_videoram[x * 8 + y], 6) ? 7 : 0;
					bitmap.pix(y, x * 8 + 7) = BIT(m_videoram[x * 8 + y], 7) ? 7 : 0;
				}
			}
		}

		return 0;
	}
}

/***************************************************************************
    CGRAM
***************************************************************************/

void mz800_state::mz800_cgram_w(offs_t offset, uint8_t data)
{
	m_cgram[offset] = data;
}
