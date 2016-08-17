// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Vector06c driver by Miodrag Milanovic

        10/07/2008 Preliminary driver.

****************************************************************************/


#include "includes/vector06.h"


void vector06_state::video_start()
{
}

UINT32 vector06_state::screen_update_vector06(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 code1,code2,code3,code4;
	UINT8 col;
	int y, x, b,draw_y;
	UINT8 *ram = m_ram->pointer();

	int width = (m_video_mode==0x00) ? 256 : 512;
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
				col = ((code1 >> b) & 0x01) * 8 + ((code2 >> b) & 0x01) * 4 + ((code3 >> b) & 0x01)* 2+ ((code4 >> b) & 0x01);
				if (m_video_mode==0x00) {
					bitmap.pix16(draw_y, x*8+(7-b)+32) =  col;
				} else {
					bitmap.pix16(draw_y, x*16+(7-b)*2+1+32) =  ((code2 >> b) & 0x01) * 2;
					bitmap.pix16(draw_y, x*16+(7-b)*2+32)   =  ((code3 >> b) & 0x01) * 2;
				}
			}
		}
	}
	return 0;
}

PALETTE_INIT_MEMBER(vector06_state, vector06)
{
	for (UINT8 i=0; i<16; i++)
		m_palette->set_pen_color( i, rgb_t::black );
}
