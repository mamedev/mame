/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *hyperspt_scroll;

static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Hyper Sports has one 32x8 palette PROM and two 256x4 lookup table PROMs
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
PALETTE_INIT( hyperspt )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
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

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* color_prom now points to the beginning of the lookup table */


	/* sprites */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = *(color_prom++) & 0x0f;

	/* characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = (*(color_prom++) & 0x0f) + 0x10;
}

WRITE8_HANDLER( hyperspt_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( hyperspt_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( hyperspt_flipscreen_w )
{
	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] + ((colorram[tile_index] & 0x80) << 1) + ((colorram[tile_index] & 0x40) << 3);
	int color = colorram[tile_index] & 0x0f;
	int flags = ((colorram[tile_index] & 0x10) ? TILE_FLIPX : 0) | ((colorram[tile_index] & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( hyperspt )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 64, 32);

	tilemap_set_scroll_rows(bg_tilemap, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx = spriteram[offs + 3];
		int sy = 240 - spriteram[offs + 1];
		int flipx = ~spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;

		if (flip_screen)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		/* Note that this adjustment must be done AFTER handling flip_screen, thus */
		/* proving that this is a hardware related "feature" */

		sy += 1;

		drawgfx(bitmap,machine->gfx[1],
			spriteram[offs + 2] + 8 * (spriteram[offs] & 0x20),
			spriteram[offs] & 0x0f,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_COLOR, 0);

		/* redraw with wraparound */

		drawgfx(bitmap,machine->gfx[1],
			spriteram[offs + 2] + 8 * (spriteram[offs] & 0x20),
			spriteram[offs] & 0x0f,
			flipx, flipy,
			sx - 256, sy,
			cliprect,
			TRANSPARENCY_COLOR, 0);
	}
}

VIDEO_UPDATE( hyperspt )
{
	int row;

	for (row = 0; row < 32; row++)
	{
		int scrollx = hyperspt_scroll[row * 2] + (hyperspt_scroll[(row * 2) + 1] & 0x01) * 256;
		if (flip_screen) scrollx = -scrollx;
		tilemap_set_scrollx(bg_tilemap, row, scrollx);
	}

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}

/* Road Fighter */

static TILE_GET_INFO( roadf_get_bg_tile_info )
{
	int code = videoram[tile_index] + ((colorram[tile_index] & 0x80) << 1) + ((colorram[tile_index] & 0x60) << 4);
	int color = colorram[tile_index] & 0x0f;
	int flags = (colorram[tile_index] & 0x10) ? TILE_FLIPX : 0;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( roadf )
{
	bg_tilemap = tilemap_create(roadf_get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 64, 32);

	tilemap_set_scroll_rows(bg_tilemap, 32);
}
