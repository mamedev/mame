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
	uint8_t y,ra,chr,gfx,gfxbit;
	uint16_t sy=0,ma=0,x;
	uint8_t cols = BIT(m_mode, 0) ? 32 : 64;
	uint8_t skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 12; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x+=skip)
			{
				chr = m_p_videoram[x];

				if (chr & 0x80)
				{
					gfxbit = (ra & 0x0c)>>1;
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
	uint8_t y,ra,chr,gfx,gfxbit;
	uint16_t sy=0,ma=0,x;
	uint8_t cols = BIT(m_mode, 0) ? 32 : 64;
	uint8_t skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 12; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x+=skip)
			{
				chr = m_p_videoram[x];

				if (chr & 0x80)
				{
					gfxbit = (ra & 0x0c)>>1;
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
					gfx = m_p_chargen[(chr<<4) | ra ];

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

/* 8-bit video, 64/80 characters per line = lnw80 */
uint32_t trs80_state::screen_update_lnw80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const uint16_t rows[] = { 0, 0x200, 0x100, 0x300, 1, 0x201, 0x101, 0x301 };
	uint8_t chr,gfx,gfxbit,bg=7,fg=0;
	uint16_t sy=0,ma=0,x,y,ra;
	uint8_t cols = BIT(m_lnw_mode, 1) ? 80 : 64;

	/* Although the OS can select 32-character mode, it is not supported by hardware */
	if (m_lnw_mode != m_size_store)
	{
		m_size_store = m_lnw_mode;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	if (BIT(m_lnw_mode, 1))
	{
		bg = 0;
		fg = 7;
	}

	switch (m_lnw_mode & 0x06)
	{
		case 0:                 // MODE 0
			for (y = 0; y < 16; y++)
			{
				for (ra = 0; ra < 12; ra++)
				{
					uint16_t *p = &bitmap.pix16(sy++);

					for (x = ma; x < ma + 64; x++)
					{
						chr = m_p_videoram[x];

						if (chr & 0x80)
						{
							gfxbit = (ra & 0x0c)>>1;
							/* Display one line of a lores character (6 pixels) */
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							gfxbit++;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
						}
						else
						{
							/* get pattern of pixels for that character scanline */
							if (ra < 8)
								gfx = m_p_chargen[(chr<<1) | rows[ra] ];
							else
								gfx = 0;

							/* Display a scanline of a character (6 pixels) */
							*p++ = BIT(gfx, 2) ? fg : bg;
							*p++ = BIT(gfx, 1) ? fg : bg;
							*p++ = BIT(gfx, 6) ? fg : bg;
							*p++ = BIT(gfx, 7) ? fg : bg;
							*p++ = BIT(gfx, 5) ? fg : bg;
							*p++ = BIT(gfx, 3) ? fg : bg;
						}
					}
				}
				ma+=64;
			}
			break;

		case 0x02:                  // MODE 1
			for (y = 0; y < 0x400; y+=0x40)
			{
				for (ra = 0; ra < 0x3000; ra+=0x400)
				{
					uint16_t *p = &bitmap.pix16(sy++);

					for (x = 0; x < 0x40; x++)
					{
						gfx = m_p_gfxram[ y | x | ra];
						/* Display 6 pixels in normal region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}

					for (x = 0; x < 0x10; x++)
					{
						gfx = m_p_gfxram[ 0x3000 | x | (ra & 0xc00) | ((ra & 0x3000) >> 8)];
						/* Display 6 pixels in extended region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}
				}
			}
			break;

		case 0x04:                  // MODE 2
			/* it seems the text video ram can have an effect in this mode,
			    not explained clearly, so not emulated */
			for (y = 0; y < 0x400; y+=0x40)
			{
				for (ra = 0; ra < 0x3000; ra+=0x400)
				{
					uint16_t *p = &bitmap.pix16(sy++);

					for (x = 0; x < 0x40; x++)
					{
						gfx = m_p_gfxram[ y | x | ra];
						/* Display 6 pixels in normal region */
						fg = (gfx & 0x38) >> 3;
						*p++ = fg;
						*p++ = fg;
						*p++ = fg;
						fg = gfx & 0x07;
						*p++ = fg;
						*p++ = fg;
						*p++ = fg;
					}
				}
			}
			break;

		case 0x06:                  // MODE 3
			/* the manual does not explain at all how colour is determined
			    for the extended area. Further, the background colour
			    is not mentioned anywhere. Black is assumed. */
			for (y = 0; y < 0x400; y+=0x40)
			{
				for (ra = 0; ra < 0x3000; ra+=0x400)
				{
					uint16_t *p = &bitmap.pix16(sy++);

					for (x = 0; x < 0x40; x++)
					{
						gfx = m_p_gfxram[ y | x | ra];
						fg = (m_p_videoram[ x | y ] & 0x38) >> 3;
						/* Display 6 pixels in normal region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						fg = m_p_videoram[ 0x3c00 | x | y ] & 0x07;
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}

					for (x = 0; x < 0x10; x++)
					{
						gfx = m_p_gfxram[ 0x3000 | x | (ra & 0xc00) | ((ra & 0x3000) >> 8)];
						fg = (m_p_gfxram[ 0x3c00 | x | y ] & 0x38) >> 3;
						/* Display 6 pixels in extended region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						fg = m_p_gfxram[ 0x3c00 | x | y ] & 0x07;
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}
				}
			}
			break;
	}
	return 0;
}

/* lores characters are in the character generator. Each character is 8x16. */
uint32_t trs80_state::screen_update_radionic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;
	uint8_t cols = BIT(m_mode, 0) ? 32 : 64;
	uint8_t skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode;
		screen.set_visible_area(0, cols*8-1, 0, 16*16-1);
	}

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 16; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x+=skip)
			{
				chr = m_p_videoram[x];

				/* get pattern of pixels for that character scanline */
				gfx = m_p_chargen[(chr<<3) | (ra & 7) | (ra & 8) << 8];

				/* Display a scanline of a character (8 pixels) */
				*p++ = BIT(gfx, 0);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 7);
			}
		}
		ma+=64;
	}
	return 0;
}


/***************************************************************************
  Palettes
***************************************************************************/

/* Levels are unknown - guessing */
static constexpr rgb_t lnw80_pens[] =
{
	{ 220, 220, 220 }, // white
	{   0, 175,   0 }, // green
	{ 200, 200,   0 }, // yellow
	{ 255,   0,   0 }, // red
	{ 255,   0, 255 }, // magenta
	{   0,   0, 175 }, // blue
	{   0, 255, 255 }, // cyan
	{   0,   0,   0 }  // black
};

void trs80_state::lnw80_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, lnw80_pens);
}
