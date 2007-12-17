/***************************************************************************

  Poly-Play
  (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

  video hardware

  driver written by Martin Buchholz (buchholz@mail.uni-greifswald.de)

***************************************************************************/

#include "driver.h"


UINT8 *polyplay_characterram;
static UINT8 dirtycharacter[256];



PALETTE_INIT( polyplay )
{
	palette_set_color(machine,0,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine,1,MAKE_RGB(0xff,0xff,0xff));

	palette_set_color(machine,2,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine,3,MAKE_RGB(0xff,0x00,0x00));
	palette_set_color(machine,4,MAKE_RGB(0x00,0xff,0x00));
	palette_set_color(machine,5,MAKE_RGB(0xff,0xff,0x00));
	palette_set_color(machine,6,MAKE_RGB(0x00,0x00,0xff));
	palette_set_color(machine,7,MAKE_RGB(0xff,0x00,0xff));
	palette_set_color(machine,8,MAKE_RGB(0x00,0xff,0xff));
	palette_set_color(machine,9,MAKE_RGB(0xff,0xff,0xff));
}


WRITE8_HANDLER( polyplay_characterram_w )
{
	if (polyplay_characterram[offset] != data)
	{
		dirtycharacter[((offset >> 3) & 0x7f) | 0x80] = 1;

		polyplay_characterram[offset] = data;
	}
}


VIDEO_UPDATE( polyplay )
{
	offs_t offs;


	for (offs = 0; offs < videoram_size; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = offs >> 6 << 3;
		UINT8 code = videoram[offs];

		if (dirtycharacter[code])
		{
			decodechar(machine->gfx[1], code & 0x7f, polyplay_characterram, machine->drv->gfxdecodeinfo[1].gfxlayout);

			dirtycharacter[code] = 0;
		}

		drawgfx(bitmap,machine->gfx[(code >> 7) & 0x01],
				code, 0, 0, 0, sx, sy,
				&machine->screen[0].visarea, TRANSPARENCY_NONE, 0);
	}

	return 0;
}
