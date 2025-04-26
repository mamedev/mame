// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   pc1403.cpp
 *   Portable Sharp PC1403 video emulator interface
 *   (Sharp pocket computers)
 *
 *   Copyright (c) 2001 Peter Trauner
 *
 * Change Log:
 * 21.07.2001 Several changes listed below were made by Mario Konegger
 *            (konegger@itp.tu-graz.ac.at)
 *        Placed the graphical symbols in the right place and added
 *        some symbols, so the display is correct.
 *        Added an edge case of the display regarding the on/off
 *        state and the BUSY symbol, which was found out with testing
 *        on an actual PC1403.
 *
 * To Do: Convert to SVG rendering or an internal layout
 *
 *****************************************************************************/

#include "emu.h"

#include "pocketc.h"
#include "pc1403.h"


void pc1403_state::video_start()
{
	m_down = 67;
	m_right = 152;
}

void pc1403h_state::video_start()
{
	m_down = 69;
	m_right = 155;
}

uint8_t pc1403_state::lcd_read(offs_t offset)
{
	return m_reg[offset];
}

void pc1403_state::lcd_write(offs_t offset, uint8_t data)
{
	m_reg[offset] = data;
}

const char* const pc1403_state::s_line[5] = /* simple line */
{
	"     ",
	"     ",
	"11111",
	"11111",
	"11111"
};

const char* const pc1403_state::s_busy[5] =
{
	"11  1 1  11 1 1",
	"1 1 1 1 1   1 1",
	"11  1 1  1  1 1",
	"1 1 1 1   1  1 ",
	"11   1  11   1 "
};
const char* const pc1403_state::s_def[5] =
{
	"11  111 111",
	"1 1 1   1  ",
	"1 1 111 11 ",
	"1 1 1   1  ",
	"11  111 1  "
};
const char* const pc1403_state::s_shift[5] =
{
	" 11 1 1 1 111 111",
	"1   1 1 1 1    1 ",
	" 1  111 1 11   1 ",
	"  1 1 1 1 1    1 ",
	"11  1 1 1 1    1 "
};
const char* const pc1403_state::s_hyp[5] =
{
	"1 1 1 1 11 ",
	"1 1 1 1 1 1",
	"111 1 1 11 ",
	"1 1  1  1  ",
	"1 1  1  1  "
};
const char* const pc1403_state::s_de[5] =
{
	"11  111",
	"1 1 1  ",
	"1 1 111",
	"1 1 1  ",
	"11  111"
};
const char* const pc1403_state::s_g[5] =
{
	" 11",
	"1  ",
	"1 1",
	"1 1",
	" 11"
};
const char* const pc1403_state::s_rad[5] =
{
	"11   1  11 ",
	"1 1 1 1 1 1",
	"11  111 1 1",
	"1 1 1 1 1 1",
	"1 1 1 1 11 "
};
const char* const pc1403_state::s_braces[5] =
{
	" 1 1 ",
	"1   1",
	"1   1",
	"1   1",
	" 1 1 "
};
const char* const pc1403_state::s_m[5] =
{
	"1   1",
	"11 11",
	"1 1 1",
	"1   1",
	"1   1"
};
const char* const pc1403_state::s_e[5] =
{
	"111",
	"1  ",
	"111",
	"1  ",
	"111"
};
const char* const pc1403_state::s_kana[5] = // katakana charset
{
	"  1     1 ",
	" 11111 111",
	"  1  1  1 ",
	" 1   1  1 ",
	"1   1  1  "
};
const char* const pc1403_state::s_shoo[5] = // minor
{
	"    1    ",
	" 1  1  1 ",
	"1   1   1",
	"    1    ",
	"   1     "
};
const char* const pc1403_state::s_sml[5] =
{
	" 11 1 1 1  ",
	"1   111 1  ",
	" 1  1 1 1  ",
	"  1 1 1 1  ",
	"11  1 1 111"
};

