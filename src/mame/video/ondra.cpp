// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Ondra driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/ondra.h"
#include "machine/ram.h"



void ondra_state::video_start()
{
	m_video_enable = 0;
}

UINT32 ondra_state::screen_update_ondra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 code1,code2;
	int y, x, b;
	int Vaddr = 0x2800;

	if (m_video_enable==1) {
		for (x = 0; x < 40; x++)
		{
			for (y = 127; y >=0; y--)
			{
				code1 = m_ram->pointer()[0xd700 + Vaddr + 0x80];
				code2 = m_ram->pointer()[0xd700 + Vaddr + 0x00];
				for (b = 0; b < 8; b++)
				{
					bitmap.pix16(2*y, x*8+b) =  ((code1 << b) & 0x80) ? 1 : 0;
					bitmap.pix16(2*y+1, x*8+b) =  ((code2 << b) & 0x80) ? 1 : 0;
				}
				Vaddr++;
			}
			Vaddr = (Vaddr - 128) - 256;
		}
	}
	return 0;
}
