/***************************************************************************

        Irisha video driver by Miodrag Milanovic

        27/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/irisha.h"


void irisha_state::video_start()
{
}

SCREEN_UPDATE_IND16( irisha )
{
	UINT8 code1; //, code2;
	UINT8 col;
	int y, x, b;
	address_space &space = *screen.machine().device("maincpu")->memory().space(AS_PROGRAM);

	// draw image
	for (y = 0; y < 200; y++)
	{
		for (x = 0; x < 40; x++)
		{
			code1 = space.read_byte(0xe000 + x + y * 40);
//          code2 = space.read_byte(0xc000 + x + y * 40);
			for (b = 0; b < 8; b++)
			{
				col = ((code1 >> b) & 0x01);
				bitmap.pix16(y, x * 8 + (7 - b)) =  col;
			}
		}
	}


	return 0;
}

