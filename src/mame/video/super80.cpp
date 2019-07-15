// license:BSD-3-Clause
// copyright-holders:Robbbert
/* Super80.c written by Robbbert, 2005-2010. See the driver source for documentation. */

/* Notes on using MAME MC6845 Device (MMD).
    1. Speed of MMD is about 20% slower than pre-MMD coding
    2. Undocumented cursor start and end-lines is not supported by MMD, so we do it here. */


#include "emu.h"
#include "includes/super80.h"



/**************************** PALETTES for super80m and super80v ******************************************/

void super80_state::super80m_palette(palette_device &palette) const
{
	// RGB
	palette.set_pen_color( 0, rgb_t(0x00, 0x00, 0x00));   //  0 Black
	palette.set_pen_color( 1, rgb_t(0x00, 0x00, 0x00));   //  1 Black
	palette.set_pen_color( 2, rgb_t(0x00, 0x00, 0x7f));   //  2 Blue
	palette.set_pen_color( 3, rgb_t(0x00, 0x00, 0xff));   //  3 Light Blue
	palette.set_pen_color( 4, rgb_t(0x00, 0x7f, 0x00));   //  4 Green
	palette.set_pen_color( 5, rgb_t(0x00, 0xff, 0x00));   //  5 Bright Green
	palette.set_pen_color( 6, rgb_t(0x00, 0x7f, 0x7f));   //  6 Cyan
	palette.set_pen_color( 7, rgb_t(0x00, 0xff, 0xff));   //  7 Turquoise
	palette.set_pen_color( 8, rgb_t(0x7f, 0x00, 0x00));   //  8 Dark Red
	palette.set_pen_color( 9, rgb_t(0xff, 0x00, 0x00));   //  9 Red
	palette.set_pen_color(10, rgb_t(0x7f, 0x00, 0x7f));   // 10 Purple
	palette.set_pen_color(11, rgb_t(0xff, 0x00, 0xff));   // 11 Magenta
	palette.set_pen_color(12, rgb_t(0x7f, 0x7f, 0x00));   // 12 Lime
	palette.set_pen_color(13, rgb_t(0xff, 0xff, 0x00));   // 13 Yellow
	palette.set_pen_color(14, rgb_t(0xbf, 0xbf, 0xbf));   // 14 Off White
	palette.set_pen_color(15, rgb_t(0xff, 0xff, 0xff));   // 15 White
	// Composite
	palette.set_pen_color(16, rgb_t(0x00, 0x00, 0x00));   //  0 Black
	palette.set_pen_color(17, rgb_t(0x80, 0x80, 0x80));   //  1 Grey
	palette.set_pen_color(18, rgb_t(0x00, 0x00, 0xff));   //  2 Blue
	palette.set_pen_color(19, rgb_t(0xff, 0xff, 0x80));   //  3 Light Yellow
	palette.set_pen_color(20, rgb_t(0x00, 0xff, 0x00));   //  4 Green
	palette.set_pen_color(21, rgb_t(0xff, 0x80, 0xff));   //  5 Light Magenta
	palette.set_pen_color(22, rgb_t(0x00, 0xff, 0xff));   //  6 Cyan
	palette.set_pen_color(23, rgb_t(0xff, 0x40, 0x40));   //  7 Light Red
	palette.set_pen_color(24, rgb_t(0xff, 0x00, 0x00));   //  8 Red
	palette.set_pen_color(25, rgb_t(0x00, 0x80, 0x80));   //  9 Dark Cyan
	palette.set_pen_color(26, rgb_t(0xff, 0x00, 0xff));   // 10 Magenta
	palette.set_pen_color(27, rgb_t(0x80, 0xff, 0x80));   // 11 Light Green
	palette.set_pen_color(28, rgb_t(0xff, 0xff, 0x00));   // 12 Yellow
	palette.set_pen_color(29, rgb_t(0x00, 0x00, 0x80));   // 13 Dark Blue
	palette.set_pen_color(30, rgb_t(0xff, 0xff, 0xff));   // 14 White
	palette.set_pen_color(31, rgb_t(0x00, 0x00, 0x00));   // 15 Black
}



WRITE_LINE_MEMBER(super80_state::screen_vblank_super80m)
{
	// rising edge
	if (state)
	{
		/* if we chose another palette or colour mode, enable it */
		m_palette_index = (m_io_config->read() & 0x60) ? 0 : 16;
	}
}

