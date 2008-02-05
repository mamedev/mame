/***************************************************************************

    Pooyan

***************************************************************************/

#include "driver.h"
#include "pooyan.h"

static tilemap *bg_tilemap;



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Pooyan has one 32x8 palette PROM and two 256x4 lookup table PROMs
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

PALETTE_INIT( pooyan )
{
	rgb_t palette[32];
	int i;

	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette[i] = MAKE_RGB(r,g,b);
		color_prom++;
	}

	/* color_prom now points to the beginning of the char lookup table */

	/* sprites */
	for (i = 0;i < 16*16;i++)
		palette_set_color(machine, 16*16+i, palette[*color_prom++ & 0x0f]);

	/* characters */
	for (i = 0;i < 16*16;i++)
		palette_set_color(machine, i, palette[(*color_prom++ & 0x0f) + 0x10]);
}



/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index];
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX(attr >> 6);

	SET_TILE_INFO(0, code, color, flags);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( pooyan )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,  8,8, 32,32);
}



/*************************************
 *
 *  Memory write handlers
 *
 *************************************/

WRITE8_HANDLER( pooyan_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( pooyan_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( pooyan_flipscreen_w )
{
	flip_screen_set(~data & 0x01);
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0x10;offs < 0x40;offs += 2)
	{
		int sx = spriteram[offs];
		int sy = 240 - spriteram_2[offs + 1];

		int code = spriteram[offs + 1];
		int color = spriteram_2[offs] & 0x0f;
		int flipx = ~spriteram_2[offs] & 0x40;
		int flipy = spriteram_2[offs] & 0x80;

		/* Sprite flipscreen is supported by software */
		drawgfx(bitmap,machine->gfx[1],
			code,
			color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( pooyan )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
