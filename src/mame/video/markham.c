/******************************************************************************

Markham (c) 1983 Sun Electronics

Video hardware driver by Uki

    17/Jun/2001 -

******************************************************************************/

#include "driver.h"

static UINT8 markham_xscroll[2];

static tilemap *bg_tilemap;

PALETTE_INIT( markham )
{
	int i;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int r = color_prom[0];
		int g = color_prom[machine->drv->total_colors];
		int b = color_prom[2*machine->drv->total_colors];

		palette_set_color_rgb(machine,i,pal4bit(r),pal4bit(g),pal4bit(b));
		color_prom++;
	}

	color_prom += 2*machine->drv->total_colors;

	/* color_prom now points to the beginning of the lookup table */

	/* sprites lookup table */
	for (i=0; i<512; i++)
		*(colortable++) = *(color_prom++);

	/* bg lookup table */
	for (i=0; i<512; i++)
		*(colortable++) = *(color_prom++);

}

WRITE8_HANDLER( markham_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( markham_scroll_x_w )
{
	markham_xscroll[offset] = data;
}

WRITE8_HANDLER( markham_flipscreen_w )
{
	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = videoram[tile_index * 2];
	int code = videoram[(tile_index * 2) + 1] + ((attr & 0x60) << 3);
	int color = (attr & 0x1f) | ((attr & 0x80) >> 2);

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( markham )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_cols,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_scroll_rows(bg_tilemap, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs=0x60; offs<0x100; offs +=4)
	{
		int chr = spriteram[offs+1];
		int col = spriteram[offs+2];

		int fx = flip_screen;
		int fy = flip_screen;

		int x = spriteram[offs+3];
		int y = spriteram[offs+0];
		int px,py;
		col &= 0x3f ;

		if (flip_screen==0)
		{
			px = x-2;
			py = 240-y;
		}
		else
		{
			px = 240-x;
			py = y;
		}

		px = px & 0xff;

		if (px>248)
			px = px-256;

		drawgfx(bitmap,machine->gfx[1],
			chr,
			col,
			fx,fy,
			px,py,
			cliprect,TRANSPARENCY_COLOR,0);
	}
}

VIDEO_UPDATE( markham )
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if ((i > 3) && (i<16))
			tilemap_set_scrollx(bg_tilemap, i, markham_xscroll[0]);
		if (i >= 16)
			tilemap_set_scrollx(bg_tilemap, i, markham_xscroll[1]);
	}

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
