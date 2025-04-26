// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        PP-01 driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "pp01.h"

u32 pp01_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 const *const ram = m_ram->pointer();

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		for (int x = cliprect.left() & ~7; x <= cliprect.right(); x += 8)
		{
			u16 const t = ((y + m_video_scroll) & 0xff) * 32 + (x >> 3);
			u8 const code_r = ram[0x6000 + t];
			u8 const code_g = ram[0xa000 + t];
			u8 const code_b = ram[0xe000 + t];
			for (int b = 0; b < 8; b++)
			{
				u8 const col = (BIT(code_r, b) << 2) | (BIT(code_g, b) << 1) | (BIT(code_b, b) << 0);
				bitmap.pix(y, x + (7 - b)) = col;
			}
		}
	}
	return 0;
}

static constexpr rgb_t pp01_pens[8] = {
	{ 0x00, 0x00, 0x00 }, // 0
	{ 0x00, 0x00, 0x80 }, // 1
	{ 0x00, 0x80, 0x00 }, // 2
	{ 0x00, 0x80, 0x80 }, // 3
	{ 0x80, 0x00, 0x00 }, // 4
	{ 0x80, 0x00, 0x80 }, // 5
	{ 0x80, 0x80, 0x00 }, // 6
	{ 0x80, 0x80, 0x80 }  // 7
};

void pp01_state::pp01_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, pp01_pens);
}
