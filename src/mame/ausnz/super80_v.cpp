// license:BSD-3-Clause
// copyright-holders:Robbbert
/* Super80.c written by Robbbert, 2005-2010. See the driver source for documentation. */



#include "emu.h"
#include "super80.h"



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



void super80_state::screen_vblank_super80m(bool state)
{
	// rising edge
	if (state)
	{
		// if we chose another palette or colour mode, enable it
		m_palette_index = (m_io_config->read() & 0x60) ? 0 : 16;
	}
}

uint32_t super80_state::screen_update_super80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space& program = m_maincpu->space(AS_PROGRAM);

	m_cass_led = BIT(m_portf0, 5);

	const uint8_t options = m_io_config->read();
	bool screen_on = BIT(m_portf0, 2) || !BIT(options, 2);    // bit 2 of port F0 is high, OR user turned on config switch

	uint8_t fg = 0;
	if (screen_on)
	{
		if (BIT(options, 5))
			fg = 15;    // b&w
		else
			fg = 5;     // green
	}

	uint16_t sy = 0;
	uint16_t ma = m_vidpg;
	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = 0; x < 32; x++)    // done this way to avoid x overflowing on page FF
			{
				uint8_t chr = 32;
				if (screen_on)
				{
					chr = program.read_byte(ma | x) & 0x7f;
					if ((chr >= 0x61) && (chr <= 0x7a))
						chr &= 0x1f;
					else
						chr &= 0x3f;
				}

				// get pattern of pixels for that character scanline
				const uint8_t gfx = m_p_chargen[(chr<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)];

				// Display a scanline of a character
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
	address_space& program = m_maincpu->space(AS_PROGRAM);
	m_cass_led = BIT(m_portf0, 5);

	const uint8_t options = m_io_config->read();
	bool screen_on = BIT(m_portf0, 2) || !BIT(options, 2); // bit 2 of port F0 is high, OR user turned on config switch

	uint8_t fg = 0;
	if (screen_on)
	{
		if (BIT(options, 5))
			fg = 15;    // b&w
		else
			fg = 5;     // green
	}

	uint16_t sy = 0;
	uint16_t ma = m_vidpg;
	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = 0; x < 32; x++)
			{
				uint8_t chr = 32;
				if (screen_on)
					chr = program.read_byte(ma | x);

				// get pattern of pixels for that character scanline
				const uint8_t gfx = m_p_chargen[((chr & 0x7f)<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)] ^ ((chr & 0x80) ? 0xff : 0);

				// Display a scanline of a character
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
	address_space& program = m_maincpu->space(AS_PROGRAM);
	m_cass_led = BIT(m_portf0, 5);

	const uint8_t options = m_io_config->read();
	bool screen_on = BIT(m_portf0, 2) || !BIT(options, 2); // bit 2 of port F0 is high, OR user turned on config switch

	uint8_t fg = 0;
	if (screen_on)
	{
		if (BIT(options, 5))
			fg = 15;    // b&w
		else
			fg = 5;     // green
	}

	uint16_t sy = 0;
	uint16_t ma = m_vidpg;
	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = 0; x < 32; x++)
			{
				uint8_t chr = 32;
				if (screen_on)
					chr = program.read_byte(ma | x);

				// get pattern of pixels for that character scanline
				const uint8_t gfx = m_p_chargen[(chr<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)];

				// Display a scanline of a character
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
	address_space& program = m_maincpu->space(AS_PROGRAM);
	m_cass_led = BIT(m_portf0, 5);

	const uint8_t options = m_io_config->read();

	// get selected character generator
	const bool cgen = m_current_charset ^ BIT(options, 4); // bit 0 of port F1 and cgen config switch

	const bool screen_on = BIT(m_portf0, 2) || !BIT(options, 2); // bit 2 of port F0 is high, OR user turned on config switch

	uint8_t fg = 0;
	if (screen_on)
	{
		if (BIT(options, 5))
			fg = 15;    // b&w
		else
			fg = 5;     // green
	}

	uint16_t sy = 0;
	uint16_t ma = m_vidpg;
	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = 0; x < 32; x++)
			{
				uint8_t chr = 32;
				if (screen_on)
					chr = program.read_byte(ma | x);

				uint8_t bg = 0;
				if (!(options & 0x40))
				{
					const uint8_t col = program.read_byte(0xfe00 | ma | x); // byte of colour to display
					fg = m_palette_index + (col & 0x0f);
					bg = m_palette_index + (col >> 4);
				}

				// get pattern of pixels for that character scanline
				uint8_t gfx;
				if (cgen)
					gfx = m_p_chargen[(chr<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)];
				else
					gfx = m_p_chargen[0x1000 | ((chr & 0x7f)<<4) | ((ra & 8) >> 3) | ((ra & 7) << 1)] ^ ((chr & 0x80) ? 0xff : 0);

				// Display a scanline of a character
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


/**************************** I/O PORTS *****************************************************************/

void super80_state::portf1_w(u8 data)
{
	m_vidpg = (data & 0xfe) << 8;
	m_current_charset = BIT(data, 0);
}

/*---------------------------------------------------------------

    Super-80R and Super-80V

---------------------------------------------------------------*/

// we place videoram at 0x0000, colour ram at 0x1000, pcg at 0x2000
u8 super80r_state::low_r(offs_t offset)
{
	return m_vram[offset];
}

void super80r_state::low_w(offs_t offset, u8 data)
{
	m_vram[offset] = data; // video
}

u8 super80r_state::high_r(offs_t offset)
{
	return m_vram[offset+0x0800]; // video
}

void super80r_state::high_w(offs_t offset, u8 data)
{
	m_vram[offset+0x0800] = data; // video
	m_vram[offset+0x2800] = data; // pcg
}

u8 super80v_state::low_r(offs_t offset)
{
	if (BIT(m_portf0, 2))
		return m_vram[offset]; // video
	else
		return m_vram[offset+0x1000]; // colour
}

void super80v_state::low_w(offs_t offset, u8 data)
{
	if (BIT(m_portf0, 2))
		m_vram[offset] = data; // video
	else
		m_vram[offset+0x1000] = data; // colour
}

u8 super80v_state::high_r(offs_t offset)
{
	if (!BIT(m_portf0, 2))
		return m_vram[offset+0x1800]; // colour
	else
	if (BIT(m_portf0, 4))
		return m_vram[offset+0x0800]; // video
	else
		return m_p_chargen[offset]; // char rom
}

void super80v_state::high_w(offs_t offset, u8 data)
{
	if (!BIT(m_portf0, 2))
		m_vram[offset+0x1800] = data; // colour
	else
	{
		m_vram[offset+0x0800] = data; // video

		if (BIT(m_portf0, 4))
			m_vram[offset+0x2800] = data; // pcg
	}
}

uint32_t super80v_state::screen_update_super80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_s_options = m_io_config->read();
	m_cass_led = BIT(m_portf0, 5);
	m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}

