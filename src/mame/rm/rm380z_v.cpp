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

	if (m_old_videomode != m_videomode)
	{
		m_old_videomode = m_videomode;
	}
}

// char attribute bits in COS 4.0

// 0=alternate charset
// 1=underline
// 2=dim
// 3=reverse


void rm380z_state::decode_videoram_char(int row, int col, uint8_t& chr, uint8_t &attrib)
{
	uint8_t ch1 = m_vramchars[row][col];
	uint8_t ch2 = m_vramattribs[row][col];

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
	else if ((ch1 == 0) && (ch2 == 0))
	{
		// delete char (?)
		chr = 0x20;
		attrib = 0;
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

void rm380z_state::scroll_videoram()
{
	// scroll up one row of videoram
	std::memmove(m_vramchars, m_vramchars[1], (RM380Z_SCREENROWS - 1) * RM380Z_SCREENCOLS);
	std::memmove(m_vramattribs, m_vramattribs[1], (RM380Z_SCREENROWS - 1) * RM380Z_SCREENCOLS);

	// the last line is filled with spaces
	std::memset(m_vramchars[RM380Z_SCREENROWS - 1], 0x20, RM380Z_SCREENCOLS);
	std::memset(m_vramattribs[RM380Z_SCREENROWS - 1], 0x00, RM380Z_SCREENCOLS);
}

void rm380z_state::check_scroll_register()
{
	const uint8_t r[3] = { m_old_old_fbfd, m_old_fbfd, m_fbfd };

	if (!(r[1] & 0x20) && !(r[2] & 0x20))
	{
		// it's a scroll command

		if (r[2] > r[1])
		{
			scroll_videoram();
		}
		else if (!r[2] && (r[1] == 0x17))
		{
			// wrap-scroll
			scroll_videoram();
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

void rm380z_state::videoram_write(offs_t offset, uint8_t data)
{
	int row, col;
	if (get_rowcol_from_offset(row, col, offset))
	{
		// we suppose videoram is being written as character/attribute couple
		// fbfc 6th bit set=attribute, unset=char
		if (m_port0 & 0x40)
		{
			m_vramattribs[row][col] = data;
		}
		else
		{
			m_vramchars[row][col] = data;
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
			return m_vramattribs[row][col];
		}
		else
		{
			return m_vramchars[row][col];
		}
	}

	return 0; // return 0 if out of bounds (see VTIN description in firmware guide)
}

void rm380z_state::putChar(int charnum, int attribs, int x, int y, bitmap_ind16 &bitmap, int vmode)
{
	const bool attrUnder = attribs & 0x02;
	//const bool attrDim = attribs & 0x04;
	const bool attrRev = attribs & 0x08;

	if ((charnum > 0) && (charnum <= 0x7f))
	{
		// normal chars (base set)

		if (vmode==RM380Z_VIDEOMODE_80COL)
		{
			int basex=RM380Z_CHDIMX*(charnum/RM380Z_NCY);
			int basey=RM380Z_CHDIMY*(charnum%RM380Z_NCY);

			for (int r=0;r<RM380Z_CHDIMY;r++)
			{
				for (int c=0;c<RM380Z_CHDIMX;c++)
				{
					uint8_t chval = (m_chargen[((basey + r) * RM380Z_CHDIMX * RM380Z_NCX) + (basex + c)] == 0xff) ? 0 : 1;

					if (attrRev)
					{
						if (chval==0) chval=1;
						else chval=0;
					}

					if (attrUnder)
					{
						if (r==(RM380Z_CHDIMY-1))
						{
							if (attrRev) chval=0;
							else chval=1;
						}
					}

					bitmap.pix((y*(RM380Z_CHDIMY+1))+r,(x*(RM380Z_CHDIMX+1))+c) = chval;
				}
			}

			// last pixel of underline
			if (attrUnder&&(!attrRev))
			{
				bitmap.pix((y*(RM380Z_CHDIMY+1))+(RM380Z_CHDIMY-1),(x*(RM380Z_CHDIMX+1))+RM380Z_CHDIMX) = attrRev?0:1;
			}

			// if reversed, print another column of pixels on the right
			if (attrRev)
			{
				for (int r=0;r<RM380Z_CHDIMY;r++)
				{
					bitmap.pix((y*(RM380Z_CHDIMY+1))+r,(x*(RM380Z_CHDIMX+1))+RM380Z_CHDIMX) = 1;
				}
			}
		}
		else if (vmode==RM380Z_VIDEOMODE_40COL)
		{
			int basex=RM380Z_CHDIMX*(charnum/RM380Z_NCY);
			int basey=RM380Z_CHDIMY*(charnum%RM380Z_NCY);

			for (int r=0;r<RM380Z_CHDIMY;r++)
			{
				for (int c=0;c<(RM380Z_CHDIMX*2);c+=2)
				{
					uint8_t chval = (m_chargen[((basey + r) * RM380Z_CHDIMX * RM380Z_NCX) + (basex + (c / 2))] == 0xff) ? 0 : 1;

					if (attrRev)
					{
						if (chval==0) chval=1;
						else chval=0;
					}

					if (attrUnder)
					{
						if (r==(RM380Z_CHDIMY-1))
						{
							if (attrRev) chval=0;
							else chval=1;
						}
					}

					bitmap.pix( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+c) = chval;
					bitmap.pix( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+c+1) = chval;
				}
			}

			// last 2 pixels of underline
			if (attrUnder)
			{
				bitmap.pix( (y*(RM380Z_CHDIMY+1))+RM380Z_CHDIMY-1 , ((x*(RM380Z_CHDIMX+1))*2)+(RM380Z_CHDIMX*2)) = attrRev?0:1;
				bitmap.pix( (y*(RM380Z_CHDIMY+1))+RM380Z_CHDIMY-1 , ((x*(RM380Z_CHDIMX+1))*2)+(RM380Z_CHDIMX*2)+1) = attrRev?0:1;
			}

			// if reversed, print another 2 columns of pixels on the right
			if (attrRev)
			{
				for (int r=0;r<RM380Z_CHDIMY;r++)
				{
					bitmap.pix( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+((RM380Z_CHDIMX)*2)) = 1;
					bitmap.pix( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+((RM380Z_CHDIMX)*2)+1) = 1;
				}
			}
		}
	}
	else
	{
		// graphic chars: 0x80-0xbf is "dimmed", 0xc0-0xff is full bright
		if (vmode==RM380Z_VIDEOMODE_80COL)
		{
			for (int r=0;r<(RM380Z_CHDIMY+1);r++)
			{
				for (int c=0;c<RM380Z_CHDIMX;c++)
				{
					bitmap.pix((y*(RM380Z_CHDIMY+1))+r,(x*(RM380Z_CHDIMX+1))+c) = m_graphic_chars[charnum&0x3f][c+(r*(RM380Z_CHDIMX+1))];
				}
			}
		}
		else
		{
			for (int r=0;r<RM380Z_CHDIMY;r++)
			{
				for (int c=0;c<(RM380Z_CHDIMX*2);c+=2)
				{
					bitmap.pix( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+c) = m_graphic_chars[charnum&0x3f][(c/2)+(r*(RM380Z_CHDIMX+1))];
					bitmap.pix( (y*(RM380Z_CHDIMY+1))+r,((x*(RM380Z_CHDIMX+1))*2)+c+1) = m_graphic_chars[charnum&0x3f][(c/2)+(r*(RM380Z_CHDIMX+1))];
				}
			}
		}
	}
}

void rm380z_state::update_screen(bitmap_ind16 &bitmap)
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
			putChar(curch, attribs, col, row, bitmap, m_videomode);
			//putChar(0x44, 0x00, 10, 10, bitmap, m_videomode);
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
					const uint8_t chr = m_vramchars[y][x];
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
