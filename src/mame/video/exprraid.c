#include "driver.h"

static int bg_index[4];

static tilemap *bg_tilemap, *fg_tilemap;

WRITE8_HANDLER( exprraid_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( exprraid_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( exprraid_flipscreen_w )
{
	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( exprraid_bgselect_w )
{
	if (bg_index[offset] != data)
	{
		bg_index[offset] = data;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

WRITE8_HANDLER( exprraid_scrollx_w )
{
	tilemap_set_scrollx(bg_tilemap, offset, data);
}

WRITE8_HANDLER( exprraid_scrolly_w )
{
	tilemap_set_scrolly(bg_tilemap, 0, data);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *tilerom = memory_region(REGION_GFX4);

	int data, attr, bank, code, color, flags;
	int quadrant = 0, offs;

	int sx = tile_index % 32;
	int sy = tile_index / 32;

	if (sx >= 16) quadrant++;
	if (sy >= 16) quadrant += 2;

	offs = (sy % 16) * 16 + (sx % 16) + (bg_index[quadrant] & 0x3f) * 0x100;

	data = tilerom[offs];
	attr = tilerom[offs + 0x4000];
	bank = (2 * (attr & 0x03) + ((data & 0x80) >> 7)) + 2;
	code = data & 0x7f;
	color = (attr & 0x18) >> 3;
	flags = (attr & 0x04) ? TILE_FLIPX : 0;

	tileinfo->category = ((attr & 0x80) ? 1 : 0);

	SET_TILE_INFO(bank, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0x07) << 8);
	int color = (attr & 0x10) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( exprraid )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 16, 16, 32, 32);

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_scroll_rows(bg_tilemap, 2);
	tilemap_set_transparent_pen(fg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs + 3] + ((attr & 0xe0) << 3);
		int color = (attr & 0x03) + ((attr & 0x08) >> 1);
		int flipx = (attr & 0x04);
		int flipy = 0;
		int sx = ((248 - spriteram[offs + 2]) & 0xff) - 8;
		int sy = spriteram[offs];

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap, machine->gfx[1],
			code, color,
			flipx, flipy,
			sx, sy,
			0, TRANSPARENCY_PEN, 0);

		/* double height */

		if (attr & 0x10)
		{
			drawgfx(bitmap,machine->gfx[1],
				code + 1, color,
				flipx, flipy,
				sx, sy + (flip_screen ? -16 : 16),
				cliprect, TRANSPARENCY_PEN, 0);
		}
	}
}

VIDEO_UPDATE( exprraid )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 1, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
