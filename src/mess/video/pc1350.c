// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"

#include "includes/pocketc.h"
#include "includes/pc1350.h"

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
}, japan={
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
}/*, rsv={
    "11   11 1   1",
    "1 1 1   1   1",
    "11   1   1 1",
    "1 1   1  1 1",
    "1 1 11    1e"
}*/;

READ8_MEMBER(pc1350_state::pc1350_lcd_read)
{
	UINT8 data = m_reg[offset&0xfff];
	logerror("pc1350 read %.3x %.2x\n",offset,data);
	return data;
}

WRITE8_MEMBER(pc1350_state::pc1350_lcd_write)
{
	logerror("pc1350 write %.3x %.2x\n",offset,data);
	m_reg[offset&0xfff] = data;
}

READ8_MEMBER(pc1350_state::pc1350_keyboard_line_r)
{
	return m_reg[0xe00];
}

/* pc1350
   24x4 5x8 no space between chars
   7000 .. 701d, 7200..721d, 7400 ..741d, 7600 ..761d, 7800 .. 781d
   7040 .. 705d, 7240..725d, 7440 ..745d, 7640 ..765d, 7840 .. 785d
   701e .. 703b, 721e..723b, 741e ..743b, 761e ..763b, 781e .. 783b
   705e .. 707b, 725e..727b, 745e ..747b, 765e ..767b, 785e .. 787b
   783c: 0 SHIFT 1 DEF 4 RUN 5 PRO 6 JAPAN 7 SML */
static const int pc1350_addr[4]={ 0, 0x40, 0x1e, 0x5e };

#define DOWN 45
#define RIGHT 76

UINT32 pc1350_state::screen_update_pc1350(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y=DOWN, i, j, k=0, b;
	int color[4];
	bitmap.fill(11, cliprect);

	/* HJB: we cannot initialize array with values from other arrays, thus... */
	color[0] = pocketc_colortable[PC1350_CONTRAST][0];
	color[1] = pocketc_colortable[PC1350_CONTRAST][1];
	color[2] = 8;
	color[3] = 7;

	for (k=0, y=DOWN; k<4; y+=16, k++)
		for (x=RIGHT, i=pc1350_addr[k]; i<0xa00; i+=0x200)
			for (j=0; j<=0x1d; j++, x+=2)
				for (b = 0; b < 8; b++)
					bitmap.plot_box(x, y + b * 2, 2, 2, color[(m_reg[j+i] >> b) & 1]);


	/* 783c: 0 SHIFT 1 DEF 4 RUN 5 PRO 6 JAPAN 7 SML */
	/* I don't know how they really look like in the lcd */
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+45, shift,
						m_reg[0x83c] & 0x01 ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+55, def,
						m_reg[0x83c] & 0x02 ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+5, run,
						m_reg[0x83c] & 0x10 ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+15, pro,
						m_reg[0x83c] & 0x20 ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+25, japan,
						m_reg[0x83c] & 0x40 ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+35, sml,
						m_reg[0x83c] & 0x80 ? color[2] : color[3]);

	return 0;
}
