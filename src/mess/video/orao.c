/***************************************************************************

        Orao video driver by Miodrag Milanovic

        01/03/2008 Updated to work with latest SVN code
        22/02/2008 Preliminary driver.

****************************************************************************/

#include "emu.h"
#include "includes/orao.h"


VIDEO_START( orao )
{
}

SCREEN_UPDATE_IND16( orao )
{
	orao_state *state = screen.machine().driver_data<orao_state>();
	UINT8 code;
	int y, x, b;

	int addr = 0;
	for (y = 0; y < 256; y++)
	{
		int horpos = 0;
		for (x = 0; x < 32; x++)
		{
			code = state->m_video_ram[addr++];
			for (b = 0; b < 8; b++)
			{
				bitmap.pix16(y, horpos++) =  (code >> b) & 0x01;
			}
		}
	}
	return 0;
}


