#include "driver.h"
#include "deprecat.h"



UINT16 *blockout_videoram;
UINT16 *blockout_frontvideoram;



static void setcolor(int color,int rgb)
{
	int bit0,bit1,bit2,bit3;
	int r,g,b;


	/* red component */
	bit0 = (rgb >> 0) & 0x01;
	bit1 = (rgb >> 1) & 0x01;
	bit2 = (rgb >> 2) & 0x01;
	bit3 = (rgb >> 3) & 0x01;
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* green component */
	bit0 = (rgb >> 4) & 0x01;
	bit1 = (rgb >> 5) & 0x01;
	bit2 = (rgb >> 6) & 0x01;
	bit3 = (rgb >> 7) & 0x01;
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* blue component */
	bit0 = (rgb >> 8) & 0x01;
	bit1 = (rgb >> 9) & 0x01;
	bit2 = (rgb >> 10) & 0x01;
	bit3 = (rgb >> 11) & 0x01;
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	palette_set_color(Machine,color,MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( blockout_paletteram_w )
{
	COMBINE_DATA(&paletteram16[offset]);
	setcolor(offset,paletteram16[offset]);
}

WRITE16_HANDLER( blockout_frontcolor_w )
{
	static UINT16 color;

	COMBINE_DATA(&color);
	setcolor(512,color);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
VIDEO_START( blockout )
{
	/* Allocate temporary bitmaps */
	tmpbitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
}



static void update_pixels(const device_config *screen, int x, int y)
{
	UINT16 front,back;
	int color;
	const rectangle *visarea = video_screen_get_visible_area(screen);

	if (x < visarea->min_x || x > visarea->max_x || y < visarea->min_y || y > visarea->max_y)
		return;

	front = blockout_videoram[y*256+x/2];
	back = blockout_videoram[0x10000 + y*256+x/2];

	if (front>>8) color = front>>8;
	else color = (back>>8) + 256;
	*BITMAP_ADDR16(tmpbitmap, y, x) = color;

	if (front&0xff) color = front&0xff;
	else color = (back&0xff) + 256;
	*BITMAP_ADDR16(tmpbitmap, y, x+1) = color;
}



WRITE16_HANDLER( blockout_videoram_w )
{
	UINT16 oldword = blockout_videoram[offset];
	COMBINE_DATA(&blockout_videoram[offset]);

	if (oldword != blockout_videoram[offset])
		update_pixels(machine->primary_screen, (offset % 256)*2, (offset / 256) % 256);
}



VIDEO_UPDATE( blockout )
{
	copybitmap(bitmap,tmpbitmap,0,0,0,0,cliprect);

	{
		int x,y;

		pen_t color = 512;

		for (y = 0;y < 256;y++)
		{
			for (x = 0;x < 320;x+=8)
			{
				int d = blockout_frontvideoram[y*64+(x/8)];

				if (d)
				{
					if (d&0x80) *BITMAP_ADDR16(bitmap, y, x+0) = color;
					if (d&0x40) *BITMAP_ADDR16(bitmap, y, x+1) = color;
					if (d&0x20) *BITMAP_ADDR16(bitmap, y, x+2) = color;
					if (d&0x10) *BITMAP_ADDR16(bitmap, y, x+3) = color;
					if (d&0x08) *BITMAP_ADDR16(bitmap, y, x+4) = color;
					if (d&0x04) *BITMAP_ADDR16(bitmap, y, x+5) = color;
					if (d&0x02) *BITMAP_ADDR16(bitmap, y, x+6) = color;
					if (d&0x01) *BITMAP_ADDR16(bitmap, y, x+7) = color;
				}
			}
		}
	}
	return 0;
}
