// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona

/*

RM 380Z video code

*/

#include "emu.h"
#include "rm380z.h"

INPUT_CHANGED_MEMBER(rm380z_state::monitor_changed)
{
	// re-calculate HRG palette values from scratchpad
	for (int c=0; c < 16; c++)
	{
		change_palette(c, m_hrg_scratchpad[c]);
	}
}

void rm380z_state::change_palette(int index, uint8_t value)
{
	rgb_t new_colour;

	if (m_io_display_type->read() & 0x01)
	{
		// value is intensity for a monochrome display
		new_colour = rgb_t(value, value, value);
	}
	else
	{
		// for colour displays value is in the format GRGBRGBR
		uint8_t red = (BIT(value, 6) << 7) | (BIT(value, 3) << 6) | (BIT(value, 0) << 5);
		uint8_t green = (BIT(value, 7) << 4) | (BIT(value, 5) << 3) | (BIT(value, 2) << 2);
		uint8_t blue = (BIT(value, 4) << 1) | BIT(value, 1);
		new_colour = raw_to_rgb_converter::standard_rgb_decoder<3, 3, 2, 5, 2, 0>(red | green | blue);
	}

	m_palette->set_pen_color(index + 3, new_colour);
}

void rm380z_state::palette_init(palette_device &palette) const
{
	// text display palette (black, grey (dim), and white)
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0xc0, 0xc0, 0xc0));
	palette.set_pen_color(2, rgb_t::white());

	// HRG palette (initialise to all black)
	for (int c=3; c < 19; c++)
	{
		palette.set_pen_color(c, rgb_t::black());
	}
}

void rm380z_state::change_hrg_scratchpad(int index, uint8_t value, uint8_t mask)
{
	m_hrg_scratchpad[index] &= mask;
	m_hrg_scratchpad[index] |= value;

	change_palette(index, m_hrg_scratchpad[index]);
}

bool rm380z_state::get_rowcol_from_offset(int& row, int& col, offs_t offset) const
{
	if (m_videomode == RM380Z_VIDEOMODE_40COL)
	{
		col = offset & 0x3f;			// the 6 least significant bits give the column (0-39)
		row = offset >> 6;				// next 5 bits give the row (0-23)
	}
	else
	{
		col = offset & 0x7f;			// the 7 least significant bits give the column (0-79)
		row = offset >> 7;				// next bit gives bit 0 of row
		row |= (m_fbfe & 0x0f) << 1;	// the remaining 4 row bits come from the lower half of PORT 1
	}

	return ((row < RM380Z_SCREENROWS) && (col < RM380Z_SCREENCOLS));
}

void rm380z_state::put_point(int charnum, int x, int y, int col)
{
	const int mx = (y == 6) ? 4 : 3;

	for (unsigned int r = y; r< (y + mx); r++)
	{
		for (unsigned int c = x; c < (x + 3); c++)
		{
			m_graphic_chars[charnum][c + (r * (RM380Z_CHDIMX + 1))] = col;
		}
	}
}

void rm380z_state::init_graphic_chars()
{
	for (int c=0;c<0x3f;c++)
	{
		if (c&0x01) put_point(c,0,0,1);
		else                put_point(c,0,0,0);

		if (c&0x02) put_point(c,3,0,1);
		else                put_point(c,3,0,0);

		if (c&0x04) put_point(c,0,3,1);
		else                put_point(c,0,3,0);

		if (c&0x08) put_point(c,3,3,1);
		else                put_point(c,3,3,0);

		if (c&0x10) put_point(c,0,6,1);
		else                put_point(c,0,6,0);

		if (c&0x20) put_point(c,3,6,1);
		else                put_point(c,3,6,0);
	}
}

void rm380z_state::config_videomode()
{
	int old_mode = m_videomode;

	if (m_port0 & 0x20 & m_port0_mask)
	{
		// 80 cols
		m_videomode = RM380Z_VIDEOMODE_80COL;
	}
	else
	{
		// 40 cols
		m_videomode = RM380Z_VIDEOMODE_40COL;
	}

	if (m_videomode != old_mode)
	{
		if (m_videomode == RM380Z_VIDEOMODE_80COL)
		{
			m_screen->set_size(640, 240);
		}
		else
		{
			m_screen->set_size(320, 240);
		}
		m_screen->set_visarea_full();
	}
}

// char attribute bits in COS 4.0

// 0=alternate charset
// 1=underline
// 2=dim
// 3=reverse


