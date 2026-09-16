// license:BSD-3-Clause
// copyright-holders:Robin Sergeant

/*

RM 480Z video code

*/

#include "emu.h"
#include "rm480z.h"

INPUT_CHANGED_MEMBER(rm480z_state::monitor_changed)
{
	// re-calculate HRG palette values from scratchpad
	for (int c=0; c < RM480Z_HRG_SCRATCHPAD_SIZE; c++)
	{
		change_palette(c, m_hrg_scratchpad[c]);
	}
}

void rm480z_state::change_palette(int index, uint8_t value)
{
	rgb_t new_colour;

	if (m_io_display_type->read())
	{
		// value is intensity for a b/w monochrome display
		new_colour = rgb_t(value, value, value);
	}
	else
	{
		// for colour displays only bits 4, 6 and 7 are used.
		uint8_t red = BIT(value, 6) << 2;
		uint8_t green = BIT(value, 7) << 1;
		uint8_t blue = BIT(value, 4);
		new_colour = raw_to_rgb_converter::standard_rgb_decoder<1, 1, 1, 2, 1, 0>(red | green | blue);
	}

	m_palette->set_pen_color(index + 3, new_colour);
	m_hrg_scratchpad[index] = value;
}

void rm480z_state::palette_init(palette_device &palette)
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

int rm480z_state::calculate_hrg_vram_index() const
{
	int index = -1;

	if (m_hrg_port0 < 192)
	{
		// 15K of pixel data, 80 bytes per row
		if (m_hrg_port1 < 80)
		{
			index = (m_hrg_port0 * 80) + m_hrg_port1;
		}
	}
	else
	{
		// 1K of user defined character data, 8 bytes per character grouped into 16 byte blocks
		if ((m_hrg_port1 >= 64) && (m_hrg_port1 <= 79))
		{
			index = 15'360 + ((m_hrg_port0 - 192) * 16) + (m_hrg_port1 - 64);
		}
	}

	return index;
}

uint8_t rm480z_state::videoram_read(offs_t offset)
{
	uint8_t data = 0;
	const int row = offset & 0xff;
	const int col = offset >> 8;

	if (col < RM480Z_SCREENCOLS)
	{
		data = m_vram.get_char(row, col);
	}

	return data;
}

void rm480z_state::videoram_write(offs_t offset, uint8_t data)
{
	const int row = offset & 0xff;
	const int col = offset >> 8;

	if (col < RM480Z_SCREENCOLS)
	{
		m_vram.set_char(row, col, data);
	}
}

void rm480z_state::putChar(int charnum, int x, int y, bitmap_ind16 &bitmap, bool bMonochrome) const
{
	const int pw = (m_videomode == RM480Z_VIDEOMODE_40COL) ? 2 : 1;
	const int ph = 1;
	bool attrDim = false;
	bool attrRev = false;

	if (charnum > 128)
	{
		if (m_alt_char_set)
		{
			charnum -= 128;
			attrDim = true;
			attrRev = true;
		}
		else if (charnum < 0xc0)
		{
			attrDim = true;
		}
	}

	int data_pos = charnum * 16;

	for (int r = 0; r < 10; r++, data_pos++)
	{
		uint8_t data = m_chargen[data_pos];

		for (int c = 0; c < 8; c++, data <<= 1)
		{
			uint8_t pixel_value;
			if (attrRev)
			{
				pixel_value = BIT(data, 7) ? 0 : 2;
			}
			else
			{
				pixel_value = BIT(data, 7) ? 2 : 0;
			}
			if (attrDim && pixel_value && bMonochrome)
			{
				// NB only monochrome monitors support dimmed (grey) text
				pixel_value = 1;
			}
			if (pixel_value)
			{
				bitmap.plot_box((x * 8 + c) * pw, y * 10 + r, pw, ph, pixel_value);
			}
		}
	}
}

void rm480z_state::draw_extra_high_res_graphics(bitmap_ind16 &bitmap) const
{
	const int pw = 1;
	const int ph = 1;

	// (1-bit per pixel, 8 pixels per byte)
	for (int y = 0; y < 192; y++)
	{
		for (int x = 0; x < 640; x+= 8)
		{
			int index = (y * 80) + (x / 8);
			uint8_t data = m_hrg_ram[index];
			for (int c = 0; c < 8; c++, data <<= 1)
			{
				uint8_t pixel_value = data & 0x80 ? 2 : 0;
				if (pixel_value)
				{
					bitmap.plot_box((x + c)*pw, y*ph, pw, ph, pixel_value);
				}
			}
		}
	}
}

void rm480z_state::draw_high_res_graphics(bitmap_ind16 &bitmap) const
{
	const int pw = 2;
	const int ph = 1;

	// (2-bits per pixel, 4 pixels per byte)
	for (int y = 0; y < 192; y++)
	{
		for (int x = 0; x < 320; x+= 4)
		{
			int index = (y * 80) + (x / 4);
			uint8_t data = m_hrg_ram[index];
			for (int c = 0; c < 4; c++, data >>= 2)
			{
				bitmap.plot_box((x + c)*pw, y*ph, pw, ph, (data & 0x03) + 3);
			}
		}
	}
}

void rm480z_state::draw_medium_res_graphics(bitmap_ind16 &bitmap) const
{
	const int page = (m_hrg_display_mode == hrg_display_mode::medium_0) ? 0 : 1;
	const int pw = 4;
	const int ph = 2;

	// (4-bits per pixel, 2 pixels per byte)
	for (int y = 0; y < 96; y++)
	{
		for (int x = 0; x < 160; x+= 2)
		{
			int index = (((y * 2) + page) * 80) + (x / 2);
			uint8_t data = m_hrg_ram[index];
			for (int c = 0; c < 2; c++, data >>= 2)
			{
				uint8_t pixel_value = (((data >> 2) & 0x0c) | (data & 0x03)) +3;
				bitmap.plot_box((x + c)*pw, y*ph, pw, ph, pixel_value);
			}
		}
	}
}

void rm480z_state::update_screen(bitmap_ind16 &bitmap) const
{
	const int ncols = (m_videomode == RM480Z_VIDEOMODE_40COL) ? 40 : 80;
	const bool bMonochrome = m_io_display_type->read();

	if (!m_hrg_mem_open && (!m_hrg_inhibit || !bMonochrome))
	{
		switch (m_hrg_display_mode)
		{
		case hrg_display_mode::extra_high:
			draw_extra_high_res_graphics(bitmap);
			break;
		case hrg_display_mode::high:
			draw_high_res_graphics(bitmap);
			break;
		case hrg_display_mode::medium_0:
		case hrg_display_mode::medium_1:
			draw_medium_res_graphics(bitmap);
			break;
		case hrg_display_mode::none:
			// don't display HRG
			break;
		}
	}

	if (!m_video_inhibit || bMonochrome)
	{
		for (int row = 0; row < RM480Z_SCREENROWS; row++)
		{
			for (int col = 0; col < ncols; col++)
			{
				uint8_t curch = m_vram.get_char(row, col);
				putChar(curch, col, row, bitmap, bMonochrome);
			}
		}
	}
}
