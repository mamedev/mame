/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

  (c) 12/2/1998 Lee Taylor

***************************************************************************/

#include "driver.h"



UINT8 *iremm15_chargen;
static int bottomline;


WRITE8_HANDLER( skychut_colorram_w )
{
	colorram[offset] = data;
}

WRITE8_HANDLER( skychut_ctrl_w )
{
//popmessage("%02x",data);

	/* I have NO IDEA if this is correct or not */
	bottomline = ~data & 0x20;
}


VIDEO_UPDATE( skychut )
{
	int offs;

	fillbitmap(bitmap,machine->pens[7],cliprect);

	for (offs = 0;offs < 0x400;offs++)
	{
		int mask=iremm15_chargen[offs];
		int x = offs / 256;
		int y = offs % 256;
		int col = 0;

		switch (x)
		{
			case 0: x = 4*8;  col = 3; break;
			case 1: x = 26*8; col = 3; break;
			case 2: x = 7*8;  col = 5; break;
			case 3: x = 6*8;  col = 5; break;
		}

		if (x >= cliprect->min_x && x+7 <= cliprect->max_x
				&& y >= cliprect->min_y && y <= cliprect->max_y)
		{
			if (mask&0x80) *BITMAP_ADDR16(bitmap, y, x+0) = col;
			if (mask&0x40) *BITMAP_ADDR16(bitmap, y, x+1) = col;
			if (mask&0x20) *BITMAP_ADDR16(bitmap, y, x+2) = col;
			if (mask&0x10) *BITMAP_ADDR16(bitmap, y, x+3) = col;
			if (mask&0x08) *BITMAP_ADDR16(bitmap, y, x+4) = col;
			if (mask&0x04) *BITMAP_ADDR16(bitmap, y, x+5) = col;
			if (mask&0x02) *BITMAP_ADDR16(bitmap, y, x+6) = col;
			if (mask&0x01) *BITMAP_ADDR16(bitmap, y, x+7) = col;
		}
	}

	if (bottomline)
	{
		int y;

		for (y = cliprect->min_y;y <= cliprect->max_y;y++)
		{
			*BITMAP_ADDR16(bitmap, y, 16) = 0;
		}
	}

	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy;

		sx = 31 - offs / 32;
		sy = offs % 32;

		drawgfx(bitmap,machine->gfx[0],
				videoram[offs],
				colorram[offs],
				0,0,
				8*sx,8*sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
	return 0;
}


static void iremm15_drawgfx(mame_bitmap *bitmap, int ch,
							INT16 color, INT16 back, int x, int y)
{
	UINT8 mask;
	int i;

	for (i=0; i<8; i++, y++) {
		mask=iremm15_chargen[ch*8+i];
		*BITMAP_ADDR16(bitmap, y, x+0) = mask&0x80?color:back;
		*BITMAP_ADDR16(bitmap, y, x+1) = mask&0x40?color:back;
		*BITMAP_ADDR16(bitmap, y, x+2) = mask&0x20?color:back;
		*BITMAP_ADDR16(bitmap, y, x+3) = mask&0x10?color:back;
		*BITMAP_ADDR16(bitmap, y, x+4) = mask&0x08?color:back;
		*BITMAP_ADDR16(bitmap, y, x+5) = mask&0x04?color:back;
		*BITMAP_ADDR16(bitmap, y, x+6) = mask&0x02?color:back;
		*BITMAP_ADDR16(bitmap, y, x+7) = mask&0x01?color:back;
	}
}


VIDEO_UPDATE( iremm15 )
{
	int offs;

	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy;


		sx = 31 - offs / 32;
		sy = offs % 32;

		iremm15_drawgfx(tmpbitmap,
						videoram[offs],
						machine->pens[colorram[offs] & 7],
						machine->pens[7], // space beam not color 0
						8*sx,8*sy);
	}

	copybitmap(bitmap,tmpbitmap,0,0,0,0,&machine->screen[0].visarea,TRANSPARENCY_NONE,0);
	return 0;
}

