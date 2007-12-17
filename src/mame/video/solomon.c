#include "driver.h"

UINT8 *solomon_videoram2;
UINT8 *solomon_colorram2;

static tilemap *bg_tilemap, *fg_tilemap;

WRITE8_HANDLER( solomon_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( solomon_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( solomon_videoram2_w )
{
	solomon_videoram2[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( solomon_colorram2_w )
{
	solomon_colorram2[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( solomon_flipscreen_w )
{
	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = solomon_colorram2[tile_index];
	int code = solomon_videoram2[tile_index] + 256 * (attr & 0x07);
	int color = ((attr & 0x70) >> 4);
	int flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x08) ? TILE_FLIPY : 0);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + 256 * (attr & 0x07);
	int color = (attr & 0x70) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( solomon )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int code = spriteram[offs] + 16 * (spriteram[offs + 1] & 0x10);
		int color = (spriteram[offs + 1] & 0x0e) >> 1;
		int flipx = spriteram[offs + 1] & 0x40;
		int flipy =	spriteram[offs + 1] & 0x80;
		int sx = spriteram[offs + 3];
		int sy = 241 - spriteram[offs + 2];

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 242 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap, machine->gfx[2],
			code, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( solomon )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
