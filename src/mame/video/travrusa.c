/***************************************************************************

  video.c

  Traverse USA

L Taylor
J Clegg

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *travrusa_videoram;

static tilemap *bg_tilemap;



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Traverse USA has one 256x8 character palette PROM (some versions have two
  256x4), one 32x8 sprite palette PROM, and one 256x4 sprite color lookup
  table PROM.

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

PALETTE_INIT( travrusa )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	/* character palette */
	for (i = 0;i < 128;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	color_prom += 256;

	/* sprite palette */
	for (i = 0;i < 16;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i+128,MAKE_RGB(r,g,b));
	}

	color_prom += 32;

	/* color_prom now points to the beginning of the lookup table */

	/* sprite lookup table */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = (color_prom[i] & 0x0f) + 128;
}

PALETTE_INIT( shtrider )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	/* character palette */
	for (i = 0;i < 128;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 2) & 0x01;
		bit2 = (color_prom[i] >> 3) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i+256] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 0) & 0x01;
		bit2 = (color_prom[i] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i+256] >> 0) & 0x01;
		bit1 = (color_prom[i+256] >> 1) & 0x01;
		bit2 = (color_prom[i+256] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	color_prom += 512;

	/* color_prom now points to the beginning of the sprite palette */

	/* sprite palette */
	for (i = 0;i < 16;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i+128,MAKE_RGB(r,g,b));
	}

	color_prom += 32;
	/* color_prom now points to the beginning of the sprite lookup table */

	/* sprite lookup table */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = (color_prom[i] & 0x0f) + 128;
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT8 attr = travrusa_videoram[2*tile_index+1];
	int flags = TILE_FLIPXY((attr & 0x30) >> 4);

	tileinfo->group = ((attr & 0x0f) == 0x0f) ? 1 : 0;	/* tunnels */

	SET_TILE_INFO(
			0,
			travrusa_videoram[2*tile_index] + ((attr & 0xc0) << 2),
			attr & 0x0f,
			flags);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( travrusa )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,64,32);

	tilemap_set_transmask(bg_tilemap,0,0xff,0x00); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(bg_tilemap,1,0x3f,0xc0); /* split type 1 has pens 6 and 7 opaque - tunnels */

	tilemap_set_scroll_rows(bg_tilemap,4);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( travrusa_videoram_w )
{
	travrusa_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset/2);
}


static int scrollx[2];

static void set_scroll(void)
{
	int i;

	for (i = 0;i <= 2;i++)
		tilemap_set_scrollx(bg_tilemap,i,scrollx[0] + 256 * scrollx[1]);
	tilemap_set_scrollx(bg_tilemap,3,0);
}

WRITE8_HANDLER( travrusa_scroll_x_low_w )
{
	scrollx[0] = data;
	set_scroll();
}

WRITE8_HANDLER( travrusa_scroll_x_high_w )
{
	scrollx[1] = data;
	set_scroll();
}


WRITE8_HANDLER( travrusa_flipscreen_w )
{
	/* screen flip is handled both by software and hardware */
	data ^= ~readinputport(4) & 1;

	flip_screen_set(data & 1);

	coin_counter_w(0,data & 0x02);
	coin_counter_w(1,data & 0x20);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;
	static rectangle spritevisiblearea =
	{
		1*8, 31*8-1,
		0*8, 24*8-1
	};
	static rectangle spritevisibleareaflip =
	{
		1*8, 31*8-1,
		8*8, 32*8-1
	};
	rectangle clip = *cliprect;
	if (flip_screen)
		sect_rect(&clip, &spritevisibleareaflip);
	else
		sect_rect(&clip, &spritevisiblearea);


	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx = ((spriteram[offs + 3] + 8) & 0xff) - 8;
		int sy = 240 - spriteram[offs];
		int code = spriteram[offs + 2];
		int attr = spriteram[offs + 1];
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[1],
				code,
				attr & 0x0f,
				flipx,flipy,
				sx,sy,
				&clip,TRANSPARENCY_PEN,0);
	}
}


VIDEO_UPDATE( travrusa )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER1,0);
	draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_LAYER0,0);
	return 0;
}
