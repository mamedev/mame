// license:BSD-3-Clause
// copyright-holders:Robbbert
//***************************************************************************

#include "emu.h"
#include "includes/trs80m3.h"
#include "screen.h"

/* Bit assignment for "m_mode"
    d7 Page select
    d5 enable alternate character set
    d3 Invert characters with bit 7 set (1=invert)
    d2 80/40 or 64/32 characters per line (1=80)
    d0 80/64 or 40/32 characters per line (1=32) */

WRITE8_MEMBER( trs80m3_state::port_88_w )
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


/* 8-bit video, 32/64/40/80 characters per line = trs80m3, trs80m4. */
uint32_t trs80m3_state::screen_update_trs80m3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx,gfxbit;
	uint16_t sy=0,ma=0,x;
	uint8_t skip=1;
	uint8_t cols = BIT(m_mode, 2) ? 80 : 64;
	uint8_t rows = BIT(m_mode, 2) ? 24 : 16;
	uint8_t lines = BIT(m_mode, 2) ? 10 : 12;
	uint8_t s_cols = cols;
	uint8_t mask = BIT(m_mode, 5) ? 0xff : 0xbf;  /* Select Japanese or extended chars */

	if (BIT(m_mode, 0))
	{
		s_cols >>= 1;
		skip = 2;
	}

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode;
		screen.set_visible_area(0, s_cols*8-1, 0, rows*lines-1);
	}

	for (y = 0; y < rows; y++)
	{
		for (ra = 0; ra < lines; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

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