uint32_t pc1403_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int contrast = m_dsw0->read() & 7;
	int color[3] =
	{
		7,
		BIT(m_portc, 0) ? 8 : 7,
		8
	};

	bitmap.fill(11, cliprect);

	if (BIT(m_portc, 0))
	{
		int x = m_right;
		int y = m_down;
		for (int i = 0; i < 6*5; x+=2)
			for (int j = 0; j < 5; j++, i++, x+=2)
				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, m_reg[i], contrast, 0, 0, x, y);

		for (int i = 9*5; i < 12*5; x+=2)
			for (int j = 0; j < 5; j++, i++, x+=2)
				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, m_reg[i], contrast, 0, 0, x, y);

		for (int i = 6*5; i < 9*5; x += 2)
			for (int j = 0; j < 5; j++, i++, x+=2)
				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, m_reg[i], contrast, 0, 0, x, y);

		for (int i = 0x7b - 3*5; i > 0x7b - 6*5; x+=2)
			for (int j = 0; j < 5; j++, i--, x+=2)
				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, m_reg[i], contrast, 0, 0, x, y);

		for (int i = 0x7b; i > 0x7b - 3*5; x += 2)
			for (int j = 0; j < 5; j++, i--, x+=2)
				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, m_reg[i], contrast, 0, 0, x, y);

		for (int i = 0x7b - 6*5; i > 0x7b - 12*5; x += 2)
			for (int j = 0; j < 5; j++, i--, x+=2)
				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, m_reg[i], contrast, 0, 0, x, y);
	}

	/* If the display is off, busy is always visible. But if the computer is off, busy is hidden. */
	if(!BIT(m_portc, 3))
	{
		if (BIT(m_portc, 0))
			pocketc_draw_special(bitmap, m_right, m_down-13, s_busy, BIT(m_reg[0x3d], 0) ? color[2] : color[0]);
		else
			pocketc_draw_special(bitmap, m_right, m_down-13, s_busy, color[2]);
	}
	else
	{
		pocketc_draw_special(bitmap, m_right, m_down-13, s_busy, color[0]);
	}

	pocketc_draw_special(bitmap, m_right+155, m_down-13, s_kana, BIT(m_reg[0x3c], 0) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+167, m_down-13, s_shoo, BIT(m_reg[0x3c], 1) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+178, m_down-13, s_sml,  BIT(m_reg[0x3c], 2) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+253, m_down+25, s_line, BIT(m_reg[0x3c], 3) ? color[1] : color[0]); // stat
	pocketc_draw_special(bitmap, m_right+232, m_down+25, s_line, BIT(m_reg[0x3c], 4) ? color[1] : color[0]); // matrix
	pocketc_draw_special(bitmap, m_right+94,  m_down+25, s_line, BIT(m_reg[0x3c], 5) ? color[1] : color[0]); // empty
	pocketc_draw_special(bitmap, m_right+10,  m_down+25, s_line, BIT(m_reg[0x3c], 6) ? color[1] : color[0]); // empty

	pocketc_draw_special(bitmap, m_right+18, m_down-13, s_def,   BIT(m_reg[0x3d], 1) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+43, m_down-13, s_shift, BIT(m_reg[0x3d], 2) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+63, m_down-13, s_hyp,   BIT(m_reg[0x3d], 3) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+73, m_down+25, s_line,  BIT(m_reg[0x3d], 4) ? color[1] : color[0]); // prog
	pocketc_draw_special(bitmap, m_right+52, m_down+25, s_line,  BIT(m_reg[0x3d], 5) ? color[1] : color[0]); // run
	pocketc_draw_special(bitmap, m_right+31, m_down+25, s_line,  BIT(m_reg[0x3d], 6) ? color[1] : color[0]); // calc

	pocketc_draw_special(bitmap, m_right+281, m_down-13, s_e,      BIT(m_reg[0x7c], 0) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+274, m_down-13, s_m,      BIT(m_reg[0x7c], 1) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+266, m_down-13, s_braces, BIT(m_reg[0x7c], 2) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+203, m_down-13, s_rad,    BIT(m_reg[0x7c], 3) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+199, m_down-13, s_g,      BIT(m_reg[0x7c], 4) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+191, m_down-13, s_de,     BIT(m_reg[0x7c], 5) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_right+274, m_down+25, s_line,   BIT(m_reg[0x7c], 6) ? color[1] : color[0]); // print

	return 0;
}
