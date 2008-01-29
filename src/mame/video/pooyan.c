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
	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + 8 * (attr & 0x20);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( pooyan )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0x10;offs < 0x40;offs += 2)
	{
		/* Sprite flipscreen is supported by software */
		drawgfx(bitmap,machine->gfx[1],
			spriteram[offs + 1],
			spriteram_2[offs] & 0x0f,
			spriteram_2[offs] & 0x40, ~spriteram_2[offs] & 0x80,
			240-spriteram[offs], spriteram_2[offs + 1],
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( pooyan )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
