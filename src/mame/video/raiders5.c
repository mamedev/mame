/*******************************************************************************

Raiders5 (c) 1985 Taito / UPL

Video hardware driver by Uki

    02/Jun/2001 -

*******************************************************************************/

#include "driver.h"


UINT8 *raiders5_foreground_videoram;
UINT8 *raiders5_foreground_colorram;
UINT8 *raiders5_background_videoram;
UINT8 *raiders5_background_colorram;
UINT8 *raiders5_spriteram;
size_t raiders5_spriteram_size;


static UINT8 raiders5_scroll_x;
static UINT8 raiders5_scroll_y;
static UINT8 raiders5_flip_screen;

static tilemap *background_tilemap;
static tilemap *foreground_tilemap;



/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

static TILE_GET_INFO( get_background_tile_info )
{
	UINT8 bank = ((raiders5_background_colorram[tile_index] >> 1) & 0x01) + 3;  /* ? */

	UINT16 code = raiders5_background_videoram[tile_index] |
				((raiders5_background_colorram[tile_index] & 0x01) << 8);

	UINT8 color = raiders5_background_colorram[tile_index] >> 4;

	SET_TILE_INFO(bank, code, color, 0);
}


static TILE_GET_INFO( get_foreground_tile_info )
{
	UINT16 code = raiders5_foreground_videoram[tile_index];

	UINT8 color = raiders5_foreground_colorram[tile_index] >> 4;

	SET_TILE_INFO(2, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( raiders5 )
{
	background_tilemap = tilemap_create(get_background_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	foreground_tilemap = tilemap_create(get_foreground_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_scrolldx(background_tilemap, 7, 0);

	tilemap_set_transparent_pen(foreground_tilemap, 0);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( raiders5_scroll_x_w )
{
	raiders5_scroll_x = data;
}


WRITE8_HANDLER( raiders5_scroll_y_w )
{
	raiders5_scroll_y = data;
}


WRITE8_HANDLER( raiders5_flip_screen_w )
{
	raiders5_flip_screen = data & 0x01;

	tilemap_set_flip(ALL_TILEMAPS, (raiders5_flip_screen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0));
}


WRITE8_HANDLER( raiders5_foreground_videoram_w )
{
	raiders5_foreground_videoram[offset] = data;
	tilemap_mark_tile_dirty(foreground_tilemap, offset);
}


WRITE8_HANDLER( raiders5_foreground_colorram_w )
{
	raiders5_foreground_colorram[offset] = data;
	tilemap_mark_tile_dirty(foreground_tilemap, offset);
}


WRITE8_HANDLER( raiders5_background_videoram_w )
{
	offs_t y = (offset + ((raiders5_scroll_y & 0xf8) << 2)) & 0x3e0;
	offs_t x = (offset + (raiders5_scroll_x >> 3)) & 0x1f;

	raiders5_background_videoram[y | x] = data;
	tilemap_mark_tile_dirty(background_tilemap, y | x);
}


READ8_HANDLER( raiders5_background_videoram_r )
{
	offs_t y = (offset + ((raiders5_scroll_y & 0xf8) << 2)) & 0x3e0;
	offs_t x = (offset + (raiders5_scroll_x >> 3)) & 0x1f;

	return raiders5_background_videoram[y | x];
}


WRITE8_HANDLER( raiders5_background_colorram_w )
{
	offs_t y = (offset + ((raiders5_scroll_y & 0xf8) << 2)) & 0x3e0;
	offs_t x = (offset + (raiders5_scroll_x >> 3)) & 0x1f;

	raiders5_background_colorram[y | x] = data;
	tilemap_mark_tile_dirty(background_tilemap, y | x);
}


READ8_HANDLER( raiders5_background_colorram_r )
{
	offs_t y = (offset + ((raiders5_scroll_y & 0xf8) << 2)) & 0x3e0;
	offs_t x = (offset + (raiders5_scroll_x >> 3)) & 0x1f;

	return raiders5_background_colorram[y | x];
}


WRITE8_HANDLER( raiders5_paletteram_w )
{
	paletteram_BBGGRRII_w(offset, data);

	if (offset < 0x10)
	{
		if (offset != 1)
		{
			int i;

			for (i = 0; i < 0x10; i++)
			{
				paletteram_BBGGRRII_w(offset + (i * 0x10) + 0x0200, data);
			}
		}

		paletteram_BBGGRRII_w((offset * 0x10) + 0x0201, data);
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	offs_t offs;

	for (offs = 0; offs < raiders5_spriteram_size; offs += 32)
	{
		UINT8 bank = (raiders5_spriteram[offs + 3] >> 1) & 0x01;

		UINT8 code = ((raiders5_spriteram[offs + 3] << 6) & 0x40) | (raiders5_spriteram[offs + 0] >> 2);

		UINT8 color = raiders5_spriteram[offs + 3] >> 4;

		int flip_y = ((raiders5_spriteram[offs + 0] >> 1) & 0x01) ^ raiders5_flip_screen;
		int flip_x = ((raiders5_spriteram[offs + 0] >> 0) & 0x01) ^ raiders5_flip_screen;

		UINT8 y = raiders5_spriteram[offs + 2];
		UINT8 x = raiders5_spriteram[offs + 1];

		if (raiders5_flip_screen)
		{
			y = 240 - y;
			x = 240 - x;
		}

		drawgfx(bitmap,machine->gfx[bank],
				code, color, flip_x, flip_y,
				x, y, cliprect, TRANSPARENCY_PEN, 0);

		/* draw it wrapped around */
		drawgfx(bitmap,machine->gfx[bank],
				code, color, flip_x, flip_y,
				x - 0x100, y, cliprect, TRANSPARENCY_PEN, 0);
	}
}


VIDEO_UPDATE( raiders5 )
{
	tilemap_set_scrolly(background_tilemap, 0, raiders5_scroll_y);
	tilemap_set_scrollx(background_tilemap, 0, raiders5_scroll_x);

	tilemap_draw(bitmap, cliprect, background_tilemap, 0, 0);

	draw_sprites(machine, bitmap, cliprect);

	tilemap_draw(bitmap, cliprect, foreground_tilemap, 0, 0);

	return 0;
}
