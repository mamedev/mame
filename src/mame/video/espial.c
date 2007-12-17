/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/espial.h"


UINT8 *espial_videoram;
UINT8 *espial_colorram;
UINT8 *espial_attributeram;
UINT8 *espial_scrollram;
UINT8 *espial_spriteram_1;
UINT8 *espial_spriteram_2;
UINT8 *espial_spriteram_3;

static int flipscreen;
static tilemap *bg_tilemap;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Espial has two 256x4 palette PROMs.

  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
  bit 0 -- 470 ohm resistor  -- GREEN
  bit 3 -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( espial )
{
	int i;


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i + machine->drv->total_colors] >> 0) & 0x01;
		bit2 = (color_prom[i + machine->drv->total_colors] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i + machine->drv->total_colors] >> 2) & 0x01;
		bit2 = (color_prom[i + machine->drv->total_colors] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT8 code = espial_videoram[tile_index];
	UINT8 col = espial_colorram[tile_index];
	UINT8 attr = espial_attributeram[tile_index];
	SET_TILE_INFO(0,
				  code | ((attr & 0x03) << 8),
				  col & 0x3f,
				  TILE_FLIPYX(attr >> 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( espial )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_scroll_cols(bg_tilemap, 32);
}

VIDEO_START( netwars )
{
	/* Net Wars has a tile map that's twice as big as Espial's */
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,64);

	tilemap_set_scroll_cols(bg_tilemap, 32);
	tilemap_set_scrolldy(bg_tilemap, 0, 0x100);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( espial_videoram_w )
{
	espial_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( espial_colorram_w )
{
	espial_colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( espial_attributeram_w )
{
	espial_attributeram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( espial_scrollram_w )
{
	espial_scrollram[offset] = data;
	tilemap_set_scrolly(bg_tilemap, offset, data);
}


WRITE8_HANDLER( espial_flipscreen_w )
{
	flipscreen = data;

	tilemap_set_flip(0, flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;


	/* Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */
	for (offs = 0;offs < 16;offs++)
	{
		int sx,sy,code,color,flipx,flipy;


		sx = espial_spriteram_1[offs + 16];
		sy = espial_spriteram_2[offs];
		code = espial_spriteram_1[offs] >> 1;
		color = espial_spriteram_2[offs + 16];
		flipx = espial_spriteram_3[offs] & 0x04;
		flipy = espial_spriteram_3[offs] & 0x08;

		if (flipscreen)
		{
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sy = 240 - sy;
		}

		if (espial_spriteram_1[offs] & 1)	/* double height */
		{
			if (flipscreen)
			{
				drawgfx(bitmap,machine->gfx[1],
						code,color,
						flipx,flipy,
						sx,sy + 16,
						cliprect,TRANSPARENCY_PEN,0);
				drawgfx(bitmap,machine->gfx[1],
						code + 1,
						color,
						flipx,flipy,
						sx,sy,
						cliprect,TRANSPARENCY_PEN,0);
			}
			else
			{
				drawgfx(bitmap,machine->gfx[1],
						code,color,
						flipx,flipy,
						sx,sy - 16,
						cliprect,TRANSPARENCY_PEN,0);
				drawgfx(bitmap,machine->gfx[1],
						code + 1,color,
						flipx,flipy,
						sx,sy,
						cliprect,TRANSPARENCY_PEN,0);
			}
		}
		else
		{
			drawgfx(bitmap,machine->gfx[1],
					code,color,
					flipx,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,0);
		}
	}
}


VIDEO_UPDATE( espial )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