void rm380z_state::decode_videoram_char(int row, int col, uint8_t& chr, uint8_t &attrib)
{
	uint8_t ch1 = m_vram.get_char(row, col);
	uint8_t ch2 = m_vram.get_attrib(row, col);

	// "special" (unknown) cases first
	if ((ch1 == 0x80) && (ch2 == 0x04))
	{
		// blank out
		chr = 0x20;
		attrib = 0;
		return;
	}
	else if ((ch1 == 0) && (ch2 == 8))
	{
		// cursor
		chr = 0x20;
		attrib = 8;
		return;
	}
	else if ((ch1 == 4) && (ch2 == 4))
	{
		// reversed cursor?
		chr = 0x20;
		attrib = 0;
		return;
	}
	else if ((ch1 == 4) && (ch2 == 8))
	{
		// normal cursor
		chr = 0x20;
		attrib = 8;
		return;
	}
	else
	{
		chr = ch1;
		attrib = ch2;

		//printf("unhandled character combination [%x][%x]\n", ch1, ch2);
	}
}

// after ctrl-L (clear screen?): routine at EBBD is executed
// EB30??? next line?
// memory at FF02 seems to hold the line counter (same as FBFD)
//
// basics:
// 20e2: prints "Ready:"
// 0195: prints "\n"

void rm380z_state::videoram_write(offs_t offset, uint8_t data)
{
	if (m_hrg_port0 & 0x04)
	{
		// write to HRG memory
		int index = (m_hrg_port1 & 0x0f) * 1280 + offset;
		m_hrg_ram[index] = data;
	}
	else
	{
		int row, col;
		if (get_rowcol_from_offset(row, col, offset))
		{
			// we suppose videoram is being written as character/attribute couple
			// fbfc 6th bit set=attribute, unset=char
			if (m_port0 & 0x40)
			{
				m_vram.set_attrib(row, col, data);
			}
			else
			{
				m_vram.set_char(row, col, data);
			}
		}
		// else out of bounds write had no effect (see VTOUT description in firmware guide)
	}
}

uint8_t rm380z_state::videoram_read(offs_t offset)
{
	uint8_t data = 0; // return 0 if out of bounds (see VTIN description in firmware guide)

	if (m_hrg_port0 & 0x04)
	{
		// read from HRG memory
		int index = (m_hrg_port1 & 0x0f) * 1280 + offset;
		data = m_hrg_ram[index];
	}
	else
	{
		int row, col;
		if (get_rowcol_from_offset(row, col, offset))
		{
			if (m_port0 & 0x40)
			{
				data = m_vram.get_attrib(row, col);
			}
			else
			{
				data = m_vram.get_char(row, col);
			}
		}
	}

	return data; 
}

void rm380z_state::putChar_vdu80(int charnum, int attribs, int x, int y, bitmap_ind16 &bitmap)
{
	const bool attrUnder = attribs & 0x02;
	const bool attrDim = attribs & 0x04;
	const bool attrRev = attribs & 0x08;
	
	int data_pos = (charnum % 128) * 16;

	for (int r=0; r < 10; r++, data_pos++)
	{
		uint8_t data;

		if (attrUnder && (r == 8))
		{
			// rows 11 and 12 of char data replace rows 9 and 10 to underline
			data_pos += 2;
		}

		if (charnum < 128)
		{
			data = m_chargen[data_pos];
		}
		else
		{
			data = m_user_defined_chars[data_pos];
		}

		for (int c=0; c < 8; c++, data <<= 1)
		{
			uint8_t pixel_value = (data & 0x80) ? 2 : 0;
			if (attrRev)
			{
				pixel_value = !pixel_value;
			}
			if (attrDim && pixel_value)
			{
				pixel_value = 1;
			}
			if (pixel_value)
			{
				bitmap.pix(y * 10 + r, x * 8 + c) = pixel_value;
			}
		}
	}
}

void rm380z_state::putChar_vdu40(int charnum, int x, int y, bitmap_ind16 &bitmap)
{
	if ((charnum > 0) && (charnum <= 0x7f))
	{
		// normal chars (base set)
		int basex=RM380Z_CHDIMX*(charnum/RM380Z_NCY);
		int basey=RM380Z_CHDIMY*(charnum%RM380Z_NCY);

		for (int r=0;r<RM380Z_CHDIMY;r++)
		{
			for (int c=0;c<RM380Z_CHDIMX;c++)
			{
				uint8_t chval = (m_chargen[((basey + r) * RM380Z_CHDIMX * RM380Z_NCX) + basex + c] == 0xff) ? 0 : 2;
				bitmap.pix(y * (RM380Z_CHDIMY+1) + r, x * (RM380Z_CHDIMX+1) + c) = chval;
			}
		}
	}
	else
	{
		// graphic chars
		for (int r=0;r<RM380Z_CHDIMY;r++)
		{
			for (int c=0;c<RM380Z_CHDIMX;c++)
			{
				uint8_t pixel_value = m_graphic_chars[charnum&0x3f][c + r * (RM380Z_CHDIMX+1)];
				if ((charnum >= 0xc0) && pixel_value)
				{
					// chars 0x80 to 0xbf are grey, chars 0xc0 to 0xff are white
					pixel_value = 2;
				}
				bitmap.pix(y * (RM380Z_CHDIMY+1) + r, x * (RM380Z_CHDIMX+1) +c) = pixel_value;
			}
		}
	}
}

