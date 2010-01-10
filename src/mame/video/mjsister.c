/*****************************************************************************

Mahjong Sisters (c) 1986 Toa Plan

Video hardware
    driver by Uki

*****************************************************************************/

#include "emu.h"

int mjsister_screen_redraw;
int mjsister_flip_screen;
int mjsister_video_enable;

int mjsister_vrambank;
int mjsister_colorbank;

static bitmap_t *mjsister_tmpbitmap0, *mjsister_tmpbitmap1;
static UINT8 *mjsister_videoram0, *mjsister_videoram1;

/****************************************************************************/

VIDEO_START( mjsister )
{
	mjsister_tmpbitmap0 = auto_bitmap_alloc(machine,256,256,video_screen_get_format(machine->primary_screen));
	mjsister_tmpbitmap1 = auto_bitmap_alloc(machine,256,256,video_screen_get_format(machine->primary_screen));
	mjsister_videoram0 = auto_alloc_array(machine, UINT8, 0x8000);
	mjsister_videoram1 = auto_alloc_array(machine, UINT8, 0x8000);
}

static void mjsister_plot0(int offset,UINT8 data)
{
	int x,y,c1,c2;

	x = offset & 0x7f;
	y = offset / 0x80;

	c1 = (data & 0x0f)        + mjsister_colorbank * 0x20;
	c2 = ((data & 0xf0) >> 4) + mjsister_colorbank * 0x20;

	*BITMAP_ADDR16(mjsister_tmpbitmap0, y, x*2+0) = c1;
	*BITMAP_ADDR16(mjsister_tmpbitmap0, y, x*2+1) = c2;
}

static void mjsister_plot1(int offset,UINT8 data)
{
	int x,y,c1,c2;

	x = offset & 0x7f;
	y = offset / 0x80;

	c1 = data & 0x0f;
	c2 = (data & 0xf0) >> 4;

	if (c1)
		c1 += mjsister_colorbank * 0x20 + 0x10;
	if (c2)
		c2 += mjsister_colorbank * 0x20 + 0x10;

	*BITMAP_ADDR16(mjsister_tmpbitmap1, y, x*2+0) = c1;
	*BITMAP_ADDR16(mjsister_tmpbitmap1, y, x*2+1) = c2;
}

WRITE8_HANDLER( mjsister_videoram_w )
{
	if (mjsister_vrambank)
	{
		mjsister_videoram1[offset] = data;
		mjsister_plot1(offset,data);
	}
	else
	{
		mjsister_videoram0[offset] = data;
		mjsister_plot0(offset,data);
	}
}

VIDEO_UPDATE( mjsister )
{
	int f = mjsister_flip_screen;
	int i,j;

	if (mjsister_screen_redraw)
	{
		int offs;

		for (offs=0; offs<0x8000; offs++)
		{
			mjsister_plot0(offs,mjsister_videoram0[offs]);
			mjsister_plot1(offs,mjsister_videoram1[offs]);
		}

		mjsister_screen_redraw = 0;
	}

	if (mjsister_video_enable)
	{
		for (i=0; i<256; i++)
			for (j=0; j<4; j++)
				*BITMAP_ADDR16(bitmap, i, 256+j) = mjsister_colorbank * 0x20;

		copybitmap      (bitmap,mjsister_tmpbitmap0,f,f,0,0,cliprect);
		copybitmap_trans(bitmap,mjsister_tmpbitmap1,f,f,2,0,cliprect,0);
	}
	else
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	return 0;
}
