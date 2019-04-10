// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        PK-8020 driver by Miodrag Milanovic
            based on work of Sergey Erokhin from pk8020.narod.ru

        18/07/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "includes/pk8020.h"
#include "machine/ram.h"

uint32_t pk8020_state::screen_update_pk8020(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *ram = m_ram->pointer();

	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			uint8_t chr = ram[x +(y*64) + 0x40000];
			uint8_t attr= ram[x +(y*64) + 0x40400];
			for (int j = 0; j < 16; j++)
			{
				uint32_t addr = 0x10000 + x + ((y*16+j)*64) + (m_video_page * 0xC000);
				uint8_t code1 = ram[addr];
				uint8_t code2 = ram[addr + 0x4000];
				uint8_t code3 = ram[addr + 0x8000];
				uint8_t code4 = m_region_gfx1[((chr<<4) + j) + (m_font*0x1000)];
				if (attr) code4 ^= 0xff;
				for (int b = 0; b < 8; b++)
				{
					uint8_t col = (((code4 >> b) & 0x01) ? 0x08 : 0x00);
					col |= (((code3 >> b) & 0x01) ? 0x04 : 0x00);
					col |= (((code2 >> b) & 0x01) ? 0x02 : 0x00);
					col |= (((code1 >> b) & 0x01) ? 0x01 : 0x00);
					bitmap.pix16((y*16)+j, x*8+(7-b)) =  col;
				}
			}
		}
	}
	return 0;
}

void pk8020_state::pk8020_palette(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
		palette.set_pen_color(i, rgb_t(i * 0x10, i * 0x10, i * 0x10)); // FIXME: if this is supposed to be a 4-bit ramp it should be 0x11, not 0x10
}
