// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona

/*

RM 380Z video code

*/

#include "emu.h"
#include "rm380z.h"

bool rm380z_state::get_rowcol_from_offset(int& row, int& col, offs_t offset) const
{
	if (m_videomode == RM380Z_VIDEOMODE_40COL)
	{
		col = offset & 0x3f;            // the 6 least significant bits give the column (0-39)
		row = offset >> 6;              // next 5 bits give the row (0-23)
	}
	else
	{
		col = offset & 0x7f;            // the 7 least significant bits give the column (0-79)
		row = offset >> 7;              // next bit gives bit 0 of row
		row |= (m_fbfe & 0x0f) << 1;    // the remaining 4 row bits come from the lower half of PORT 1
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

uint8_t rm380z_state::videoram_read(offs_t offset)
{
	int row, col;
	if (get_rowcol_from_offset(row, col, offset))
	{
		if (m_port0 & 0x40)
		{
			return m_vram.get_attrib(row, col);
		}
		else
		{
			return m_vram.get_char(row, col);
		}
	}

	return 0; // return 0 if out of bounds (see VTIN description in firmware guide)
}

void rm380z_state::putChar_vdu80(int charnum, int attribs, int x, int y, bitmap_ind16 &bitmap)
{
	const bool attrUnder = attribs & 0x02;
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
			uint8_t pixel_value = (data & 0x80) ? 1 : 0;
			if (attrRev)
			{
				pixel_value = !pixel_value;
			}
			bitmap.pix(y * 10 + r, x * 8 + c) = pixel_value;
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
				uint8_t chval = (m_chargen[((basey + r) * RM380Z_CHDIMX * RM380Z_NCX) + basex + c] == 0xff) ? 0 : 1;
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
				bitmap.pix(y * (RM380Z_CHDIMY+1) + r, x * (RM380Z_CHDIMX+1) +c) = m_graphic_chars[charnum&0x3f][c + r * (RM380Z_CHDIMX+1)];
			}
		}
	}
}

void rm380z_state::update_screen_vdu80(bitmap_ind16 &bitmap)
{
	const int ncols = (m_videomode == RM380Z_VIDEOMODE_40COL) ? 40 : 80;

	// blank screen
	bitmap.fill(0);

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
