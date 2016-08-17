// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        BK video driver by Miodrag Milanovic

        10/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/bk.h"


void bk_state::video_start()
{
}

UINT32 bk_state::screen_update_bk0010(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 code;
	int y, x, b;
	int nOfs;

	nOfs = (m_scrool - 728) % 256;

	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 32; x++)
		{
			code = m_bk0010_video_ram[((y+nOfs) %256)*32 + x];
			for (b = 0; b < 16; b++)
			{
				bitmap.pix16(y, x*16 + b) =  (code >> b) & 0x01;
			}
		}
	}
	return 0;
}
