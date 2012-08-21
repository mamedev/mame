/***************************************************************************

        Mikro-80 video driver by Miodrag Milanovic

        10/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/mikro80.h"


VIDEO_START( mikro80 )
{
}

SCREEN_UPDATE_IND16( mikro80 )
{
	mikro80_state *state = screen.machine().driver_data<mikro80_state>();
	UINT8 *gfx = state->memregion("gfx1")->base();
	int x,y,b;

	for(y = 0; y < 32*8; y++ )
	{
		for(x = 0; x < 64; x++ )
		{
			int addr = x + (y / 8)*64;
			UINT8 code = gfx[state->m_video_ram [addr]*8+ (y % 8)];
			UINT8 attr = state->m_cursor_ram[addr+1] & 0x80 ? 1 : 0;
			for (b = 7; b >= 0; b--)
			{
				UINT8 col = (code >> b) & 0x01;
				bitmap.pix16(y, x*8+(7-b)) =  attr ? col ^ 1 : col;
			}
		}
	}
	return 0;
}
