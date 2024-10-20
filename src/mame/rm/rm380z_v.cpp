// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona

/*

RM 380Z video code

*/

#include "emu.h"
#include "rm380z.h"

INPUT_CHANGED_MEMBER(rm380z_state_cos40_hrg::monitor_changed)
{
	// re-calculate HRG palette values from scratchpad
	for (int c=0; c < RM380Z_HRG_SCRATCHPAD_SIZE; c++)
	{
		change_palette(c, m_hrg_scratchpad[c]);
	}
}

void rm380z_state_cos40_hrg::change_palette(int index, uint8_t value)
{
	rgb_t new_colour;

	if (m_io_display_type->read() & 0x01)
	{
		// value is intensity for a b/w monochrome display
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

void rm380z_state_cos40_hrg::palette_init(palette_device &palette)
{
	// text display palette (black, grey (dim), and white)
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0xc0, 0xc0, 0xc0));
	palette.set_pen_color(2, rgb_t::white());

	// HRG palette (initialise to all black)
	for (int c = 3; c < 19; c++)
	{
		palette.set_pen_color(c, rgb_t::black());
	}
}

void rm380z_state_cos40_hrg::change_hrg_scratchpad(int index, uint8_t value, uint8_t mask)
{
	if (index < RM380Z_HRG_SCRATCHPAD_SIZE)
	{
		m_hrg_scratchpad[index] &= mask;
		m_hrg_scratchpad[index] |= value;

		change_palette(index, m_hrg_scratchpad[index]);
	}
}

int rm380z_state_cos40_hrg::calculate_hrg_vram_index(offs_t offset) const
{
	int index;
	int page = m_hrg_port1 & 0x0f;

	if (page < 12)
	{
		// the first 15k is addressed using twelve 1280 byte pages
		// this is used to store pixel data
		index = (page * 1280) + (offset % 1280);
	}
	else
	{
		// the remaining 1k is addressed using four 256 byte pages
		// this is used to store 128 user defined HRG characters
		page &= 0x03;
		index = 15360 + (page * 256) + (offset & 0xff);
	}

	return index;
}

bool rm380z_state::get_rowcol_from_offset(int& row, int& col, offs_t offset) const
{
	col = offset & 0x3f;  // the 6 least significant bits give the column (0-39)
	row = offset >> 6;    // next 5 bits give the row (0-23)

	return ((row < RM380Z_SCREENROWS) && (col < RM380Z_SCREENCOLS));
}

bool rm380z_state_cos40::get_rowcol_from_offset(int& row, int& col, offs_t offset) const
{
	if (m_videomode == RM380Z_VIDEOMODE_80COL)
	{
		col = offset & 0x7f;          // the 7 least significant bits give the column (0-79)
		row = offset >> 7;            // next bit gives bit 0 of row
		row |= (m_fbfe & 0x0f) << 1;  // the remaining 4 row bits come from the lower half of PORT 1
	}
	else
	{
		(void)rm380z_state::get_rowcol_from_offset(row, col, offset);
	}

	return ((row < RM380Z_SCREENROWS) && (col < RM380Z_SCREENCOLS));
}

void rm380z_state_cos40::config_videomode()
{
	int old_mode = m_videomode;

	if (BIT(m_port0, 5))
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
			m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 240);
		}
		else
		{
			m_screen->set_raw(8_MHz_XTAL, 512, 0, 320, 312, 0, 240);
		}
	}
}

// after ctrl-L (clear screen?): routine at EBBD is executed
// EB30??? next line?
// memory at FF02 seems to hold the line counter (same as FBFD)
//
// basics:
// 20e2: prints "Ready:"
// 0195: prints "\n"

void rm380z_state_cos34::videoram_write(offs_t offset, uint8_t data)
{
	int row, col;
	if (get_rowcol_from_offset(row, col, offset))
	{
		m_vram.set_char(row, col, data);
	}
	// else out of bounds write had no effect (see VTOUT description in firmware guide)
}

void rm380z_state_cos40::videoram_write(offs_t offset, uint8_t data)
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
			// when a character is written, the corresponding attributes are cleared
			m_vram.set_attrib(row, col, 0);
		}
	}
	// else out of bounds write had no effect (see VTOUT description in firmware guide)
}

void rm380z_state_cos40_hrg::videoram_write(offs_t offset, uint8_t data)
{
	if (BIT(m_hrg_port0, 2))
	{
		// write to HRG memory
		m_hrg_ram[calculate_hrg_vram_index(offset)] = data;
	}
	else
	{
		rm380z_state_cos40::videoram_write(offset, data);
	}
}

uint8_t rm380z_state_cos34::videoram_read(offs_t offset)
{
	uint8_t data = 0; // return 0 if out of bounds (see VTIN description in firmware guide)

	int row, col;
	if (get_rowcol_from_offset(row, col, offset))
	{
		data = m_vram.get_char(row, col);
	}

	return data;
}

uint8_t rm380z_state_cos40::videoram_read(offs_t offset)
{
	uint8_t data = 0; // return 0 if out of bounds (see VTIN description in firmware guide)

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

	return data;
}

uint8_t rm380z_state_cos40_hrg::videoram_read(offs_t offset)
{
	uint8_t data;

	if (BIT(m_hrg_port0, 2))
	{
		// read from HRG memory
		data = m_hrg_ram[calculate_hrg_vram_index(offset)];
	}
	else
	{
		data = rm380z_state_cos40::videoram_read(offset);
	}

	return data;
}

