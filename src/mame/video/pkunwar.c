/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


static int flipscreen[2];


WRITE8_HANDLER( pkunwar_flipscreen_w )
{
	if (flipscreen[0] != (data & 1))
	{
		flipscreen[0] = data & 1;
		memset(dirtybuffer,1,videoram_size);
	}
	if (flipscreen[1] != (data & 2))
	{
		flipscreen[1] = data & 2;
		memset(dirtybuffer,1,videoram_size);
	}
}



VIDEO_UPDATE( pkunwar )
{
	int offs;


	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		if (dirtybuffer[offs])
		{
			int sx,sy;


			dirtybuffer[offs] = 0;

			sx = offs % 32;
			sy = offs / 32;
			if (flipscreen[0]) sx = 31 - sx;
			if (flipscreen[1]) sy = 31 - sy;

			drawgfx(tmpbitmap,machine->gfx[0],
					videoram[offs] + ((colorram[offs] & 0x07) << 8),
					(colorram[offs] & 0xf0) >> 4,
					flipscreen[0],flipscreen[1],
					8*sx,8*sy,
					&machine->screen[0].visarea,TRANSPARENCY_NONE,0);
		}
	}

	/* copy the character mapped graphics */
	copybitmap(bitmap,tmpbitmap,0,0,0,0,&machine->screen[0].visarea,TRANSPARENCY_NONE,0);


	/* Draw the sprites. */
	for (offs = 0;offs < spriteram_size;offs += 32)
	{
		int sx,sy,flipx,flipy;


		sx = ((spriteram[offs + 1] + 8) & 0xff) - 8;
		sy = spriteram[offs + 2];
		flipx = spriteram[offs] & 0x01;
		flipy = spriteram[offs] & 0x02;
		if (flipscreen[0])
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flipscreen[1])
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[1],
				((spriteram[offs] & 0xfc) >> 2) + ((spriteram[offs + 3] & 0x07) << 6),
				(spriteram[offs + 3] & 0xf0) >> 4,
				flipx,flipy,
				sx,sy,
				&machine->screen[0].visarea,TRANSPARENCY_PEN,0);
	}


	/* redraw characters which have priority over sprites */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		if (colorram[offs] & 0x08)
		{
			int sx,sy;


			sx = offs % 32;
			sy = offs / 32;
			if (flipscreen[0]) sx = 31 - sx;
			if (flipscreen[1]) sy = 31 - sy;

			drawgfx(bitmap,machine->gfx[0],
					videoram[offs] + ((colorram[offs] & 0x07) << 8),
					(colorram[offs] & 0xf0) >> 4,
					flipscreen[0],flipscreen[1],
					8*sx,8*sy,
					&machine->screen[0].visarea,TRANSPARENCY_PEN,0);
		}
	}
	return 0;
}
