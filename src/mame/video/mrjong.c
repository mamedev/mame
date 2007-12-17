/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs. (from video/penco.c)

***************************************************************************/
PALETTE_INIT( mrjong )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn, offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + (offs)])

	for (i = 0; i < machine->drv->total_colors; i++)
	{
		int bit0, bit1, bit2, r, g, b;

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

	color_prom += 0x10;
	/* color_prom now points to the beginning of the lookup table */

	/* character lookup table */
	/* sprites use the same color lookup table as characters */
	for (i = 0; i < TOTAL_COLORS(0); i++)
		COLOR(0, i) = *(color_prom++) & 0x0f;
}


/***************************************************************************

  Display control parameter.

***************************************************************************/
WRITE8_HANDLER( mrjong_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( mrjong_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( mrjong_flipscreen_w )
{
	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] | ((colorram[tile_index] & 0x20) << 3);
	int color = colorram[tile_index] & 0x1f;
	int flags = ((colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( mrjong )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows_flip_xy,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = (spriteram_size - 4); offs >= 0; offs -= 4)
	{
		int sprt;
		int color;
		int sx, sy;
		int flipx, flipy;

		sprt = (((spriteram[offs + 1] >> 2) & 0x3f) | ((spriteram[offs + 3] & 0x20) << 1));
		flipx = (spriteram[offs + 1] & 0x01) >> 0;
		flipy = (spriteram[offs + 1] & 0x02) >> 1;
		color = (spriteram[offs + 3] & 0x1f);

		sx = 224 - spriteram[offs + 2];
		sy = spriteram[offs + 0];
		if (flip_screen)
		{
			sx = 208 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap, machine->gfx[1],
				sprt,
				color,
				flipx, flipy,
				sx, sy,
				cliprect, TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( mrjong )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
