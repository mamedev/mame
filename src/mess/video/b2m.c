/***************************************************************************

        Bashkiria-2M video driver by Miodrag Milanovic

        28/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/b2m.h"
#include "machine/ram.h"

VIDEO_START( b2m )
{
}

SCREEN_UPDATE_IND16( b2m )
{
	b2m_state *state = screen.machine().driver_data<b2m_state>();
	UINT8 code1;
	UINT8 code2;
	UINT8 col;
	int y, x, b;
	UINT8 *ram = screen.machine().device<ram_device>(RAM_TAG)->pointer();

	for (x = 0; x < 48; x++)
	{
		for (y = 0; y < 256; y++)
		{
			if (state->m_b2m_video_page==0) {
				code1 = ram[0x11000 + x*256 + ((y + state->m_b2m_video_scroll) & 0xff)];
				code2 = ram[0x15000 + x*256 + ((y + state->m_b2m_video_scroll) & 0xff)];
			} else {
				code1 = ram[0x19000 + x*256 + ((y + state->m_b2m_video_scroll) & 0xff)];
				code2 = ram[0x1d000 + x*256 + ((y + state->m_b2m_video_scroll) & 0xff)];
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
	MAKE_RGB(0x00, 0x00, 0x00), // 0
	MAKE_RGB(0x00, 0x00, 0x00), // 1
	MAKE_RGB(0x00, 0x00, 0x00), // 2
	MAKE_RGB(0x00, 0x00, 0x00), // 3
};

PALETTE_INIT( b2m )
{
	palette_set_colors(machine, 0, b2m_palette, ARRAY_LENGTH(b2m_palette));
}
