/*******************************************************************************

     Mouser - Video Hardware:

     Character map with scrollable rows, 1024 possible characters.
        - index = byte from videoram + 2 bits from colorram)
        - (if row is scrolled, videoram is offset, colorram is not)
        - 16 4-color combinations for each char, from colorram

     15 Sprites controlled by 4-byte records
        - 16 4-color combinations
        - 2 banks of 64 sprite characters each

*******************************************************************************/

#include "driver.h"

PALETTE_INIT( mouser )
{
	int i;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = BIT(*color_prom,0);
		bit1 = BIT(*color_prom,1);
		bit2 = BIT(*color_prom,2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = BIT(*color_prom,3);
		bit1 = BIT(*color_prom,4);
		bit2 = BIT(*color_prom,5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = BIT(*color_prom,6);
		bit1 = BIT(*color_prom,7);
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( mouser_flip_screen_x_w )
{
	flip_screen_x_set(~data & 1);
}

WRITE8_HANDLER( mouser_flip_screen_y_w )
{
	flip_screen_y_set(~data & 1);
}

WRITE8_HANDLER( mouser_spriteram_w )
{
	/* Mark the entire row as dirty if row scrollram is written */
	/* Only used by the MOUSER logo */

	int i;

	if (offset < 32)
	{
		for(i=0;i<32;i++)
		dirtybuffer[offset+i*32] = 1;
	}
	spriteram_w(offset, data);
}

WRITE8_HANDLER( mouser_colorram_w )
{
	dirtybuffer[offset] = 1;
	colorram_w(offset, data);
}

VIDEO_UPDATE( mouser )
{
	int offs;
	int sx,sy;
	int flipx,flipy;

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		int scrolled_y_position;
		int color_offs;

		if (dirtybuffer[offs])
		{
			dirtybuffer[offs] = 0;

			sx = offs % 32;
			sy = offs / 32;

			if (flip_screen_x)
			{
				sx = 31 - sx;
			}

			if (flip_screen_y)
			{
				sy = 31 - sy;
			}

			/* This bit of spriteram appears to be for row scrolling */
			/* Note: this is dependant on flipping in y */
			scrolled_y_position = (256 + 8*sy - spriteram[offs%32])%256;
			/* I think we still need to fetch the colorram bits to from the ram underneath, which is not scrolled */
			/* Ideally we would merge these on a pixel-by-pixel basis, but it's ok to do this char-by-char, */
			/* Since it's only for the MOUSER logo and it looks fine */
			/* Note: this is _not_ dependant on flipping */
			color_offs = offs%32 + ((256 + 8*(offs/32) - spriteram[offs%32])%256)/8*32;

			drawgfx(tmpbitmap,machine->gfx[0],
					videoram[offs] | (colorram[color_offs]>>5)*256 | ((colorram[color_offs]>>4)&1)*512,
					colorram[color_offs]%16,
					flip_screen_x,flip_screen_y,
					8*sx,scrolled_y_position,
					0,TRANSPARENCY_NONE,0);
		}
	}

	copyscrollbitmap(bitmap,tmpbitmap,0,0,0,0,&machine->screen[0].visarea,TRANSPARENCY_NONE,0);

	/* There seem to be two sets of sprites, each decoded identically */

	/* This is the first set of 7 sprites */
	for(offs = 0x0084; offs < 0x00A0; offs += 4)
	{
		sx = spriteram[offs+3];
		sy = 0xef-spriteram[offs+2];

		flipx = (spriteram[offs]&0x40)>>6;
		flipy = (spriteram[offs]&0x80)>>7;

		if (flip_screen_x)
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y)
		{
			flipy = !flipy;
			sy = 238 - sy;
		}

		if ((spriteram[offs+1]&0x10)>>4)
			drawgfx(bitmap,machine->gfx[1+((spriteram[offs+1]&0x20)>>5)],
					spriteram[offs]&0x3f,
					spriteram[offs+1]%16,
					flipx,flipy,
					sx,sy,
					&machine->screen[0].visarea,TRANSPARENCY_PEN,0);
	}

	/* This is the second set of 8 sprites */
	for(offs = 0x00C4; offs < 0x00E4; offs += 4)
	{
		sx = spriteram[offs+3];
		sy = 0xef-spriteram[offs+2];

		flipx = (spriteram[offs]&0x40)>>6;
		flipy = (spriteram[offs]&0x80)>>7;

		if (flip_screen_x)
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y)
		{
			flipy = !flipy;
			sy = 238 - sy;
		}

		if ((spriteram[offs+1]&0x10)>>4)
			drawgfx(bitmap,machine->gfx[1+((spriteram[offs+1]&0x20)>>5)],
					spriteram[offs]&0x3f,
					spriteram[offs+1]%16,
					flipx,flipy,
					sx,sy,
					&machine->screen[0].visarea,TRANSPARENCY_PEN,0);
	}

	return 0;
}
