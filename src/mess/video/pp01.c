// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        PP-01 driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "includes/pp01.h"

void pp01_state::video_start()
{
}

UINT32 pp01_state::screen_update_pp01(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 code_r,code_g,code_b;
	UINT8 col;
	int y, x, b;
	UINT8 *ram = m_ram->pointer();

	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 32; x++)
		{
			code_r = ram[0x6000 + ((y+m_video_scroll)&0xff)*32 + x];
			code_g = ram[0xa000 + ((y+m_video_scroll)&0xff)*32 + x];
			code_b = ram[0xe000 + ((y+m_video_scroll)&0xff)*32 + x];
			for (b = 0; b < 8; b++)
			{
				col = (((code_r >> b) & 0x01) ? 4 : 0) + (((code_g >> b) & 0x01) ? 2 : 0) + (((code_b >> b) & 0x01) ? 1 : 0);
				bitmap.pix16(y, x*8+(7-b)) =  col;
			}
		}
	}
	return 0;
}

static const rgb_t pp01_palette[8] = {
	rgb_t(0x00, 0x00, 0x00), // 0
	rgb_t(0x00, 0x00, 0x80), // 1
	rgb_t(0x00, 0x80, 0x00), // 2
	rgb_t(0x00, 0x80, 0x80), // 3
	rgb_t(0x80, 0x00, 0x00), // 4
	rgb_t(0x80, 0x00, 0x80), // 5
	rgb_t(0x80, 0x80, 0x00), // 6
	rgb_t(0x80, 0x80, 0x80), // 7
};

PALETTE_INIT_MEMBER(pp01_state, pp01)
{
	palette.set_pen_colors(0, pp01_palette, ARRAY_LENGTH(pp01_palette));
}