MC6845_UPDATE_ROW( super80v_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (uint16_t x = 0; x < x_count; x++)               // for each character
	{
		uint8_t inv = 0;
		if (x == cursor_x) inv=0xff;
		const uint16_t mem = (ma + x) & 0xfff;
		uint8_t chr = m_vram[mem];

		/* get colour or b&w */
		uint8_t fg = 5;            // green
		if ((m_s_options & 0x60) == 0x60)
			fg = 15;       // b&w

		uint8_t bg = 0;
		if (~m_s_options & 0x40)
		{
			const uint8_t col = m_vram[mem+0x1000];                 // byte of colour to display
			fg = m_palette_index + (col & 0x0f);
			bg = m_palette_index + (col >> 4);
		}

		// if inverse mode, replace any pcgram chrs with inverse chrs
		if (!BIT(m_portf0, 4) && BIT(chr, 7))          // is it a high chr in inverse mode
		{
			inv ^= 0xff;                        // invert the chr
			chr &= 0x7f;                        // and drop bit 7
		}

		// get pattern of pixels for that character scanline
		const uint8_t gfx = BIT(chr, 7)
			? m_vram[0x2000 | ((chr << 4) | ra)] ^ inv
			: m_p_chargen[((chr << 4) | ra)] ^ inv;

		// Display a scanline of a character
		*p++ = palette[BIT(gfx, 7) ? fg : bg];
		*p++ = palette[BIT(gfx, 6) ? fg : bg];
		*p++ = palette[BIT(gfx, 5) ? fg : bg];
		*p++ = palette[BIT(gfx, 4) ? fg : bg];
		*p++ = palette[BIT(gfx, 3) ? fg : bg];
		*p++ = palette[BIT(gfx, 2) ? fg : bg];
		*p++ = palette[BIT(gfx, 1) ? fg : bg];
	}
}


