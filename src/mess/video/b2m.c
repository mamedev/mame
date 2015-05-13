// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Bashkiria-2M video driver by Miodrag Milanovic

        28/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/b2m.h"
#include "machine/ram.h"

void b2m_state::video_start()
{
}

UINT32 b2m_state::screen_update_b2m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 code1;
	UINT8 code2;
	UINT8 col;
	int y, x, b;
	UINT8 *ram = m_ram->pointer();

	for (x = 0; x < 48; x++)
	{
		for (y = 0; y < 256; y++)
		{
			if (m_b2m_video_page==0) {
				code1 = ram[0x11000 + x*256 + ((y + m_b2m_video_scroll) & 0xff)];
				code2 = ram[0x15000 + x*256 + ((y + m_b2m_video_scroll) & 0xff)];
			} else {
				code1 = ram[0x19000 + x*256 + ((y + m_b2m_video_scroll) & 0xff)];
				code2 = ram[0x1d000 + x*256 + ((y + m_b2m_video_scroll) & 0xff)];
			}
			for (b = 7; b >= 0; b--)
			{
				col = (((code2 >> b) & 0x01)<<1) + ((code1 >> b) & 0x01);
				bitmap.pix16(y, x*8+b) =  col;
			}
		}
	}

	return 0;
}

static const rgb_t b2m_palette[4] = {
	rgb_t(0x00, 0x00, 0x00), // 0
	rgb_t(0x00, 0x00, 0x00), // 1
	rgb_t(0x00, 0x00, 0x00), // 2
	rgb_t(0x00, 0x00, 0x00), // 3
};

PALETTE_INIT_MEMBER(b2m_state, b2m)
{
	palette.set_pen_colors(0, b2m_palette, ARRAY_LENGTH(b2m_palette));
}
