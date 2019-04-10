// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"

#include "includes/pocketc.h"
#include "includes/pc1401.h"

// TODO: Convert to SVG rendering or internal layout

/* pc140x
   16 5x7 with space between char
   6000 .. 6027, 6067.. 6040
   603c: 3 STAT
   603d: 0 BUSY, 1 DEF, 2 SHIFT, 3 HYP, 4 PRO, 5 RUN, 6 CAL
   607c: 0 E, 1 M, 2 (), 3 RAD, 4 G, 5 DE, 6 PRINT */

/* pc1421
   16 5x7 with space between char
   6000 .. 6027, 6067.. 6040
   603c: 3 RUN
   603d: 0 BUSY, 1 DEF, 2 SHIFT, 3 BGN, 4 STAT, 5 FIN, 6 PRINT
   607c: 0 E, 1 M, 2 BAL, 3 INT, 4 PRN, 5 Sum-Sign, 6 PRO */

READ8_MEMBER(pc1401_state::lcd_read)
{
	return m_reg[offset & 0xff];
}

WRITE8_MEMBER(pc1401_state::lcd_write)
{
	m_reg[offset & 0xff] = data;
}

const char* const pc1401_state::s_line[5] = /* simple line */
{
	"     ",
	"     ",
	"11111",
	"11111",
	"11111"
};
const char* const pc1401_state::s_busy[5] =
{
	"11  1 1  11 1 1",
	"1 1 1 1 1   1 1",
	"11  1 1  1  1 1",
	"1 1 1 1   1  1 ",
	"11   1  11   1 "
};
const char* const pc1401_state::s_def[5] =
{
	"11  111 111",
	"1 1 1   1  ",
	"1 1 111 11 ",
	"1 1 1   1  ",
	"11  111 1  "
};
const char* const pc1401_state::s_shift[5] =
{
	" 11 1 1 1 111 111",
	"1   1 1 1 1    1 ",
	" 1  111 1 11   1 ",
	"  1 1 1 1 1    1 ",
	"11  1 1 1 1    1 "
};
const char* const pc1401_state::s_hyp[5] =
{
	"1 1 1 1 11 ",
	"1 1 1 1 1 1",
	"111 1 1 11 ",
	"1 1  1  1  ",
	"1 1  1  1  "
};
const char* const pc1401_state::s_de[5] =
{
	"11  111",
	"1 1 1  ",
	"1 1 111",
	"1 1 1  ",
	"11  111"
};
const char* const pc1401_state::s_g[5] =
{
	" 11",
	"1  ",
	"1 1",
	"1 1",
	" 11"
};
const char* const pc1401_state::s_rad[5] =
{
	"11   1  11 ",
	"1 1 1 1 1 1",
	"11  111 1 1",
	"1 1 1 1 1 1",
	"1 1 1 1 11 "
};
const char* const pc1401_state::s_braces[5] =
{
	" 1 1 ",
	"1   1",
	"1   1",
	"1   1",
	" 1 1 "
};
const char* const pc1401_state::s_m[5] =
{
	"1   1",
	"11 11",
	"1 1 1",
	"1   1",
	"1   1"
};
const char* const pc1401_state::s_e[5] =
{
	"111",
	"1  ",
	"111",
	"1  ",
	"111"
};

#define DOWN 57
#define RIGHT 114

uint32_t pc1401_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int contrast = m_dsw0->read() & 7;
	int color[2] =
	{
#if 0
		pocketc_colortable[contrast][0],
		pocketc_colortable[contrast][1]
#endif
		/* Above can be unreadable or misleading at certain contrast settings, this is better */
		7,
		8
	};

	bitmap.fill(11, cliprect);

	if (BIT(m_portc, 0))
	{
		int x = RIGHT;
		int y = DOWN;
		for (int i = 0; i < 0x28; x += 2)
			for (int j = 0; j < 5; j++, i++, x += 2)
				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, m_reg[i], contrast, 0, 0, x, y);

		for (int i = 0x67; i >= 0x40; x += 2)
			for (int j = 0; j < 5; j++, i--, x+=2)
				m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, m_reg[i], contrast, 0, 0, x, y);
	}

/*
  603c: 3 STAT
  603d: 0 BUSY, 1 DEF, 2 SHIFT, 3 HYP, 4 PRO, 5 RUN, 6 CAL
  607c: 0 E, 1 M, 2 (), 3 RAD, 4 G, 5 DE, 6 PRINT
*/

	pocketc_draw_special(bitmap, RIGHT+149, DOWN+24, s_line,   BIT(m_reg[0x3c], 3) ? color[1] : color[0]);

	pocketc_draw_special(bitmap, RIGHT,     DOWN-10, s_busy,   BIT(m_reg[0x3d], 0) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+18,  DOWN-10, s_def,    BIT(m_reg[0x3d], 1) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+43,  DOWN-10, s_shift,  BIT(m_reg[0x3d], 2) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+63,  DOWN-10, s_hyp,    BIT(m_reg[0x3d], 3) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+38,  DOWN+24, s_line,   BIT(m_reg[0x3d], 4) ? color[1] : color[0]); // pro
	pocketc_draw_special(bitmap, RIGHT+23,  DOWN+24, s_line,   BIT(m_reg[0x3d], 5) ? color[1] : color[0]); // run
	pocketc_draw_special(bitmap, RIGHT+8,   DOWN+24, s_line,   BIT(m_reg[0x3d], 6) ? color[1] : color[0]); // cal

	pocketc_draw_special(bitmap, RIGHT+183, DOWN-10, s_e,      BIT(m_reg[0x7c], 0) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+176, DOWN-10, s_m,      BIT(m_reg[0x7c], 1) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+168, DOWN-10, s_braces, BIT(m_reg[0x7c], 2) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+138, DOWN-10, s_rad,    BIT(m_reg[0x7c], 3) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+134, DOWN-10, s_g,      BIT(m_reg[0x7c], 4) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+126, DOWN-10, s_de,     BIT(m_reg[0x7c], 5) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+165, DOWN+24, s_line,   BIT(m_reg[0x7c], 6) ? color[1] : color[0]); // print

	return 0;
}
