/*******************************************************************************

XX Mission (c) 1986 UPL

Video hardware driver by Uki

    31/Mar/2001 -

*******************************************************************************/

#include "driver.h"

UINT8 *xxmissio_fgram;

static UINT8 xxmissio_xscroll,xxmissio_yscroll;
static UINT8 flipscreen;
static UINT8 xxmissio_bg_redraw;


WRITE8_HANDLER( xxmissio_scroll_x_w )
{
	xxmissio_xscroll = data;
}
WRITE8_HANDLER( xxmissio_scroll_y_w )
{
	xxmissio_yscroll = data;
}

WRITE8_HANDLER( xxmissio_flipscreen_w )
{
	if ((data & 0x01) != flipscreen)
	{
		flipscreen = data & 0x01;
		xxmissio_bg_redraw = 1;
	}
}

WRITE8_HANDLER( xxmissio_videoram_w )
{
	int offs = offset & 0x7e0;
	int x = (offset + (xxmissio_xscroll >> 3) ) & 0x1f;
	offs |= x;

	videoram[offs] = data;
	dirtybuffer[offs & 0x3ff] = 1;
}
READ8_HANDLER( xxmissio_videoram_r )
{
	int offs = offset & 0x7e0;
	int x = (offset + (xxmissio_xscroll >> 3) ) & 0x1f;
	offs |= x;

	return videoram[offs];
}

WRITE8_HANDLER( xxmissio_paletteram_w )
{
	paletteram_BBGGRRII_w(offset,data);

	if (offset >= 0x200)
		xxmissio_bg_redraw = 1;
}

/****************************************************************************/

VIDEO_UPDATE( xxmissio )
{
	int offs;
	int chr,col;
	int x,y,px,py,fx,fy,sx,sy;

	int size = videoram_size/2;

	if (xxmissio_bg_redraw==1)
		memset(dirtybuffer,1,size);

/* draw BG layer */

	for (y=0; y<32; y++)
	{
		for (x=0; x<32; x++)
		{
			offs = y*0x20 + x;

			if (flipscreen!=0)
				offs = (size-1)-offs;

			if (dirtybuffer[offs] != 0)
			{
				dirtybuffer[offs]=0;

				px = x*16;
				py = y*8;

				chr = videoram[ offs ] ;
				col = videoram[ offs + size];
				chr = chr + ((col & 0xc0) << 2 );
				col = col & 0x0f;

				drawgfx(tmpbitmap,machine->gfx[2],
					chr,
					col,
					flipscreen,flipscreen,
					px,py,
					&machine->screen[0].visarea,TRANSPARENCY_NONE,0);
			}
		}
	}

	if (flipscreen == 0)
	{
		sx = -xxmissio_xscroll*2+12;
		sy = -xxmissio_yscroll;
	}
	else
	{
		sx = xxmissio_xscroll*2+2;
		sy = xxmissio_yscroll;
	}

	copyscrollbitmap(bitmap,tmpbitmap,1,&sx,1,&sy,&machine->screen[0].visarea,TRANSPARENCY_NONE,0);
	xxmissio_bg_redraw = 0;

/* draw sprites */

	for (offs=0; offs<spriteram_size; offs +=32)
	{
		chr = spriteram[offs];
		col = spriteram[offs+3];

		fx = ((col & 0x10) >> 4) ^ flipscreen;
		fy = ((col & 0x20) >> 5) ^ flipscreen;

		x = spriteram[offs+1]*2;
		y = spriteram[offs+2];

		chr = chr + ((col & 0x40) << 2);
		col = col & 0x07;

		if (flipscreen==0)
		{
			px = x-8;
			py = y;
		}
		else
		{
			px = 480-x-8;
			py = 240-y;
		}

		px &= 0x1ff;

		drawgfx(bitmap,machine->gfx[1],
			chr,
			col,
			fx,fy,
			px,py,
			&machine->screen[0].visarea,TRANSPARENCY_PEN,0);
		if (px>0x1e0)
			drawgfx(bitmap,machine->gfx[1],
				chr,
				col,
				fx,fy,
				px-0x200,py,
				&machine->screen[0].visarea,TRANSPARENCY_PEN,0);

	}


/* draw FG layer */

	for (y=4; y<28; y++)
	{
		for (x=0; x<32; x++)
		{
			offs = y*32+x;
			chr = xxmissio_fgram[offs];
			col = xxmissio_fgram[offs + 0x400] & 0x07;

			if (flipscreen==0)
			{
				px = 16*x;
				py = 8*y;
			}
			else
			{
				px = 496-16*x;
				py = 248-8*y;
			}

			drawgfx(bitmap,machine->gfx[0],
				chr,
				col,
				flipscreen,flipscreen,
				px,py,
				&machine->screen[0].visarea,TRANSPARENCY_PEN,0);
		}
	}

	return 0;
}