void rm380z_state_cos40::putChar_vdu80(int charnum, int attribs, int x, int y, bitmap_ind16 &bitmap) const
{
	const bool attrUnder = attribs & 0x02;
	const bool attrDim = attribs & 0x04;
	const bool attrRev = attribs & 0x08;

	int data_pos = (charnum % 128) * 16;

	for (int r = 0; r < 10; r++, data_pos++)
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

		for (int c = 0; c < 8; c++, data <<= 1)
		{
			uint8_t pixel_value = BIT(data, 7) ? 2 : 0;
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

void rm380z_state_cos34::putChar_vdu40(int charnum, int x, int y, bitmap_ind16 &bitmap) const
{
	if (charnum <= 0x7f)
	{
		// 5x9 characters are drawn in 8x10 grid
		// with 1 pixel gap to the left, 2 pixel gap to the right, and 1 pixel gap at the bottom
		for (int r = 0; r < 9; r++)
		{
			uint8_t data = m_rocg->read(charnum, r);

			for (int c = 1; c < 6; c++, data <<= 1)
			{
				if (BIT(data, 6))
				{
					bitmap.pix(y * 10 + r, x * 8 + c) = 2;
				}
			}
		}
	}
	else
	{
		// graphic chars (chars 0x80 to 0xbf are grey, chars 0xc0 to 0xff are white)
		uint8_t colour = (charnum >= 0xc0) ? 2 : 1;

		// discrete logic gates were used to produce a full 8x10 grid of pixels
		// the top block is 4 pixels high, and the two lower two blocks are 3 pixels high
		if (BIT(charnum, 0))
		{
			bitmap.plot_box(x * 8, y * 10, 4, 4, colour);
		}
		if (BIT(charnum, 1))
		{
			bitmap.plot_box(x * 8 + 4, y * 10, 4, 4, colour);
		}
		if (BIT(charnum, 2))
		{
			bitmap.plot_box(x * 8, y * 10 + 4, 4, 3, colour);
		}
		if (BIT(charnum, 3))
		{
			bitmap.plot_box(x * 8 + 4, y * 10 + 4, 4, 3, colour);
		}
		if (BIT(charnum, 4))
		{
			bitmap.plot_box(x * 8, y * 10 + 7, 4, 3, colour);
		}
		if (BIT(charnum, 5))
		{
			bitmap.plot_box(x * 8 + 4, y * 10 + 7, 4, 3, colour);
		}
	}
}

void rm380z_state_cos40_hrg::draw_high_res_graphics(bitmap_ind16 &bitmap) const
{
	const int pw = (m_videomode == RM380Z_VIDEOMODE_40COL) ? 1 : 2;
	const int ph = 1;

	// see section C.3 of HRG reference manual for ram layout
	// (2-bits per pixel, 4 pixels per byte)
	for (int y = 0; y < 192; y++)
	{
		for (int x = 0; x < 320; x+= 4)
		{
			int index = ((y / 16) * 1280) + ((x / 4) << 4) + (y % 16);
			uint8_t data = m_hrg_ram[index];
			for (int c = 0; c < 4; c++, data >>= 2)
			{
				bitmap.plot_box((x + c)*pw, y*ph, pw, ph, (data & 0x03) + 3);
			}
		}
	}
}

void rm380z_state_cos40_hrg::draw_medium_res_graphics(bitmap_ind16 &bitmap) const
{
	const int page = (m_hrg_display_mode == hrg_display_mode::medium_0) ? 0 : 1;
	const int pw = (m_videomode == RM380Z_VIDEOMODE_40COL) ? 2 : 4;
	const int ph = 2;

	// see section C.5 of HRG reference manual for ram layout
	// (4-bits per pixel, 2 pixels per byte)
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

void rm380z_state_cos40_hrg::update_screen(bitmap_ind16 &bitmap) const
{
	if (m_hrg_display_mode == hrg_display_mode::high)
	{
		draw_high_res_graphics(bitmap);
	}
	else if ((m_hrg_display_mode == hrg_display_mode::medium_0) || (m_hrg_display_mode == hrg_display_mode::medium_1))
	{
		draw_medium_res_graphics(bitmap);
	}

	if (!BIT(m_fbfd, 7))
	{
		// display text on top of graphics unless prevented by bit 7 of fbfd (VID INHIB)
		rm380z_state_cos40::update_screen(bitmap);
	}
}

void rm380z_state_cos40::update_screen(bitmap_ind16 &bitmap) const
{
	const int ncols = (m_videomode == RM380Z_VIDEOMODE_40COL) ? 40 : 80;

	for (int row = 0; row < RM380Z_SCREENROWS; row++)
	{
		for (int col = 0; col < ncols; col++)
		{
			uint8_t curch,attribs;
			curch = m_vram.get_char(row, col);
			attribs = m_vram.get_attrib(row, col);
			putChar_vdu80(curch, attribs, col, row, bitmap);
		}
	}
}

void rm380z_state_cos34::update_screen(bitmap_ind16 &bitmap) const
{
	const int ncols = 40;

	for (int row = 0; row < RM380Z_SCREENROWS; row++)
	{
		for (int col = 0; col < ncols; col++)
		{
			uint8_t curch = m_vram.get_char(row, col);
			putChar_vdu40(curch, col, row, bitmap);
		}
	}
}