void rm380z_state::draw_high_res_graphics(bitmap_ind16 &bitmap)
{
	const int pw = (m_videomode == RM380Z_VIDEOMODE_40COL) ? 1 : 2;
	const int ph = 1;

	for (int y = 0; y < 192; y++)
	{
		for (int x = 0; x < 320; x+= 4)
		{
			int index = ((y / 16) * 1280) + ((x / 4) << 4) + (y % 16);
			uint8_t data = m_hrg_ram[index];
			for (int c=0; c < 4; c++, data >>= 2)
			{
				bitmap.plot_box((x+c)*pw, y*ph, pw, ph, (data & 0x03) + 3);
			}
		}
	}
}

void rm380z_state::draw_medium_res_graphics(bitmap_ind16 &bitmap)
{
	const int page = (display_mode == HRG_MEDIUM_0) ? 0 : 1;
	const int pw = (m_videomode == RM380Z_VIDEOMODE_40COL) ? 2 : 4;
	const int ph = 2;

	for (int y = 0; y < 96; y++)
	{
		for (int x = 0; x < 160; x+= 2)
		{
			int index = ((y / 8) * 1280) + ((x / 2) << 4) + ((y % 8) << 1) + page;
			uint8_t data = m_hrg_ram[index];
			bitmap.plot_box(x*pw, y*ph, pw, ph, ((data & 0x03) | ((data >> 2) & 0x0c)) + 3);
			bitmap.plot_box((x+1)*pw, y*ph, pw, ph, (((data >> 2) & 0x03) | ((data >> 4) & 0x0c)) + 3);
		}
	}
}

void rm380z_state::update_screen_vdu80(bitmap_ind16 &bitmap)
{
	const int ncols = (m_videomode == RM380Z_VIDEOMODE_40COL) ? 40 : 80;

	// blank screen
	bitmap.fill(0);

	if (display_mode == HRG_HIGH)
	{
		draw_high_res_graphics(bitmap);
	}
	else if ((display_mode == HRG_MEDIUM_0) || (display_mode == HRG_MEDIUM_1))
	{
		draw_medium_res_graphics(bitmap);
	}

	for (int row = 0; row < RM380Z_SCREENROWS; row++)
	{
		for (int col = 0; col < ncols; col++)
		{
			uint8_t curch,attribs;
			decode_videoram_char(row, col, curch, attribs);
			putChar_vdu80(curch, attribs, col, row, bitmap);
		}
	}
}

void rm380z_state::update_screen_vdu40(bitmap_ind16 &bitmap)
{
	const int ncols = 40;

	// blank screen
	bitmap.fill(0);

	for (int row = 0; row < RM380Z_SCREENROWS; row++)
	{
		for (int col = 0; col < ncols; col++)
		{
			uint8_t curch = m_vram.get_char(row, col);
			if (curch == 0)
			{
				// NUL character looked like 'O' when displayed and could be used instead of 'O'
				// In fact the front panel writes 0x49, 0x00 to display "IO", and in COS 3.4
				// displaying or typing 'O' actually writes 0x00 to vram.
				// This hack is only necessary because we don't have the real 74LS262 charset ROM
				curch = 'O';
			}
			putChar_vdu40(curch, col, row, bitmap);
		}
	}
}

// This needs the attributes etc from above to be added
uint32_t rm380z_state::screen_update_rm480z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy = 0, ma = 0;

	for (uint8_t y = 0; y < RM380Z_SCREENROWS; y++)
	{
		for (uint8_t ra = 0; ra < 11; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 64; x++)
			{
				uint8_t gfx = 0;
				if (ra < 10)
				{
					const uint8_t chr = m_vram.get_char(y, x);
					gfx = m_chargen[(chr << 4) | ra ];
				}
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
		ma += 64;
	}
	return 0;
}
