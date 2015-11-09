// license:BSD-3-Clause
// copyright-holders:Robbbert and unknown others
/***************************************************************************

  trs80.c

  Functions to emulate the video hardware of the TRS80.

***************************************************************************/

#include "includes/trs80.h"

/* Bit assignment for "state->m_mode"
    d7 Page select
    d6 LNW80 switch to graphics ram
    d5 LNW80 colour or monochrome (1=colour)
    d4 LNW80 lores or hires (1=hires) also does 64 or 80 chars per line
    d3 LNW80 invert entire screen / Model III/4 Invert characters with bit 7 set (1=invert)
    d2 80/40 or 64/32 characters per line (1=80)
    d1 7 or 8 bit video (1=requires 7-bit, 0=don't care)
    d0 80/64 or 40/32 characters per line (1=32) */


WRITE8_MEMBER( trs80_state::trs80m4_88_w )
{
/* This is for the programming of the CRTC registers.
    However this CRTC is mask-programmed, and only the
    start address register can be used. The cursor and
    light-pen facilities are ignored. The character clock
    is changed depending on the screen size chosen.
    Therefore it is easier to use normal
    coding rather than the mc6845 device. */

	if (!offset) m_crtc_reg = data & 0x1f;

	if (offset) switch (m_crtc_reg)
	{
		case 12:
			m_start_address = (m_start_address & 0x00ff) | (data << 8);
			break;
		case 13:
			m_start_address = (m_start_address & 0xff00) | data;
	}
}


void trs80_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
	m_size_store = 0xff;
	m_mode &= 2;
}


/* 7 or 8-bit video, 32/64 characters per line = trs80, trs80l2, sys80 */
UINT32 trs80_state::screen_update_trs80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx,gfxbit;
	UINT16 sy=0,ma=0,x;
	UINT8 cols = BIT(m_mode, 0) ? 32 : 64;
	UINT8 skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode & 1;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 12; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

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

/* 8-bit video, 32/64/40/80 characters per line = trs80m3, trs80m4. */
UINT32 trs80_state::screen_update_trs80m4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx,gfxbit;
	UINT16 sy=0,ma=0,x;
	UINT8 skip=1;
	UINT8 cols = BIT(m_mode, 2) ? 80 : 64;
	UINT8 rows = BIT(m_mode, 2) ? 24 : 16;
	UINT8 lines = BIT(m_mode, 2) ? 10 : 12;
	UINT8 s_cols = cols;
	UINT8 mask = BIT(m_mode, 5) ? 0xff : 0xbf;  /* Select Japanese or extended chars */

	if (m_mode & 1)
	{
		s_cols >>= 1;
		skip = 2;
	}

	if ((m_mode & 0x7f) != m_size_store)
	{
		m_size_store = m_mode & 5;
		screen.set_visible_area(0, s_cols*8-1, 0, rows*lines-1);
	}

	for (y = 0; y < rows; y++)
	{
		for (ra = 0; ra < lines; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + cols; x+=skip)
			{
				chr = m_p_videoram[x+m_start_address];

				if (((chr & 0xc0) == 0xc0) && (~m_mode & 8))
				{
					if (ra < 8)
						gfx = m_p_chargen[((chr&mask)<<3) | ra ];
					else
						gfx = 0;

					*p++ = BIT(gfx, 7);
					*p++ = BIT(gfx, 6);
					*p++ = BIT(gfx, 5);
					*p++ = BIT(gfx, 4);
					*p++ = BIT(gfx, 3);
					*p++ = BIT(gfx, 2);
					*p++ = BIT(gfx, 1);
					*p++ = BIT(gfx, 0);
				}
				else
				if ((chr & 0x80) && (~m_mode & 8))
				{
					gfxbit = (ra & 0x0c)>>1;
					/* Display one line of a lores character */
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					gfxbit++;
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
					*p++ = BIT(chr, gfxbit);
				}
				else
				{
					/* get pattern of pixels for that character scanline */
					if (ra < 8)
						gfx = m_p_chargen[((chr&0x7f)<<3) | ra ];
					else
						gfx = 0;

					/* if inverse mode, and bit 7 set, invert gfx */
					if (BIT(m_mode, 3) & BIT(chr, 7))
						gfx ^= 0xff;

					/* Display a scanline of a character */
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
		}
		ma+=cols;
	}
	return 0;
}

