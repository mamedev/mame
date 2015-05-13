// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"

#include "includes/pocketc.h"
#include "includes/pc1251.h"

static const POCKETC_FIGURE /*busy={
    "11  1 1  11 1 1",
    "1 1 1 1 1   1 1",
    "11  1 1  1  1 1",
    "1 1 1 1   1  1",
    "11   1  11   1e"
},*/ def={
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
}, /*hyp={
    "1 1 1 1 11",
    "1 1 1 1 1 1",
    "111 1 1 11",
    "1 1  1  1",
    "1 1  1  1e"
},*/ de={
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
}, /*braces={
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
},*/ run={
	"11  1 1 1  1",
	"1 1 1 1 11 1",
	"11  1 1 1 11",
	"1 1 1 1 1  1",
	"1 1  1  1  1e"
}, pro={
	"11  11   1  ",
	"1 1 1 1 1 1",
	"11  11  1 1",
	"1   1 1 1 1",
	"1   1 1  1e"
}, /*japan={
    "  1  1  11   1  1  1",
    "  1 1 1 1 1 1 1 11 1",
    "  1 111 11  111 1 11",
    "1 1 1 1 1   1 1 1  1",
    " 1  1 1 1   1 1 1  1e"
}, sml={
    " 11 1 1 1",
    "1   111 1",
    " 1  1 1 1",
    "  1 1 1 1",
    "11  1 1 111e"
},*/ rsv={
	"11   11 1   1",
	"1 1 1   1   1",
	"11   1   1 1",
	"1 1   1  1 1",
	"1 1 11    1e"
};

READ8_MEMBER(pc1251_state::pc1251_lcd_read)
{
	UINT8 data = m_reg[offset&0xff];
	logerror("pc1251 read %.3x %.2x\n",offset,data);
	return data;
}

WRITE8_MEMBER(pc1251_state::pc1251_lcd_write)
{
	logerror("pc1251 write %.3x %.2x\n",offset,data);
	m_reg[offset&0xff] = data;
}

#define DOWN 62
#define RIGHT 68

UINT32 pc1251_state::screen_update_pc1251(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y, i, j;
	int color[2];

	bitmap.fill(11, cliprect);

	/* HJB: we cannot initialize array with values from other arrays, thus... */
	color[0] = 7; //pocketc_colortable[PC1251_CONTRAST][0];
	color[1] = 8; //pocketc_colortable[PC1251_CONTRAST][1];

	for (x=RIGHT,y=DOWN, i=0; i<60; x+=3)
	{
		for (j=0; j<5; j++, i++, x+=3)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_reg[i],
					PC1251_CONTRAST,0,0,
					x,y);
	}
	for (i=0x7b; i>=0x40; x+=3)
	{
		for (j=0; j<5; j++, i--, x+=3)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_reg[i],
					PC1251_CONTRAST,0,0,
					x,y);
	}

	pocketc_draw_special(bitmap, RIGHT+134, DOWN-10, de,
						m_reg[0x3c] & 0x08 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+142, DOWN-10, g,
						m_reg[0x3c] & 0x04 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+146, DOWN-10, rad,
						m_reg[0x3d] & 0x04 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+18, DOWN-10, def,
						m_reg[0x3c] & 0x01 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT, DOWN-10, shift,
						m_reg[0x3d] & 0x02 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+38, DOWN-10, pro,
						m_reg[0x3e] & 0x01 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+53, DOWN-10, run,
						m_reg[0x3e] & 0x02 ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+68, DOWN-10, rsv,
						m_reg[0x3e] & 0x04 ? color[1] : color[0]);

	/* 0x3c 1 def?, 4 g, 8 de
	   0x3d 2 shift, 4 rad, 8 error
	   0x3e 1 pro?, 2 run?, 4rsv?*/
	return 0;
}
