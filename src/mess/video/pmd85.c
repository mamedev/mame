// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/***************************************************************************

  pmd85.c

  Functions to emulate the video hardware of PMD-85.

  Krzysztof Strzecha

***************************************************************************/

#include "emu.h"
#include "includes/pmd85.h"

const unsigned char pmd85_palette[3*3] =
{
	0x00, 0x00, 0x00,
	0x7f, 0x7f, 0x7f,
	0xff, 0xff, 0xff
};

PALETTE_INIT_MEMBER(pmd85_state, pmd85)
{
	int i;

	for ( i = 0; i < sizeof(pmd85_palette) / 3; i++ ) {
		m_palette->set_pen_color(i, pmd85_palette[i*3], pmd85_palette[i*3+1], pmd85_palette[i*3+2]);
	}
}

void pmd85_state::video_start()
{
}

void pmd85_state::pmd85_draw_scanline(bitmap_ind16 &bitmap, int pmd85_scanline)
{
	int x, i;
	int pen0, pen1;
	UINT8 data;

	/* set up scanline */
	UINT16 *scanline = &bitmap.pix16(pmd85_scanline);

	/* address of current line in PMD-85 video memory */
	UINT8* pmd85_video_ram_line = m_ram->pointer() + 0xc000 + 0x40*pmd85_scanline;

	for (x=0; x<288; x+=6)
	{
		data = pmd85_video_ram_line[x/6];
		pen0 = 0;
		pen1 = data & 0x80 ? 1 : 2;

		for (i=0; i<6; i++)
			scanline[x+i] = (data & (0x01<<i)) ? pen1 : pen0;

	}
}

UINT32 pmd85_state::screen_update_pmd85(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int pmd85_scanline;

	for (pmd85_scanline=0; pmd85_scanline<256; pmd85_scanline++)
	{
		pmd85_draw_scanline(bitmap, pmd85_scanline);
	}
	return 0;
}
