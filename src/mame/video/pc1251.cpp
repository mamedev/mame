// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"

#include "includes/pocketc.h"
#include "includes/pc1251.h"

// TODO: Convert to SVG rendering or internal layout

#define LOG_LCD (1 << 0)

#define VERBOSE (0)
#include "logmacro.h"

const char *const pc1251_state::s_def[5] =
{
	"11  111 111",
	"1 1 1   1  ",
	"1 1 111 11 ",
	"1 1 1   1  ",
	"11  111 1  "
};
const char *const pc1251_state::s_shift[5] =
{
	" 11 1 1 1 111 111",
	"1   1 1 1 1    1 ",
	" 1  111 1 11   1 ",
	"  1 1 1 1 1    1 ",
	"11  1 1 1 1    1 "
};
const char *const pc1251_state::s_de[5] =
{
	"11  111",
	"1 1 1  ",
	"1 1 111",
	"1 1 1  ",
	"11  111"
};
const char *const pc1251_state::s_g[5] =
{
	" 11",
	"1  ",
	"1 1",
	"1 1",
	" 11"
};
const char *const pc1251_state::s_rad[5] =
{
	"11   1  11 ",
	"1 1 1 1 1 1",
	"11  111 1 1",
	"1 1 1 1 1 1",
	"1 1 1 1 11 "
};
const char *const pc1251_state::s_run[5] =
{
	"11  1 1 1  1",
	"1 1 1 1 11 1",
	"11  1 1 1 11",
	"1 1 1 1 1  1",
	"1 1  1  1  1"
};
const char *const pc1251_state::s_pro[5] =
{
	"11  11   1  ",
	"1 1 1 1 1 1 ",
	"11  11  1 1 ",
	"1   1 1 1 1 ",
	"1   1 1  1  "
};
const char *const pc1251_state::s_rsv[5] =
{
	"11   11 1   1",
	"1 1 1   1   1",
	"11   1   1 1 ",
	"1 1   1  1 1 ",
	"1 1 11    1  "
};

READ8_MEMBER(pc1251_state::lcd_read)
{
	uint8_t data = m_reg[offset & 0xff];
	LOGMASKED(LOG_LCD, "pc1251 read %.3x %.2x\n", offset, data);
	return data;
}

WRITE8_MEMBER(pc1251_state::lcd_write)
{
	LOGMASKED(LOG_LCD, "pc1251 write %.3x %.2x\n", offset, data);
	m_reg[offset & 0xff] = data;
}

#define DOWN 62
#define RIGHT 68

uint32_t pc1251_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int color[2] =
	{
		7, //pocketc_colortable[PC1251_CONTRAST][0],
		8, //pocketc_colortable[PC1251_CONTRAST][1]
	};

	bitmap.fill(11, cliprect);

	const int contrast = m_dsw0->read() & 7;
	int x = RIGHT;
	int y = DOWN;
	for (int i = 0; i < 60; x += 3)
		for (int j = 0; j < 5; j++, i++, x += 3)
			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, m_reg[i], contrast, 0, 0, x, y);

	for (int i = 0x7b; i >= 0x40; x += 3)
		for (int j = 0; j < 5; j++, i--, x += 3)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, m_reg[i], contrast, 0, 0, x, y);

	/* 0x3c 1 def?, 4 g, 8 de
	   0x3d 2 shift, 4 rad, 8 error
	   0x3e 1 pro?, 2 run?, 4rsv?*/

	pocketc_draw_special(bitmap, RIGHT+18,  DOWN-10, s_def,   BIT(m_reg[0x3c], 0) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+142, DOWN-10, s_g,     BIT(m_reg[0x3c], 2) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+134, DOWN-10, s_de,    BIT(m_reg[0x3c], 3) ? color[1] : color[0]);

	pocketc_draw_special(bitmap, RIGHT,     DOWN-10, s_shift, BIT(m_reg[0x3d], 1) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+146, DOWN-10, s_rad,   BIT(m_reg[0x3d], 2) ? color[1] : color[0]);

	pocketc_draw_special(bitmap, RIGHT+38,  DOWN-10, s_pro,   BIT(m_reg[0x3e], 0) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+53,  DOWN-10, s_run,   BIT(m_reg[0x3e], 1) ? color[1] : color[0]);
	pocketc_draw_special(bitmap, RIGHT+68,  DOWN-10, s_rsv,   BIT(m_reg[0x3e], 2) ? color[1] : color[0]);

	return 0;
}
