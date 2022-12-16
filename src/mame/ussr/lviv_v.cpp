// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/***************************************************************************

  lviv.c

  Functions to emulate the video hardware of PK-01 Lviv.

  Krzysztof Strzecha

***************************************************************************/

#include "emu.h"
#include "lviv.h"

const rgb_t lviv_state::s_palette[8] =
{
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xa4 },
	{ 0x00, 0xa4, 0x00 },
	{ 0x00, 0xa4, 0xa4 },
	{ 0xa4, 0x00, 0x00 },
	{ 0xa4, 0x00, 0xa4 },
	{ 0xa4, 0xa4, 0x00 },
	{ 0xa4, 0xa4, 0xa4 }
};


void lviv_state::lviv_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, s_palette);
}


void lviv_state::update_palette(uint8_t pal)
{
	m_colortable[0][0] = 0;
	m_colortable[0][1] = 0;
	m_colortable[0][2] = 0;
	m_colortable[0][3] = 0;

	m_colortable[0][0] |= (BIT(pal, 3) == BIT(pal, 4)) ? 0x04 : 0x00;
	m_colortable[0][0] |= BIT(pal, 5) ? 0x02 : 0x00;
	m_colortable[0][0] |= (BIT(pal, 2) == BIT(pal, 6)) ? 0x01 : 0x00;

	m_colortable[0][1] |= (BIT(pal, 0) == BIT(pal, 4)) ? 0x04 : 0x00;
	m_colortable[0][1] |= BIT(pal, 5) ? 0x02 : 0x00;
	m_colortable[0][1] |= BIT(pal, 6) ? 0x00 : 0x01;

	m_colortable[0][2] |= BIT(pal, 4) ? 0x04 : 0x00;
	m_colortable[0][2] |= BIT(pal, 5) ? 0x00 : 0x02;
	m_colortable[0][2] |= BIT(pal, 6) ? 0x01 : 0x00;

	m_colortable[0][3] |= BIT(pal, 4) ? 0x00 : 0x04;
	m_colortable[0][3] |= (BIT(pal, 1) == BIT(pal, 5)) ? 0x02 : 0x00;
	m_colortable[0][3] |= BIT(pal, 6) ? 0x01 : 0x00;
}

uint32_t lviv_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x += 4)
		{
			const uint8_t data = m_vram[(y << 6) | (x >> 2)];
			int pen;

			pen = m_colortable[0][((data & 0x08) >> 3) | ((data & 0x80) >> (3+3))];
			bitmap.pix(y, x + 0) = pen;

			pen = m_colortable[0][((data & 0x04) >> 2) | ((data & 0x40) >> (2+3))];
			bitmap.pix(y, x + 1) = pen;

			pen = m_colortable[0][((data & 0x02) >> 1) | ((data & 0x20) >> (1+3))];
			bitmap.pix(y, x + 2) = pen;

			pen = m_colortable[0][((data & 0x01) >> 0) | ((data & 0x10) >> (0+3))];
			bitmap.pix(y, x + 3) = pen;
		}
	}
	return 0;
}
