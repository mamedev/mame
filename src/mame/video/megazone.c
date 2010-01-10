/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"

UINT8 *megazone_scrollx;
UINT8 *megazone_scrolly;
static int flipscreen;

UINT8 *megazone_videoram;
UINT8 *megazone_colorram;
UINT8 *megazone_videoram2;
UINT8 *megazone_colorram2;
size_t megazone_videoram_size;
size_t megazone_videoram2_size;

/***************************************************************************
Based on driver from MAME 0.55
Changes by Martin M. (pfloyd@gmx.net) 14.10.2001:

 - Added support for screen flip in cocktail mode (tricky!) */


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Megazone has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( megazone )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances_rg[0], rweights, 1000, 0,
			3, &resistances_rg[0], gweights, 1000, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* sprites */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* characters */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE8_HANDLER( megazone_flipscreen_w )
{
	flipscreen = data & 1;
}

VIDEO_START( megazone )
{
	machine->generic.tmpbitmap = auto_bitmap_alloc(machine,256,256,video_screen_get_format(machine->primary_screen));
}


VIDEO_UPDATE( megazone )
{
	int offs;
	int x,y;

	/* for every character in the Video RAM */
	for (offs = megazone_videoram_size - 1;offs >= 0;offs--)
	{
		int sx,sy,flipx,flipy;

		sx = offs % 32;
		sy = offs / 32;
		flipx = megazone_colorram[offs] & (1<<6);
		flipy = megazone_colorram[offs] & (1<<5);
		if (flipscreen)
		{
			sx = 31 - sx;
			sy = 31 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_opaque(screen->machine->generic.tmpbitmap,0,screen->machine->gfx[1],
				((int)megazone_videoram[offs]) + ((megazone_colorram[offs] & (1<<7) ? 256 : 0) ),
				(megazone_colorram[offs] & 0x0f) + 0x10,
				flipx,flipy,
				8*sx,8*sy);
	}

	/* copy the temporary bitmap to the screen */
	{
		int scrollx;
		int scrolly;

		if (flipscreen)
		{
			scrollx = *megazone_scrolly;
			scrolly = *megazone_scrollx;
		}
		else
		{
			scrollx = -*megazone_scrolly+4*8; // leave space for credit&score overlay
			scrolly = -*megazone_scrollx;
		}


		copyscrollbitmap(bitmap,screen->machine->generic.tmpbitmap,1,&scrollx,1,&scrolly,cliprect);
	}


	/* Draw the sprites. */
	{
		UINT8 *spriteram = screen->machine->generic.spriteram.u8;
		for (offs = screen->machine->generic.spriteram_size-4; offs >= 0;offs -= 4)
		{
			int sx = spriteram[offs + 3];
			int sy = 255-((spriteram[offs + 1]+16)&0xff);
			int color =  spriteram[offs + 0] & 0x0f;
			int flipx = ~spriteram[offs + 0] & 0x40;
			int flipy =  spriteram[offs + 0] & 0x80;

			if (flipscreen)
			{
				sx = sx - 11;
				sy = sy + 2;
			}
			else
				sx = sx + 32;

			drawgfx_transmask(bitmap,cliprect,screen->machine->gfx[0],
					spriteram[offs + 2],
					color,
					flipx,flipy,
					sx,sy,
					colortable_get_transpen_mask(screen->machine->colortable, screen->machine->gfx[0], color, 0));
		}
	}

	for (y = 0; y < 32;y++)
	{
		offs = y*32;
		for (x = 0; x < 6; x++)
		{
			int sx,sy,flipx,flipy;

			sx = x;
			sy = y;

			flipx = megazone_colorram2[offs] & (1<<6);
			flipy = megazone_colorram2[offs] & (1<<5);

			if (flipscreen)
			{
				sx = 35 - sx;
				sy = 31 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}




			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[1],
					((int)megazone_videoram2[offs]) + ((megazone_colorram2[offs] & (1<<7) ? 256 : 0) ),
					(megazone_colorram2[offs] & 0x0f) + 0x10,
					flipx,flipy,
					8*sx,8*sy);
			offs++;
		}
	}
	return 0;
}
