/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"



UINT8 *zaccaria_videoram,*zaccaria_attributesram;

static tilemap *bg_tilemap;

static rectangle spritevisiblearea =
{
	2*8+1, 29*8-1,
	2*8, 30*8-1
};
static rectangle spritevisiblearea_flipx =
{
	3*8+1, 30*8-1,
	2*8, 30*8-1
};



/***************************************************************************

  Convert the color PROMs into a more useable format.


Here's the hookup from the proms (82s131) to the r-g-b-outputs

     Prom 9F        74LS374
    -----------   ____________
       12         |  3   2   |---680 ohm----| blue out
       11         |  4   5   |---1k ohm-----| (+ 470 ohm pulldown)
       10         |  7   6   |---820 ohm-------|
        9         |  8   9   |---1k ohm--------| green out
     Prom 9G      |          |                 | (+ 390 ohm pulldown)
       12         |  13  12  |---1.2k ohm------|
       11         |  14  15  |---820 ohm----------|
       10         |  17  16  |---1k ohm-----------| red out
        9         |  18  19  |---1.2k ohm---------| (+ 390 ohm pulldown)
                  |__________|


***************************************************************************/
PALETTE_INIT( zaccaria )
{
	int i,j,k;
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/*
          TODO: I'm not sure, but I think that pen 0 must always be black, otherwise
          there's some junk brown background in Jack Rabbit.
          From the schematics it seems that the background color can be changed, but
          I'm not sure where it would be taken from; I think the high bits of
          attributesram, but they are always 0 in these games so they would turn out
          black anyway.
         */
		if (((i % 64) / 8) == 0)
		{
			palette_set_color(machine,i,MAKE_RGB(0,0,0));
		}
		else
		{
			/* red component */
			bit0 = (color_prom[0] >> 3) & 0x01;
			bit1 = (color_prom[0] >> 2) & 0x01;
			bit2 = (color_prom[0] >> 1) & 0x01;
			r = 0x46 * bit0 + 0x53 * bit1 + 0x66 * bit2;
			/* green component */
			bit0 = (color_prom[0] >> 0) & 0x01;
			bit1 = (color_prom[machine->drv->total_colors] >> 3) & 0x01;
			bit2 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
			g = 0x46 * bit0 + 0x53 * bit1 + 0x66 * bit2;
			/* blue component */
			bit0 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
			bit1 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
			b = 0x66 * bit0 + 0x96 * bit1;
			palette_set_color(machine,i,MAKE_RGB(r,g,b));
		}

		color_prom++;
	}

	/* There are 512 unique colors, which seem to be organized in 8 blocks */
	/* of 64. In each block, colors are not in the usual sequential order */
	/* but in interleaved order, like Phoenix. Additionally, colors for */
	/* background and sprites are interleaved. */
	for (i = 0;i < 8;i++)
	{
		for (j = 0;j < 4;j++)
		{
			for (k = 0;k < 8;k++)
			{
				/* swap j and k to make the colors sequential */
				COLOR(0,32 * i + 8 * j + k) = 64 * i + 8 * k + 2*j;
			}
		}
	}
	for (i = 0;i < 8;i++)
	{
		for (j = 0;j < 4;j++)
		{
			for (k = 0;k < 8;k++)
			{
				/* swap j and k to make the colors sequential */
				COLOR(1,32 * i + 8 * j + k) = 64 * i + 8 * k + 2*j+1;
			}
		}
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT8 attr = zaccaria_videoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			zaccaria_videoram[tile_index] + ((attr & 0x03) << 8),
			((attr & 0x0c) >> 2) + ((zaccaria_attributesram[2 * (tile_index % 32) + 1] & 0x07) << 2),
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( zaccaria )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_scroll_cols(bg_tilemap,32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( zaccaria_videoram_w )
{
	zaccaria_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( zaccaria_attributes_w )
{
	if (offset & 1)
	{
		if (zaccaria_attributesram[offset] != data)
		{
			int i;

			for (i = offset / 2;i < 0x400;i += 32)
				tilemap_mark_tile_dirty(bg_tilemap,i);
		}
	}
	else
		tilemap_set_scrolly(bg_tilemap,offset / 2,data);

	zaccaria_attributesram[offset] = data;
}

WRITE8_HANDLER( zaccaria_flip_screen_x_w )
{
	flip_screen_x_set(data & 1);
}

WRITE8_HANDLER( zaccaria_flip_screen_y_w )
{
	flip_screen_y_set(data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;
	rectangle clip = *cliprect;

	if (flip_screen_x)
		sect_rect(&clip, &spritevisiblearea_flipx);
	else
		sect_rect(&clip, &spritevisiblearea);

	/*
      TODO: sprites have 32 color codes, but we are using only 8. In Jack
      Rabbit the extra codes are all duplicates, but there is a quadruple
      of codes in Money Money which contains two different combinations. That
      color code seems to be used only by crocodiles, so the one we are picking
      seems the correct one (otherwise they would be red).
    */

	/*
      TODO: sprite placement is not perfect, I made the Jack Rabbit mouth
      animation correct but this moves one pixel to the left the sprite
      which masks the holes when you fall in them. The hardware is probably
      similar to Amidar, but the code in the Amidar driver is not good either.
    */
	for (offs = 0;offs < spriteram_2_size;offs += 4)
	{
		int sx = spriteram_2[offs + 3] + 1;
		int sy = 242 - spriteram_2[offs];
		int flipx = spriteram_2[offs + 2] & 0x40;
		int flipy = spriteram_2[offs + 2] & 0x80;

		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[1],
				(spriteram_2[offs + 2] & 0x3f) + (spriteram_2[offs + 1] & 0xc0),
				4 * (spriteram_2[offs + 1] & 0x07),
				flipx,flipy,
				sx,sy,
				&clip,TRANSPARENCY_PEN,0);
	}

	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		int sx = spriteram[offs + 3] + 1;
		int sy = 242 - spriteram[offs];
		int flipx = spriteram[offs + 1] & 0x40;
		int flipy = spriteram[offs + 1] & 0x80;

		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[1],
				(spriteram[offs + 1] & 0x3f) + (spriteram[offs + 2] & 0xc0),
				4 * (spriteram[offs + 2] & 0x07),
				flipx,flipy,
				sx,sy,
				&clip,TRANSPARENCY_PEN,0);
	}
}


VIDEO_UPDATE( zaccaria )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_sprites(machine, bitmap,cliprect);
	return 0;
}
