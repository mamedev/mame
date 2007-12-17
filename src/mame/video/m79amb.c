/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"



static UINT8 mask = 0;

WRITE8_HANDLER( ramtek_mask_w )
{
	mask = data;
}

WRITE8_HANDLER( ramtek_videoram_w )
{
	data = data & ~mask;

	if (videoram[offset] != data)
	{
		int i,x,y;

		videoram[offset] = data;

		y = offset / 32;
		x = 8 * (offset % 32);

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? RGB_WHITE : RGB_BLACK;
			*BITMAP_ADDR32(tmpbitmap, y, x) = pen;

			x++;
			data <<= 1;
		}
	}
}
