/* Diver Boy - Video Hardware */

#include "driver.h"

UINT16 *diverboy_spriteram;
size_t diverboy_spriteram_size;


VIDEO_START(diverboy)
{
}

static void draw_sprites(running_machine* machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT16 *source = diverboy_spriteram;
	UINT16 *finish = source + (diverboy_spriteram_size/2);

	while (source < finish)
	{
		INT16 xpos,ypos,number,colr,bank,flash;

		ypos = source[4];
		xpos = source[0];
		colr = (source[1]& 0x00f0) >> 4;
		number = source[3];
		flash = source[1] & 0x1000;

		colr |= ((source[1] & 0x000c) << 2);

		ypos = 0x100 - ypos;

		bank = (source[1]&0x0002) >> 1;

		if (!flash || (video_screen_get_frame_number(machine->primary_screen) & 1))
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[bank],
					number,
					colr,
					0,0,
					xpos,ypos,
					(source[1] & 0x0008) ? -1 : 0);
		}

		source+=8;
	}
}

VIDEO_UPDATE(diverboy)
{
//  bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}
