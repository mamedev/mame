// license:BSD-3-Clause
// copyright-holders:Robbbert

#include "emu.h"
#include "includes/kaypro.h"
#include "screen.h"



/***********************************************************

    Video

************************************************************/

void kaypro_state::kaypro_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0, 220, 0)); // green
	palette.set_pen_color(2, rgb_t(0, 110, 0)); // low intensity green
}

uint32_t kaypro_state::screen_update_kayproii(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/* The display consists of 80 columns and 24 rows. Each row is allocated 128 bytes of ram,
    but only the first 80 are used. The total video ram therefore is 0x0c00 bytes.
    There is one video attribute: bit 7 causes blinking. The first half of the
    character generator is blank, with the visible characters in the 2nd half.
    During the "off" period of blanking, the first half is used. Only 5 pixels are
    connected from the rom to the shift register, the remaining pixels are held high. */

	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;

	m_framecnt++;

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				gfx = 0;

				if (ra < 8)
				{
					chr = m_p_videoram[x]^0x80;

					/* Take care of flashing characters */
					if ((chr < 0x80) && (m_framecnt & 0x08))
						chr |= 0x80;

					/* get pattern of pixels for that character scanline */
					gfx = m_p_chargen[(chr<<3) | ra ];
				}

				/* Display a scanline of a character (7 pixels) */
				*p++ = 0;
				*p++ = BIT( gfx, 4 );
				*p++ = BIT( gfx, 3 );
				*p++ = BIT( gfx, 2 );
				*p++ = BIT( gfx, 1 );
				*p++ = BIT( gfx, 0 );
				*p++ = 0;
			}
		}
		ma+=128;
	}
	return 0;
}

uint32_t kaypro_state::screen_update_omni2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;

	m_framecnt++;

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				gfx = 0;

				if (ra < 8)
				{
					chr = m_p_videoram[x];

					/* Take care of flashing characters */
					if ((chr > 0x7f) && (m_framecnt & 0x08))
						chr |= 0x80;

					/* get pattern of pixels for that character scanline */
					gfx = m_p_chargen[(chr<<3) | ra ];
				}

				/* Display a scanline of a character (7 pixels) */
				*p++ = BIT( gfx, 6 );
				*p++ = BIT( gfx, 5 );
				*p++ = BIT( gfx, 4 );
				*p++ = BIT( gfx, 3 );
				*p++ = BIT( gfx, 2 );
				*p++ = BIT( gfx, 1 );
				*p++ = BIT( gfx, 0 );
			}
		}
		ma+=128;
	}
	return 0;
}

uint32_t kaypro_state::screen_update_kaypro484(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_framecnt++;
	m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}

/* bit 6 of kaypro484_system_port selects alternate characters (A12 on character generator rom).
    The diagram specifies a 2732 with 28 pins, and more address pins. Possibly a 2764 or 27128.
    Since our dump only goes up to A11, the alternate character set doesn't exist.

    0000-07FF of videoram is memory-mapped characters; 0800-0FFF is equivalent attribute bytes.
    d3 Underline
    d2 blinking (at unknown rate)
    d1 low intensity
    d0 reverse video

    Not sure how the attributes interact, for example does an underline blink? */


MC6845_UPDATE_ROW( kaypro_state::kaypro484_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix32(y);
	uint16_t x;
	uint8_t gfx,fg,bg;

	for (x = 0; x < x_count; x++)               // for each character
	{
		uint8_t inv=0;
		if (x == cursor_x) inv=0xff;
		uint16_t mem = (ma + x) & 0x7ff;
		uint8_t chr = m_p_videoram[mem];
		uint8_t attr = m_p_videoram[mem | 0x800];

		if ((attr & 3) == 3)
		{
			fg = 0;
			bg = 2;
		}
		else
		if ((attr & 3) == 2)
		{
			fg = 2;
			bg = 0;
		}
		else
		if ((attr & 3) == 1)
		{
			fg = 0;
			bg = 1;
		}
		else
		{
			fg = 1;
			bg = 0;
		}

		/* Take care of flashing characters */
		if ( (BIT(attr, 2)) & (BIT(m_framecnt, 3)) )
			fg = bg;

		/* get pattern of pixels for that character scanline */
		if ( (ra == 15) & (BIT(attr, 3)) )  /* underline */
			gfx = 0xff;
		else
			gfx = m_p_chargen[(chr<<4) | ra ] ^ inv;

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT( gfx, 7 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 6 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 5 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 4 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 3 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 2 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 1 ) ? fg : bg];
		*p++ = palette[BIT( gfx, 0 ) ? fg : bg];
	}
}

/************************************* MC6845 SUPPORT ROUTINES ***************************************/

/* Resize the screen within the limits of the hardware. Expand the image to fill the screen area.
    Standard screen is 640 x 400 = 0x7d0 bytes. */

void kaypro_state::mc6845_screen_configure()
{
	uint16_t width = m_mc6845_reg[1]*8-1;                         // width in pixels
	uint16_t height = m_mc6845_reg[6]*(m_mc6845_reg[9]+1)-1;                  // height in pixels
	uint16_t bytes = m_mc6845_reg[1]*m_mc6845_reg[6]-1;                       // video ram needed -1

	/* Resize the screen */
	if ((width < 640) && (height < 400) && (bytes < 0x800)) /* bounds checking to prevent an assert or violation */
		m_screen->set_visible_area(0, width, 0, height);
}


/**************************** I/O PORTS *****************************************************************/

READ8_MEMBER( kaypro_state::kaypro484_status_r )
{
/* Need bit 7 high or computer hangs */

	return 0x80 | m_crtc->register_r();
}

WRITE8_MEMBER( kaypro_state::kaypro484_index_w )
{
	m_mc6845_ind = data & 0x1f;
	m_crtc->address_w(data);
}

WRITE8_MEMBER( kaypro_state::kaypro484_register_w )
{
	static const uint8_t mcmask[32]={0xff,0xff,0xff,0x0f,0x7f,0x1f,0x7f,0x7f,3,0x1f,0x7f,0x1f,0x3f,0xff,0x3f,0xff,0,0};

	if (m_mc6845_ind < 16)
		m_mc6845_reg[m_mc6845_ind] = data & mcmask[m_mc6845_ind];   /* save data in register */
	else
		m_mc6845_reg[m_mc6845_ind] = data;

	m_crtc->register_w(data);

	if ((m_mc6845_ind == 1) || (m_mc6845_ind == 6) || (m_mc6845_ind == 9))
		mc6845_screen_configure();          /* adjust screen size */

	if ((m_mc6845_ind > 17) && (m_mc6845_ind < 20))
		m_mc6845_video_address = m_mc6845_reg[19] | ((m_mc6845_reg[18] & 0x3f) << 8);   /* internal ULA address */
}

READ8_MEMBER( kaypro_state::kaypro_videoram_r )
{
	return m_p_videoram[offset];
}

WRITE8_MEMBER( kaypro_state::kaypro_videoram_w )
{
	m_p_videoram[offset] = data;
}

READ8_MEMBER( kaypro_state::kaypro484_videoram_r )
{
	return m_p_videoram[m_mc6845_video_address];
}

WRITE8_MEMBER( kaypro_state::kaypro484_videoram_w )
{
	m_p_videoram[m_mc6845_video_address] = data;
}

VIDEO_START_MEMBER(kaypro_state,kaypro)
{
	m_p_videoram = memregion("roms")->base()+0x3000;
}
