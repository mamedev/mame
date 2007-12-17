#include "driver.h"


UINT8 *troangel_scroll;

static int flipscreen;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Tropical Angel has two 256x4 character palette PROMs, one 32x8 sprite
  palette PROM, and one 256x4 sprite color lookup table PROM.

  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably something like this; note that RED and BLUE
  are swapped wrt the usual configuration.

  bit 7 -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
  bit 0 -- 1  kohm resistor  -- BLUE

***************************************************************************/
PALETTE_INIT( troangel )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	/* character palette */
	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = 0;
		bit1 = (color_prom[256] >> 2) & 0x01;
		bit2 = (color_prom[256] >> 3) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[256] >> 0) & 0x01;
		bit2 = (color_prom[256] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		COLOR(0,i) = i;
		color_prom++;
	}

	color_prom += 256;
	/* color_prom now points to the beginning of the sprite palette */


	/* sprite palette */
	for (i = 0;i < 16;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i+256,MAKE_RGB(r,g,b));
		color_prom++;
	}

	color_prom += 16;
	/* color_prom now points to the beginning of the sprite lookup table */


	/* sprite lookup table */
	for (i = 0;i < TOTAL_COLORS(1);i++)
	{
		COLOR(1,i) = 256 + (~*color_prom & 0x0f);
		color_prom++;
	}
}



WRITE8_HANDLER( troangel_flipscreen_w )
{
	/* screen flip is handled both by software and hardware */
	data ^= ~readinputport(4) & 1;

	if (flipscreen != (data & 1))
	{
		flipscreen = data & 1;
		memset(dirtybuffer,1,videoram_size);
	}

	coin_counter_w(0,data & 0x02);
	coin_counter_w(1,data & 0x20);
}




static void draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;
	const gfx_element *gfx = machine->gfx[0];

	for (offs = videoram_size - 2;offs >= 0;offs -= 2)
	{
		if (dirtybuffer[offs] || dirtybuffer[offs+1])
		{
			int sx,sy,code,attr,flipx,flipy;


			dirtybuffer[offs] = dirtybuffer[offs+1] = 0;

			sx = (offs/2) % 32;
			sy = (offs/2) / 32;

			attr = videoram[offs+0];
			code = videoram[offs+1] | ((attr & 0xc0) << 2);
			flipx = attr & 0x20;
			flipy = (attr & 0x10) >> 4;

			if (flipscreen)
			{
				sx = 31 - sx;
				sy = 31 - sy;
				flipx = !flipx;
			}

			drawgfx(tmpbitmap,gfx,
				code,
				attr & 0x0f,
				flipx,flipy ^ flipscreen,
				8*sx,8*sy,
				0,TRANSPARENCY_NONE,0);
		}
	}

	{
		int xscroll[256];

		if (flipscreen)
		{
			/* fixed */
			for (offs = 0;offs < 64;offs++) xscroll[255-offs] = 0;

			/* scroll (wraps around) */
			for (offs = 64;offs < 128;offs++) xscroll[255-offs] = troangel_scroll[64];

			/* linescroll (no wrap) */
			for (offs = 128;offs < 256;offs++) xscroll[255-offs] = troangel_scroll[offs];
		}
		else
		{
			/* fixed */
			for (offs = 0;offs < 64;offs++) xscroll[offs] = 0;

			/* scroll (wraps around) */
			for (offs = 64;offs < 128;offs++) xscroll[offs] = -troangel_scroll[64];

			/* linescroll (no wrap) */
			for (offs = 128;offs < 256;offs++) xscroll[offs] = -troangel_scroll[offs];
		}

		copyscrollbitmap(bitmap,tmpbitmap,256,xscroll,0,0,cliprect,TRANSPARENCY_NONE,0);
	}
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;


	for (offs = spriteram_size-4;offs >= 0;offs -= 4)
	{
		UINT8 attributes = spriteram[offs+1];
		int sx = spriteram[offs+3];
		int sy = ((224-spriteram[offs+0]-32)&0xff)+32;
		int code = spriteram[offs+2];
		int color = attributes&0x1f;
		int flipy = attributes&0x80;
		int flipx = attributes&0x40;

		int tile_number = code & 0x3f;

		int bank = 0;
		if( code&0x80 ) bank += 1;
		if( attributes&0x20 ) bank += 2;

		if (flipscreen)
		{
			sx = 240 - sx;
			sy = 224 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[1+bank],
			tile_number,
			color,
			flipx,flipy,
			sx,sy,
			cliprect,TRANSPARENCY_PEN,0);
	}
}



VIDEO_UPDATE( troangel )
{
	draw_background(machine, bitmap, cliprect);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
