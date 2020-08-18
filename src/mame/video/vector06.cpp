// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Vector06c driver by Miodrag Milanovic

        10/07/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/vector06.h"


uint32_t vector06_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t code1,code2,code3,code4;
	uint8_t col;
	int y, x, b,draw_y;
	uint8_t *ram = m_ram->pointer();

	u16 width = (m_video_mode) ? 512 : 256;
	rectangle screen_area(0,width+64-1,0,256+64-1);
	// fill border color
	bitmap.fill(m_color_index, screen_area);

	// draw image
	for (x = 0; x < 32; x++)
	{
		for (y = 0; y < 256; y++)
		{
			// port A of 8255 also used as scroll
			draw_y = ((255-y-m_keyboard_mask) & 0xff) +32;
			code1 = ram[0x8000 + x*256 + y];
			code2 = ram[0xa000 + x*256 + y];
			code3 = ram[0xc000 + x*256 + y];
			code4 = ram[0xe000 + x*256 + y];
			for (b = 0; b < 8; b++)
			{
				col = BIT(code1, b) * 8 + BIT(code2, b) * 4 + BIT(code3, b)* 2+ BIT(code4, b);
				if (!m_video_mode)
					bitmap.pix16(draw_y, x*8+(7-b)+32) = col;
				else
				{
					bitmap.pix16(draw_y, x*16+(7-b)*2+1+32) = BIT(code2, b) * 2;
					bitmap.pix16(draw_y, x*16+(7-b)*2+32)   = BIT(code3, b) * 2;
				}
			}
		}
	}
	return 0;
}