/* 7 or 8-bit video, 64/32 characters per line = ht1080z, ht1080z2, ht108064 */
UINT32 trs80_state::screen_update_ht1080z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx,gfxbit;
	UINT16 sy=0,ma=0,x;
	UINT8 cols = BIT(m_mode, 0) ? 32 : 64;
	UINT8 skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode & 1;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 12; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

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
					if ((m_mode & 2) && (chr < 32)) chr+=64;

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
UINT32 trs80_state::screen_update_lnw80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const UINT16 rows[] = { 0, 0x200, 0x100, 0x300, 1, 0x201, 0x101, 0x301 };
	UINT8 chr,gfx,gfxbit,bg=7,fg=0;
	UINT16 sy=0,ma=0,x,y,ra;
	UINT8 cols = BIT(m_mode, 4) ? 80 : 64;

	/* Although the OS can select 32-character mode, it is not supported by hardware */
	if (m_mode != m_size_store)
	{
		m_size_store = m_mode & 0x10;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	if (m_mode & 8)
	{
		bg = 0;
		fg = 7;
	}

	switch (m_mode & 0x30)
	{
		case 0:                 // MODE 0
			for (y = 0; y < 16; y++)
			{
				for (ra = 0; ra < 12; ra++)
				{
					UINT16 *p = &bitmap.pix16(sy++);

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

		case 0x10:                  // MODE 1
			for (y = 0; y < 0x400; y+=0x40)
			{
				for (ra = 0; ra < 0x3000; ra+=0x400)
				{
					UINT16 *p = &bitmap.pix16(sy++);

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

		case 0x20:                  // MODE 2
			/* it seems the text video ram can have an effect in this mode,
			    not explained clearly, so not emulated */
			for (y = 0; y < 0x400; y+=0x40)
			{
				for (ra = 0; ra < 0x3000; ra+=0x400)
				{
					UINT16 *p = &bitmap.pix16(sy++);

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

		case 0x30:                  // MODE 3
			/* the manual does not explain at all how colour is determined
			    for the extended area. Further, the background colour
			    is not mentioned anywhere. Black is assumed. */
			for (y = 0; y < 0x400; y+=0x40)
			{
				for (ra = 0; ra < 0x3000; ra+=0x400)
				{
					UINT16 *p = &bitmap.pix16(sy++);

					for (x = 0; x < 0x40; x++)
					{
						gfx = m_p_gfxram[ y | x | ra];
						fg = (m_p_videoram[ 0x3c00 | x | y ] & 0x38) >> 3;
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
UINT32 trs80_state::screen_update_radionic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;
	UINT8 cols = BIT(m_mode, 0) ? 32 : 64;
	UINT8 skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode & 1;
		screen.set_visible_area(0, cols*8-1, 0, 16*16-1);
	}

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 16; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

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

UINT32 trs80_state::screen_update_meritum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
/* lores characters are in the character generator. Each character is 6x11. */
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;
	UINT8 cols = BIT(m_mode, 0) ? 32 : 64;
	UINT8 skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode;
		screen.set_visible_area(0, cols*6-1, 0, 16*11-1);
	}

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 11; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x+=skip)
			{
				chr = m_p_videoram[x];

				/* get pattern of pixels for that character scanline */
				gfx = m_p_chargen[(chr<<4) | ra];

				/* Display a scanline of a character (6 pixels) */
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



/***************************************************************************
  Write to video ram
***************************************************************************/

READ8_MEMBER( trs80_state::trs80_videoram_r )
{
	if ((m_mode & 0x80) && (~m_model4 & 1)) offset |= 0x400;
	return m_p_videoram[offset];
}

WRITE8_MEMBER( trs80_state::trs80_videoram_w )
{
	if ((m_mode & 0x80) && (~m_model4 & 1)) offset |= 0x400;
	m_p_videoram[offset] = data;
}


/***************************************************************************
  Write to graphics ram
***************************************************************************/

READ8_MEMBER( trs80_state::trs80_gfxram_r )
{
	return m_p_gfxram[offset];
}

WRITE8_MEMBER( trs80_state::trs80_gfxram_w )
{
	m_p_gfxram[offset] = data;
}


/***************************************************************************
  Palettes
***************************************************************************/

/* Levels are unknown - guessing */
static const rgb_t lnw80_palette[] =
{
	rgb_t(220, 220, 220), // white
	rgb_t(0, 175, 0), // green
	rgb_t(200, 200, 0), // yellow
	rgb_t(255, 0, 0), // red
	rgb_t(255, 0, 255), // magenta
	rgb_t(0, 0, 175), // blue
	rgb_t(0, 255, 255), // cyan
	rgb_t(0, 0, 0), // black
};

PALETTE_INIT_MEMBER(trs80_state,lnw80)
{
	palette.set_pen_colors(0, lnw80_palette, ARRAY_LENGTH(lnw80_palette));
}
