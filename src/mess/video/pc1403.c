// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   pc1403.c
 *   portable sharp pc1403 video emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright (c) 2001 Peter Trauner, all rights reserved.
 *
 * History of changes:
 * 21.07.2001 Several changes listed below were made by Mario Konegger
 *            (konegger@itp.tu-graz.ac.at)
 *        Placed the grafical symbols onto the right place and added
 *        some symbols, so the display is correct rebuit.
 *        Added a strange behaviour of the display concerning the on/off
 *        state and the BUSY-symbol, which I found out with experiments
 *        with my own pc1403.
 *****************************************************************************/

#include "emu.h"

#include "includes/pocketc.h"
#include "includes/pc1403.h"


void pc1403_state::video_start()
{
	if (strcmp(machine().system().name, "pc1403h") == 0)
	{
		m_DOWN = 69;
		m_RIGHT = 155;
	}
	else
	{
		m_DOWN = 67;
		m_RIGHT = 152;
	}
}

READ8_MEMBER(pc1403_state::pc1403_lcd_read)
{
	return m_reg[offset];
}

WRITE8_MEMBER(pc1403_state::pc1403_lcd_write)
{
	m_reg[offset]=data;
}

static const POCKETC_FIGURE line={ /* simple line */
	"11111",
	"11111",
	"11111e"
};
static const POCKETC_FIGURE busy={
	"11  1 1  11 1 1",
	"1 1 1 1 1   1 1",
	"11  1 1  1  1 1",
	"1 1 1 1   1  1",
	"11   1  11   1e"
}, def={
	"11  111 111",
	"1 1 1   1",
	"1 1 111 11",
	"1 1 1   1",
	"11  111 1e"
}, shift={
	" 11 1 1 1 111 111",
	"1   1 1 1 1    1",
	" 1  111 1 11   1",
	"  1 1 1 1 1    1",
	"11  1 1 1 1    1e"
}, hyp={
	"1 1 1 1 11",
	"1 1 1 1 1 1",
	"111 1 1 11",
	"1 1  1  1",
	"1 1  1  1e"
}, de={
	"11  111",
	"1 1 1",
	"1 1 111",
	"1 1 1",
	"11  111e"
}, g={
	" 11",
	"1",
	"1 1",
	"1 1",
	" 11e"
}, rad={
	"11   1  11",
	"1 1 1 1 1 1",
	"11  111 1 1",
	"1 1 1 1 1 1",
	"1 1 1 1 11e"
}, braces={
	" 1 1",
	"1   1",
	"1   1",
	"1   1",
	" 1 1e"
}, m={
	"1   1",
	"11 11",
	"1 1 1",
	"1   1",
	"1   1e"
}, e={
	"111",
	"1",
	"111",
	"1",
	"111e"
}, kana={ // katakana charset
	"  1     1 ",
	" 11111 111",
	"  1  1  1 ",
	" 1   1  1 ",
	"1   1  1e"
}, shoo={ // minor
	"    1    ",
	" 1  1  1 ",
	"1   1   1",
	"    1    ",
	"   1e"
}, sml={
	" 11 1 1 1",
	"1   111 1",
	" 1  1 1 1",
	"  1 1 1 1",
	"11  1 1 111e"
};