uint32_t super80_state::screen_update_super80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_cass_led = BIT(m_portf0, 5);

	const uint8_t options = m_io_config->read();
	bool screen_on = BIT(m_portf0, 2) || !BIT(options, 2);    /* bit 2 of port F0 is high, OR user turned on config switch */

	uint8_t fg = 0;
	if (screen_on)
	{
		if (BIT(options, 5))
			fg = 15;    /* b&w */
		else
			fg = 5;     /* green */
	}

	uint16_t sy = 0;
	uint16_t ma = m_vidpg;
	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (uint16_t x = 0; x < 32; x++)    // done this way to avoid x overflowing on page FF
			{
				uint8_t chr = 32;
				if (screen_on)
				{
					chr = m_p_ram[ma | x] & 0x7f;
					if ((chr >= 0x61) && (chr <= 0x7a))
						chr &= 0x1f;
					else
						chr &= 0x3f;
				}

				/* get pattern of pixels for that character scanline */
				const uint8_t gfx = m_p_chargen[(chr<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : 0;
				*p++ = BIT(gfx, 6) ? fg : 0;
				*p++ = BIT(gfx, 5) ? fg : 0;
				*p++ = BIT(gfx, 4) ? fg : 0;
				*p++ = BIT(gfx, 3) ? fg : 0;
				*p++ = BIT(gfx, 2) ? fg : 0;
				*p++ = BIT(gfx, 1) ? fg : 0;
				*p++ = BIT(gfx, 0) ? fg : 0;
			}
		}
		ma += 32;
	}
	return 0;
}

uint32_t super80_state::screen_update_super80d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_cass_led = BIT(m_portf0, 5);

	const uint8_t options = m_io_config->read();
	bool screen_on = BIT(m_portf0, 2) || !BIT(options, 2); /* bit 2 of port F0 is high, OR user turned on config switch */

	uint8_t fg = 0;
	if (screen_on)
	{
		if (BIT(options, 5))
			fg = 15;    /* b&w */
		else
			fg = 5;     /* green */
	}

	uint16_t sy = 0;
	uint16_t ma = m_vidpg;
	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (uint16_t x = 0; x < 32; x++)
			{
				uint8_t chr = 32;
				if (screen_on)
					chr = m_p_ram[ma | x];

				/* get pattern of pixels for that character scanline */
				const uint8_t gfx = m_p_chargen[((chr & 0x7f)<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)] ^ ((chr & 0x80) ? 0xff : 0);

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : 0;
				*p++ = BIT(gfx, 6) ? fg : 0;
				*p++ = BIT(gfx, 5) ? fg : 0;
				*p++ = BIT(gfx, 4) ? fg : 0;
				*p++ = BIT(gfx, 3) ? fg : 0;
				*p++ = BIT(gfx, 2) ? fg : 0;
				*p++ = BIT(gfx, 1) ? fg : 0;
				*p++ = BIT(gfx, 0) ? fg : 0;
			}
		}
		ma+=32;
	}
	return 0;
}

uint32_t super80_state::screen_update_super80e(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_cass_led = BIT(m_portf0, 5);

	const uint8_t options = m_io_config->read();
	bool screen_on = BIT(m_portf0, 2) || !BIT(options, 2); /* bit 2 of port F0 is high, OR user turned on config switch */

	uint8_t fg = 0;
	if (screen_on)
	{
		if (BIT(options, 5))
			fg = 15;    /* b&w */
		else
			fg = 5;     /* green */
	}

	uint16_t sy = 0;
	uint16_t ma = m_vidpg;
	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (uint16_t x = 0; x < 32; x++)
			{
				uint8_t chr = 32;
				if (screen_on)
					chr = m_p_ram[ma | x];

				/* get pattern of pixels for that character scanline */
				const uint8_t gfx = m_p_chargen[(chr<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : 0;
				*p++ = BIT(gfx, 6) ? fg : 0;
				*p++ = BIT(gfx, 5) ? fg : 0;
				*p++ = BIT(gfx, 4) ? fg : 0;
				*p++ = BIT(gfx, 3) ? fg : 0;
				*p++ = BIT(gfx, 2) ? fg : 0;
				*p++ = BIT(gfx, 1) ? fg : 0;
				*p++ = BIT(gfx, 0) ? fg : 0;
			}
		}
		ma+=32;
	}
	return 0;
}

