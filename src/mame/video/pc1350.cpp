// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"

#include "includes/pocketc.h"
#include "includes/pc1350.h"

// TODO: Convert to SVG rendering or internal layout

#define LOG_LCD (1 << 0)

#define VERBOSE (0)
#include "logmacro.h"

const char* const pc1350_state::s_def[5] =
{
	"11  111 111",
	"1 1 1   1  ",
	"1 1 111 11 ",
	"1 1 1   1  ",
	"11  111 1  "
};
const char* const pc1350_state::s_shift[5] =
{
	" 11 1 1 1 111 111",
	"1   1 1 1 1    1 ",
	" 1  111 1 11   1 ",
	"  1 1 1 1 1    1 ",
	"11  1 1 1 1    1 "
};
const char* const pc1350_state::s_run[5] =
{
	"11  1 1 1  1",
	"1 1 1 1 11 1",
	"11  1 1 1 11",
	"1 1 1 1 1  1",
	"1 1  1  1  1"
};
const char* const pc1350_state::s_pro[5] =
{
	"11  11   1 ",
	"1 1 1 1 1 1",
	"11  11  1 1",
	"1   1 1 1 1",
	"1   1 1  1 "
};
const char* const pc1350_state::s_japan[5] =
{
	"  1  1  11   1  1  1",
	"  1 1 1 1 1 1 1 11 1",
	"  1 111 11  111 1 11",
	"1 1 1 1 1   1 1 1  1",
	" 1  1 1 1   1 1 1  1"
};
const char* const pc1350_state::s_sml[5] =
{
	" 11 1 1 1  ",
	"1   111 1  ",
	" 1  1 1 1  ",
	"  1 1 1 1  ",
	"11  1 1 111"
};

READ8_MEMBER(pc1350_state::lcd_read)
{
	uint8_t data = m_reg[offset & 0xfff];
	LOGMASKED(LOG_LCD, "pc1350 read %.3x %.2x\n",offset,data);
	return data;
}

WRITE8_MEMBER(pc1350_state::lcd_write)
{
	LOGMASKED(LOG_LCD, "pc1350 write %.3x %.2x\n",offset,data);
	m_reg[offset & 0xfff] = data;
}

READ8_MEMBER(pc1350_state::keyboard_line_r)
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

uint32_t pc1350_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int contrast = m_dsw0->read() & 7;
	int color[4] =
	{
		colortable[contrast][0],
		colortable[contrast][1],
		8,
		7
	};

	bitmap.fill(11, cliprect);

	for (int k = 0, y = DOWN; k < 4; y += 16, k++)
		for (int x = RIGHT, i = pc1350_addr[k]; i < 0xa00; i += 0x200)
			for (int j = 0; j <= 0x1d; j++, x+=2)
				for (int bit = 0; bit < 8; bit++)
					bitmap.plot_box(x, y + bit * 2, 2, 2, color[BIT(m_reg[j+i], bit)]);

	/* 783c: 0 SHIFT 1 DEF 4 RUN 5 PRO 6 JAPAN 7 SML */
	/* I don't know how they really look like on the LCD */
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+45, s_shift, BIT(m_reg[0x83c], 0) ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+55, s_def,   BIT(m_reg[0x83c], 1) ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+5,  s_run,   BIT(m_reg[0x83c], 4) ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+15, s_pro,   BIT(m_reg[0x83c], 5) ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+25, s_japan, BIT(m_reg[0x83c], 6) ? color[2] : color[3]);
	pocketc_draw_special(bitmap, RIGHT-30, DOWN+35, s_sml,   BIT(m_reg[0x83c], 7) ? color[2] : color[3]);

	return 0;
}