UINT32 pc1403_state::screen_update_pc1403(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y, i, j;
	int color[3];

	bitmap.fill(11, cliprect);

	/* HJB: we cannot initialize array with values from other arrays, thus... */
	color[0] = 7; // pocketc_colortable[CONTRAST][0];
	color[2] = 8; // pocketc_colortable[CONTRAST][1];
	color[1] = (m_portc & 1) ? color[2] : color[0];

	if (m_portc & 1)
	{
		for (x=m_RIGHT, y=m_DOWN, i=0; i<6*5; x+=2) {
			for (j=0; j<5; j++, i++, x+=2)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_reg[i],CONTRAST,0,0,
				x,y);
		}
		for (i=9*5; i<12*5; x+=2)
		{
			for (j=0; j<5; j++, i++, x+=2)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_reg[i],CONTRAST,0,0,
				x,y);
		}
		for (i=6*5; i<9*5; x+=2)
		{
			for (j=0; j<5; j++, i++, x+=2)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_reg[i],CONTRAST,0,0,
				x,y);
		}
		for (i=0x7b-3*5; i>0x7b-6*5; x+=2)
		{
			for (j=0; j<5; j++, i--, x+=2)
				m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_reg[i],CONTRAST,0,0,
				x,y);
		}
		for (i=0x7b; i>0x7b-3*5; x+=2)
		{
			for (j=0; j<5; j++, i--, x+=2)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_reg[i],CONTRAST,0,0,
				x,y);
		}
		for (i=0x7b-6*5; i>0x7b-12*5; x+=2)
		{
			for (j=0; j<5; j++, i--, x+=2)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_reg[i],CONTRAST,0,0,
				x,y);
		}
	}
	/* if display is off, busy is always visible? it seems to behave like that. */
	/* But if computer is off, busy is hidden. */
	if(!(m_portc&8))
	{
		if (m_portc&1)
			pocketc_draw_special(bitmap, m_RIGHT, m_DOWN-13, busy,
				m_reg[0x3d] & 1 ? color[2] : color[0]);

		else pocketc_draw_special(bitmap, m_RIGHT, m_DOWN-13, busy, color[2]);
	}
	else
		pocketc_draw_special(bitmap, m_RIGHT, m_DOWN-13, busy, color[0]);

	pocketc_draw_special(bitmap, m_RIGHT+18, m_DOWN-13, def,
				m_reg[0x3d] & 0x02 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+43, m_DOWN-13, shift,
				m_reg[0x3d] & 0x04 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+63, m_DOWN-13, hyp,
				m_reg[0x3d] & 0x08 ? color[1] : color[0]);

	pocketc_draw_special(bitmap, m_RIGHT+155, m_DOWN-13, kana,
				m_reg[0x3c] & 0x01 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+167, m_DOWN-13, shoo,
				m_reg[0x3c] & 0x02 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+178, m_DOWN-13, sml,
				m_reg[0x3c] & 0x04 ? color[1] : color[0]);

	pocketc_draw_special(bitmap, m_RIGHT+191, m_DOWN-13, de,
				m_reg[0x7c] & 0x20 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+199, m_DOWN-13, g,
				m_reg[0x7c] & 0x10 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+203, m_DOWN-13, rad,
				m_reg[0x7c] & 0x08 ? color[1] : color[0]);

	pocketc_draw_special(bitmap, m_RIGHT+266, m_DOWN-13, braces,
				m_reg[0x7c] & 0x04 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+274, m_DOWN-13, m,
				m_reg[0x7c] & 0x02 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+281, m_DOWN-13, e,
				m_reg[0x7c] & 0x01 ? color[1] : color[0]);

	pocketc_draw_special(bitmap, m_RIGHT+10, m_DOWN+27, line /* empty */,
				m_reg[0x3c] & 0x40 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+31, m_DOWN+27, line /*calc*/,
				m_reg[0x3d] & 0x40 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+52, m_DOWN+27, line/*run*/,
				m_reg[0x3d] & 0x20 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+73, m_DOWN+27, line/*prog*/,
				m_reg[0x3d] & 0x10 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+94, m_DOWN+27, line /* empty */,
				m_reg[0x3c] & 0x20 ? color[1] : color[0]);

	pocketc_draw_special(bitmap, m_RIGHT+232, m_DOWN+27, line/*matrix*/,
				m_reg[0x3c] & 0x10 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+253, m_DOWN+27, line/*stat*/,
				m_reg[0x3c] & 0x08 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, m_RIGHT+274, m_DOWN+27, line/*print*/,
				m_reg[0x7c] & 0x40 ? color[1] : color[0]);

	return 0;
}
