/***************************************************************************

        PK-8020 driver by Miodrag Milanovic
            based on work of Sergey Erokhin from pk8020.narod.ru

        18/07/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "includes/pk8020.h"
#include "machine/ram.h"

VIDEO_START( pk8020 )
{
}

SCREEN_UPDATE_IND16( pk8020 )
{
	pk8020_state *state = screen.machine().driver_data<pk8020_state>();
	int y, x, b, j;
	UINT8 *gfx = state->memregion("gfx1")->base();
	UINT8 *ram = screen.machine().device<ram_device>(RAM_TAG)->pointer();

	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 64; x++)
		{
			UINT8 chr = ram[x +(y*64) + 0x40000];
			UINT8 attr= ram[x +(y*64) + 0x40400];
			for (j = 0; j < 16; j++) {
				UINT32 addr = 0x10000 + x + ((y*16+j)*64) + (state->m_video_page * 0xC000);
				UINT8 code1 = ram[addr];
				UINT8 code2 = ram[addr + 0x4000];
				UINT8 code3 = ram[addr + 0x8000];
				UINT8 code4 = gfx[((chr<<4) + j) + (state->m_font*0x1000)];
				if (attr) code4 ^= 0xff;
				for (b = 0; b < 8; b++)
				{
					UINT8 col = (((code4 >> b) & 0x01) ? 0x08 : 0x00);
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

PALETTE_INIT( pk8020 )
{
	int i;
	for(i=0;i<16;i++) {
		palette_set_color( machine, i, MAKE_RGB(i*0x10,i*0x10,i*0x10) );
	}
}
