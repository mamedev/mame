/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"



UINT8 *goldstar_video1, *goldstar_video2, *goldstar_video3;
size_t goldstar_video_size;
UINT8 *goldstar_scroll1, *goldstar_scroll2, *goldstar_scroll3;

static bitmap_t *tmpbitmap1, *tmpbitmap2, *tmpbitmap3, *tmpbitmap4;
static int bgcolor;



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
VIDEO_START( goldstar )
{
//        int i;

	/* the background area is half as high as the screen */
	tmpbitmap1 = video_screen_auto_bitmap_alloc(machine->primary_screen);
	tmpbitmap2 = video_screen_auto_bitmap_alloc(machine->primary_screen);
	tmpbitmap3 = video_screen_auto_bitmap_alloc(machine->primary_screen);
	tmpbitmap4 = video_screen_auto_bitmap_alloc(machine->primary_screen);
}




WRITE8_HANDLER( goldstar_fa00_w )
{
	/* bit 1 toggles continuously - might be irq enable or watchdog reset */

	/* bit 2 selects background gfx color (I think) */
	bgcolor = (data & 0x04) >> 2;
}



static const rectangle visible1 = { 14*8, (14+48)*8-1,  4*8,  (4+7)*8-1 };
static const rectangle visible2 = { 14*8, (14+48)*8-1, 12*8, (12+7)*8-1 };
static const rectangle visible3 = { 14*8, (14+48)*8-1, 20*8, (20+7)*8-1 };


VIDEO_UPDATE( goldstar )
{
	int offs;


	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy;


		sx = offs % 64;
		sy = offs / 64;

		drawgfx(bitmap,screen->machine->gfx[0],
				videoram[offs] + ((colorram[offs] & 0xf0) << 4),
				colorram[offs] & 0x0f,
				0,0,
				8*sx,8*sy,
				0,TRANSPARENCY_NONE,0);
	}

	copybitmap(bitmap,tmpbitmap4,0,0,0,0,cliprect);


	for (offs = goldstar_video_size - 1;offs >= 0;offs--)
	{
		int sx = offs % 64;
		int sy = offs / 64;


		drawgfx(tmpbitmap1,screen->machine->gfx[1],
				goldstar_video1[offs],
				bgcolor,
				0,0,
				sx*8,sy*32,
				0,TRANSPARENCY_NONE,0);

		drawgfx(tmpbitmap2,screen->machine->gfx[1],
				goldstar_video2[offs],
				bgcolor,
				0,0,
				sx*8,sy*32,
				0,TRANSPARENCY_NONE,0);

		drawgfx(tmpbitmap3,screen->machine->gfx[1],
				goldstar_video3[offs],
				bgcolor,
				0,0,
				sx*8,sy*32,
				0,TRANSPARENCY_NONE,0);
	}

	{
		int i,scrolly[64];

		for (i= 0;i < 64;i++)
			scrolly[i] = -goldstar_scroll1[i];

		copyscrollbitmap(bitmap,tmpbitmap1,0,0,64,scrolly,&visible1);
		copybitmap_trans(bitmap,tmpbitmap4,0,0,0,0,&visible1,0);

		for (i= 0;i < 64;i++)
			scrolly[i] = -goldstar_scroll2[i];

		copyscrollbitmap(bitmap,tmpbitmap2,0,0,64,scrolly,&visible2);
		copybitmap_trans(bitmap,tmpbitmap4,0,0,0,0,&visible2,0);

		for (i= 0;i < 64;i++)
			scrolly[i] = -goldstar_scroll3[i];

		copyscrollbitmap(bitmap,tmpbitmap3,0,0,64,scrolly,&visible3);
		copybitmap_trans(bitmap,tmpbitmap4,0,0,0,0,&visible3,0);
	}
	return 0;
}
