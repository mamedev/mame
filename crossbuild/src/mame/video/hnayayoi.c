/***************************************************************************

First version of the Dynax blitter.

Can handle up to 8 256x256 bitmaps; in the games supported, every pair of
bitmaps is interleaved horizontally to form 4 higher res 512x256 layer.

The blitter reads compressed data from ROM and copies it to the bitmap RAM.

***************************************************************************/

#include "driver.h"


static UINT8 *pixmap[8];
static int palbank;
static int total_pixmaps;


static void common_vh_start(int num_pixmaps)
{
	int i;

	total_pixmaps = num_pixmaps;

	for (i = 0;i < 8;i++)
	{
		if (i < total_pixmaps)
		{
			pixmap[i] = auto_malloc(256*256);
		}
		else
			pixmap[i] = NULL;
	}
}

VIDEO_START( hnayayoi )
{
	common_vh_start(4);	/* 4 bitmaps -> 2 layers */
}

VIDEO_START( untoucha )
{
	common_vh_start(8);	/* 8 bitmaps -> 4 layers */
}



/***************************************************************************

Blitter support

three parameters:
blit_layer: mask of the bitmaps to write to (can write to multiple bitmaps
            at the same time)
blit_dest:  position in the destination bitmap where to start blitting
blit_src:   address of source data in the gfx ROM

additional parameters specify the palette base, but this is handled while rendering
the screen, not during blitting (games change the palette base without redrawing
the screen).

It is not known whether the palette base control registers are part of the blitter
hardware or latched somewhere else. Since they are mapped in memory immediately
before the bitter parameters, they probably are part of the blitter, but I'm
handling them separately anyway.


The format of the blitter data stored in ROM is very simple:

7654 ----   Pen to draw with
---- 3210   Command

Commands:

0       Stop
1-b     Draw 1-b pixels along X.
c       Followed by 1 byte (N): draw N pixels along X.
d       Followed by 2 bytes (X,N): move on the line to pixel (start+X), draw N pixels
        along X.
e       Followed by 1 byte (N): set blit_layer = N. Used to draw interleaved graphics
        with a single blitter run.
f       Move to next line.

At the end of the blit, blit_src is left pointing to the next data in the gfx ROM.
This is used to draw interleaved graphics with two blitter runs without having to set
up blit_src for the second call.

***************************************************************************/

static UINT8 blit_layer;
static UINT16 blit_dest;
static UINT32 blit_src;

WRITE8_HANDLER( dynax_blitter_rev1_param_w )
{
	switch (offset)
	{
		case 0: blit_dest = (blit_dest & 0xff00) | (data << 0); break;
		case 1: blit_dest = (blit_dest & 0x00ff) | (data << 8); break;
		case 2: blit_layer = data; break;
		case 3: blit_src = (blit_src & 0xffff00) | (data << 0); break;
		case 4: blit_src = (blit_src & 0xff00ff) | (data << 8); break;
		case 5: blit_src = (blit_src & 0x00ffff) | (data <<16); break;
	}
}

static void copy_pixel(int x,int y,int pen)
{
	if (x >= 0 && x <= 255 && y >= 0 && y <= 255)
	{
		int i;

		for (i = 0;i < 8;i++)
		{
			if ((~blit_layer & (1 << i)) && (pixmap[i]))
				pixmap[i][256*y+x] = pen;
		}
	}
}

WRITE8_HANDLER( dynax_blitter_rev1_start_w )
{
	UINT8 *rom = memory_region(REGION_GFX1);
	int romlen = memory_region_length(REGION_GFX1);
	int sx = blit_dest & 0xff;
	int sy = blit_dest >> 8;
	int x,y;

	x = sx;
	y = sy;
	while (blit_src < romlen)
	{
		int cmd = rom[blit_src] & 0x0f;
		int pen = rom[blit_src] >> 4;

		blit_src++;

		switch (cmd)
		{
			case 0xf:
				y++;
				x = sx;
				break;

			case 0xe:
				if (blit_src >= romlen)
				{
					popmessage("GFXROM OVER %06x",blit_src);
					return;
				}
				x = sx;
				blit_layer = rom[blit_src++];
				break;

			case 0xd:
				if (blit_src >= romlen)
				{
					popmessage("GFXROM OVER %06x",blit_src);
					return;
				}
				x = sx + rom[blit_src++];
				/* fall through into next case */

			case 0xc:
				if (blit_src >= romlen)
				{
					popmessage("GFXROM OVER %06x",blit_src);
					return;
				}
				cmd = rom[blit_src++];
				/* fall through into next case */

			case 0xb:
			case 0xa:
			case 0x9:
			case 0x8:
			case 0x7:
			case 0x6:
			case 0x5:
			case 0x4:
			case 0x3:
			case 0x2:
			case 0x1:
				while (cmd--)
					copy_pixel(x++,y,pen);
				break;

			case 0x0:
				return;
		}
	}

	popmessage("GFXROM OVER %06x",blit_src);
}

WRITE8_HANDLER( dynax_blitter_rev1_clear_w )
{
	int pen = data >> 4;
	int i;

	for (i = 0;i < 8;i++)
	{
		if ((~blit_layer & (1 << i)) && (pixmap[i]))
			memset(pixmap[i] + blit_dest, pen, 0x10000 - blit_dest);
	}
}


WRITE8_HANDLER( hnayayoi_palbank_w )
{
	offset *= 8;
	palbank = (palbank & (0xff00 >> offset)) | (data << offset);
}


static void draw_layer_interleaved(mame_bitmap *bitmap, const rectangle *cliprect,
		int left_pixmap, int right_pixmap, int palbase, int transp)
{
	int county,countx,pen,offs;
	UINT8 *src1 = pixmap[left_pixmap];
	UINT8 *src2 = pixmap[right_pixmap];
	UINT16 *dstbase = (UINT16 *)bitmap->base;

	palbase *= 16;
	offs = 0;

	for (county = 255; county >= 0; county--, dstbase += bitmap->rowpixels)
	{
		UINT16 *dst = dstbase;

		if (transp)
		{
			for (countx = 255; countx >= 0; countx--, dst += 2)
			{
				pen = *(src1++);
				if (pen) *dst     = palbase + pen;
				pen = *(src2++);
				if (pen) *(dst+1) = palbase + pen;
			}
		}
		else
		{
			for (countx = 255; countx >= 0; countx--, dst += 2)
			{
				*dst     = palbase + *(src1++);
				*(dst+1) = palbase + *(src2++);
			}
		}
	}
}


VIDEO_UPDATE( hnayayoi )
{
	int col0 = (palbank >>  0) & 0x0f;
	int col1 = (palbank >>  4) & 0x0f;
	int col2 = (palbank >>  8) & 0x0f;
	int col3 = (palbank >> 12) & 0x0f;

	if (total_pixmaps == 4)
	{
		draw_layer_interleaved(bitmap,cliprect,3,2,col1,0);
		draw_layer_interleaved(bitmap,cliprect,1,0,col0,1);
	}
	else	/* total_pixmaps == 8 */
	{
		draw_layer_interleaved(bitmap,cliprect,7,6,col3,0);
		draw_layer_interleaved(bitmap,cliprect,5,4,col2,1);
		draw_layer_interleaved(bitmap,cliprect,3,2,col1,1);
		draw_layer_interleaved(bitmap,cliprect,1,0,col0,1);
	}
	return 0;
}