uint32_t super80_state::screen_update_super80m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_cass_led = BIT(m_portf0, 5);

	const uint8_t options = m_io_config->read();

	/* get selected character generator */
	const uint8_t cgen = m_current_charset ^ ((options & 0x10) >> 4); /* bit 0 of port F1 and cgen config switch */

	const bool screen_on = BIT(m_portf0, 2) || !BIT(options, 2); /* bit 2 of port F0 is high, OR user turned on config switch */

	uint8_t fg = 0;
	if (screen_on)
	{
		if (BIT(options, 5))
			fg = 15;    /* b&w */
		else
			fg = 5;     /* green */
	}

	uint16_t sy = 0;
	uint16_t ma = m_vidpg;
	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (uint16_t x = 0; x < 32; x++)
			{
				uint8_t chr = 32;
				if (screen_on)
					chr = m_p_ram[ma | x];

				uint8_t bg = 0;
				if (!(options & 0x40))
				{
					const uint8_t col = m_p_ram[0xfe00 | ma | x]; /* byte of colour to display */
					fg = m_palette_index + (col & 0x0f);
					bg = m_palette_index + (col >> 4);
				}

				/* get pattern of pixels for that character scanline */
				uint8_t gfx;
				if (cgen)
					gfx = m_p_chargen[(chr<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)];
				else
					gfx = m_p_chargen[0x1000 | ((chr & 0x7f)<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)] ^ ((chr & 0x80) ? 0xff : 0);

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : bg;
				*p++ = BIT(gfx, 6) ? fg : bg;
				*p++ = BIT(gfx, 5) ? fg : bg;
				*p++ = BIT(gfx, 4) ? fg : bg;
				*p++ = BIT(gfx, 3) ? fg : bg;
				*p++ = BIT(gfx, 2) ? fg : bg;
				*p++ = BIT(gfx, 1) ? fg : bg;
				*p++ = BIT(gfx, 0) ? fg : bg;
			}
		}
		ma+=32;
	}
	return 0;
}

VIDEO_START_MEMBER(super80_state,super80)
{
	m_vidpg = 0xfe00;
}

/**************************** I/O PORTS *****************************************************************/

WRITE8_MEMBER( super80_state::super80_f1_w )
{
	m_vidpg = (data & 0xfe) << 8;
	m_current_charset = data & 1;
}

/*---------------------------------------------------------------

    Super-80R and Super-80V

---------------------------------------------------------------*/

static const uint8_t mc6845_mask[32]={0xff,0xff,0xff,0x0f,0x7f,0x1f,0x7f,0x7f,3,0x1f,0x7f,0x1f,0x3f,0xff,0x3f,0xff,0,0};

READ8_MEMBER( super80_state::super80v_low_r )
{
	if (BIT(m_portf0, 2))
		return m_p_videoram[offset];
	else
		return m_p_colorram[offset];
}

WRITE8_MEMBER( super80_state::super80v_low_w )
{
	if (BIT(m_portf0, 2))
		m_p_videoram[offset] = data;
	else
		m_p_colorram[offset] = data;
}

READ8_MEMBER( super80_state::super80v_high_r )
{
	if (!BIT(m_portf0, 2))
		return m_p_colorram[0x800 | offset];
	else
	if (BIT(m_portf0, 4))
		return m_p_ram[0xf800 | offset];
	else
		return m_p_ram[0xf000 | offset];
}

WRITE8_MEMBER( super80_state::super80v_high_w )
{
	if (!BIT(m_portf0, 2))
		m_p_colorram[0x800 | offset] = data;
	else
	{
		m_p_videoram[0x800 | offset] = data;

		if (BIT(m_portf0, 4))
			m_p_ram[0xf800 | offset] = data;
	}
}

