// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Dirk Best
/***************************************************************************
 *  Sharp MZ700
 *
 *  video hardware
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Reference: http://sharpmz.computingmuseum.com
 *
 ***************************************************************************/

#include "emu.h"
#include "includes/mz700.h"


UINT32 mz_state::screen_update_mz700(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,gfx,col,bg=0,fg=0,oldcol=0x7e;
	UINT16 sy=0,ma=0,x,chr;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 40; x++)
			{
				col = m_colorram[x];
				if (col != oldcol)
				{
					oldcol = col;
					col = BITSWAP8(col, 7, 3, 4, 6, 5, 0, 2, 1); // turn BRG into RGB
					bg = col & 7;
					fg = (col >> 3) & 7;
				}
				chr = m_videoram[x] | (BIT(col, 7)<<8);
				gfx = m_p_chargen[(chr<<3) | ra ];

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

UINT32 mz_state::screen_update_mz800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
			int x, y;

			for (x = 0; x < 40; x++)
			{
				for (y = 0; y < 200; y++)
				{
					bitmap.pix16(y, x * 8 + 0) = BIT(m_videoram[x * 8 + y], 0) ? 7 : 0;
					bitmap.pix16(y, x * 8 + 1) = BIT(m_videoram[x * 8 + y], 1) ? 7 : 0;
					bitmap.pix16(y, x * 8 + 2) = BIT(m_videoram[x * 8 + y], 2) ? 7 : 0;
					bitmap.pix16(y, x * 8 + 3) = BIT(m_videoram[x * 8 + y], 3) ? 7 : 0;
					bitmap.pix16(y, x * 8 + 4) = BIT(m_videoram[x * 8 + y], 4) ? 7 : 0;
					bitmap.pix16(y, x * 8 + 5) = BIT(m_videoram[x * 8 + y], 5) ? 7 : 0;
					bitmap.pix16(y, x * 8 + 6) = BIT(m_videoram[x * 8 + y], 6) ? 7 : 0;
					bitmap.pix16(y, x * 8 + 7) = BIT(m_videoram[x * 8 + y], 7) ? 7 : 0;
				}
			}
		}

		return 0;
	}
}

/***************************************************************************
    CGRAM
***************************************************************************/

WRITE8_MEMBER(mz_state::mz800_cgram_w)
{
	m_cgram[offset] = data;
}
