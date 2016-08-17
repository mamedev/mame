// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/***************************************************************************

  lviv.c

  Functions to emulate the video hardware of PK-01 Lviv.

  Krzysztof Strzecha

***************************************************************************/

#include "emu.h"
#include "includes/lviv.h"

const unsigned char lviv_palette[8*3] =
{
	0x00, 0x00, 0x00,
	0x00, 0x00, 0xa4,
	0x00, 0xa4, 0x00,
	0x00, 0xa4, 0xa4,
	0xa4, 0x00, 0x00,
	0xa4, 0x00, 0xa4,
	0xa4, 0xa4, 0x00,
	0xa4, 0xa4, 0xa4
};


PALETTE_INIT_MEMBER(lviv_state, lviv)
{
	int i;

	for ( i = 0; i < sizeof(lviv_palette) / 3; i++ ) {
		m_palette->set_pen_color(i, lviv_palette[i*3], lviv_palette[i*3+1], lviv_palette[i*3+2]);
	}
}


void lviv_state::lviv_update_palette(UINT8 pal)
{
	m_colortable[0][0] = 0;
	m_colortable[0][1] = 0;
	m_colortable[0][2] = 0;
	m_colortable[0][3] = 0;

	m_colortable[0][0] |= (((pal>>3)&0x01) == ((pal>>4)&0x01)) ? 0x04 : 0x00;
	m_colortable[0][0] |= ((pal>>5)&0x01) ? 0x02 : 0x00;
	m_colortable[0][0] |= (((pal>>2)&0x01) == ((pal>>6)&0x01)) ? 0x01 : 0x00;

	m_colortable[0][1] |= ((pal&0x01) == ((pal>>4)&0x01)) ? 0x04 : 0x00;
	m_colortable[0][1] |= ((pal>>5)&0x01) ? 0x02 : 0x00;
	m_colortable[0][1] |= ((pal>>6)&0x01) ? 0x00 : 0x01;

	m_colortable[0][2] |= ((pal>>4)&0x01) ? 0x04 : 0x00;
	m_colortable[0][2] |= ((pal>>5)&0x01) ? 0x00 : 0x02;
	m_colortable[0][2] |= ((pal>>6)&0x01) ? 0x01 : 0x00;

	m_colortable[0][3] |= ((pal>>4)&0x01) ? 0x00 : 0x04;
	m_colortable[0][3] |= (((pal>>1)&0x01) == ((pal>>5)&0x01)) ? 0x02 : 0x00;
	m_colortable[0][3] |= ((pal>>6)&0x01) ? 0x01 : 0x00;
}

void lviv_state::video_start()
{
}

UINT32 lviv_state::screen_update_lviv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int pen;
	UINT8 data;

	for (y=0; y<256; y++)
		for (x=0; x<256; x+=4)
		{
			data = m_video_ram[y*64+x/4];

			pen = m_colortable[0][((data & 0x08) >> 3) | ((data & 0x80) >> (3+3))];
			bitmap.pix16(y, x + 0) = pen;

			pen = m_colortable[0][((data & 0x04) >> 2) | ((data & 0x40) >> (2+3))];
			bitmap.pix16(y, x + 1) = pen;

			pen = m_colortable[0][((data & 0x02) >> 1) | ((data & 0x20) >> (1+3))];
			bitmap.pix16(y, x + 2) = pen;

			pen = m_colortable[0][((data & 0x01) >> 0) | ((data & 0x10) >> (0+3))];
			bitmap.pix16(y, x + 3) = pen;
		}
	return 0;
}