/* The 6845 can produce a variety of cursor shapes - all are emulated here - remove when mame fixed */
void super80_state::mc6845_cursor_configure()
{
	/* curs_type holds the general cursor shape to be created
	    0 = no cursor
	    1 = partial cursor (only shows on a block of scan lines)
	    2 = full cursor
	    3 = two-part cursor (has a part at the top and bottom with the middle blank) */

	for (uint8_t i = 0; i < ARRAY_LENGTH(m_mc6845_cursor); i++) m_mc6845_cursor[i] = 0;        // prepare cursor by erasing old one

	uint8_t r9  = m_mc6845_reg[9];                  // number of scan lines - 1
	uint8_t r10 = m_mc6845_reg[10] & 0x1f;              // cursor start line = last 5 bits
	uint8_t r11 = m_mc6845_reg[11]+1;                   // cursor end line incremented to suit for-loops below

	/* decide the curs_type by examining the registers */
	uint8_t curs_type = 0;
	if (r10 < r11)
		curs_type=1;             // start less than end, show start to end
	else if (r10 == r11)
		curs_type=2;                // if equal, show full cursor
	else
		curs_type=3;                   // if start greater than end, it's a two-part cursor

	if ((r11 - 1) > r9) curs_type=2;            // if end greater than scan-lines, show full cursor
	if (r10 > r9) curs_type=0;              // if start greater than scan-lines, then no cursor
	if (r11 > 16) r11=16;                   // truncate 5-bit register to fit our 4-bit hardware

	/* create the new cursor */
	if (curs_type > 1)
		for (uint8_t i = 0; i < ARRAY_LENGTH(m_mc6845_cursor); i++)
			m_mc6845_cursor[i] = 0xff; // turn on full cursor

	if (curs_type == 1)
		for (uint8_t i = r10; i < r11; i++)
			m_mc6845_cursor[i] = 0xff; // for each line that should show, turn on that scan line

	if (curs_type == 3)
		for (uint8_t i = r11; i < r10; i++)
			m_mc6845_cursor[i] = 0; // now take a bite out of the middle
}

uint32_t super80_state::screen_update_super80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_s_options = m_io_config->read();
	m_cass_led = BIT(m_portf0, 5);
	m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}

MC6845_UPDATE_ROW( super80_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix32(y);

	for (uint16_t x = 0; x < x_count; x++)               // for each character
	{
		uint8_t inv = 0;
		//      if (x == cursor_x) inv=0xff;    /* uncomment when mame fixed */
		const uint16_t mem = (ma + x) & 0xfff;
		uint8_t chr = m_p_videoram[mem];

		/* get colour or b&w */
		uint8_t fg = 5;            /* green */
		if ((m_s_options & 0x60) == 0x60)
			fg = 15;       /* b&w */

		uint8_t bg = 0;
		if (~m_s_options & 0x40)
		{
			const uint8_t col = m_p_colorram[mem];                 /* byte of colour to display */
			fg = m_palette_index + (col & 0x0f);
			bg = m_palette_index + (col >> 4);
		}

		/* if inverse mode, replace any pcgram chrs with inverse chrs */
		if (!BIT(m_portf0, 4) && (chr & 0x80))          // is it a high chr in inverse mode
		{
			inv ^= 0xff;                        // invert the chr
			chr &= 0x7f;                        // and drop bit 7
		}

		/* process cursor */
		if (x == cursor_x)
			inv ^= m_mc6845_cursor[ra];

		/* get pattern of pixels for that character scanline */
		const uint8_t gfx = m_p_ram[0xf000 | ((chr << 4) | ra)] ^ inv;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 7) ? fg : bg];
		*p++ = palette[BIT(gfx, 6) ? fg : bg];
		*p++ = palette[BIT(gfx, 5) ? fg : bg];
		*p++ = palette[BIT(gfx, 4) ? fg : bg];
		*p++ = palette[BIT(gfx, 3) ? fg : bg];
		*p++ = palette[BIT(gfx, 2) ? fg : bg];
		*p++ = palette[BIT(gfx, 1) ? fg : bg];
	}
}

/**************************** I/O PORTS *****************************************************************/

WRITE8_MEMBER( super80_state::super80v_10_w )
{
	data &= 0x1f;
	m_mc6845_ind = data;
	m_crtc->address_w(data);
}

WRITE8_MEMBER( super80_state::super80v_11_w )
{
	m_mc6845_reg[m_mc6845_ind] = data & mc6845_mask[m_mc6845_ind];  /* save data in register */
	m_crtc->register_w(data);
	if ((m_mc6845_ind > 8) && (m_mc6845_ind < 12)) mc6845_cursor_configure();       /* adjust cursor shape - remove when mame fixed */
}
