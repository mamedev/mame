// license:BSD-3-Clause
// copyright-holders:Robin Sergeant

/*

RM 480Z video code

*/

#include "emu.h"
#include "rm480z.h"

void rm480z_state::config_videomode(bool b80col)
{
	int old_mode = m_videomode;

	if (b80col)
	{
		// 80 cols
		m_videomode = RM480Z_VIDEOMODE_80COL;
	}
	else
	{
		// 40 cols
		m_videomode = RM480Z_VIDEOMODE_40COL;
	}

	if (m_videomode != old_mode)
	{
		if (m_videomode == RM480Z_VIDEOMODE_80COL)
		{
			m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 240);
		}
		else
		{
			m_screen->set_raw(8_MHz_XTAL, 512, 0, 320, 312, 0, 240);
		}
	}
}

uint8_t rm480z_state::videoram_read(offs_t offset)
{
	int row = offset & 0xff;
	int col = offset >> 8;

	return m_vram.get_char(row, col);
}

void rm480z_state::videoram_write(offs_t offset, uint8_t data)
{
	int row = offset & 0xff;
	int col = offset >> 8;

	m_vram.set_char(row, col, data);
}

void rm480z_state::putChar(int charnum, int x, int y, bitmap_ind16 &bitmap) const
{
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

void rm480z_state::update_screen(bitmap_ind16 &bitmap) const
{
	const int ncols = (m_videomode == RM480Z_VIDEOMODE_40COL) ? 40 : 80;

	for (int row = 0; row < RM480Z_SCREENROWS; row++)
	{
		for (int col = 0; col < ncols; col++)
		{
			uint8_t curch = m_vram.get_char(row, col);
			putChar(curch, col, row, bitmap);
		}
	}
}
