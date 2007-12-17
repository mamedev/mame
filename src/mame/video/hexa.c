/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static int charbank;
static tilemap *bg_tilemap;

WRITE8_HANDLER( hexa_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( hexa_d008_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	int bankaddress;

	/* bit 0 = flipx (or y?) */
	if (flip_screen_x != (data & 0x01))
	{
		flip_screen_x_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	/* bit 1 = flipy (or x?) */
	if (flip_screen_y != (data & 0x02))
	{
		flip_screen_y_set(data & 0x02);
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	/* bit 2 - 3 unknown */

	/* bit 4 could be the ROM bank selector for 8000-bfff (not sure) */
	bankaddress = 0x10000 + ((data & 0x10) >> 4) * 0x4000;
	memory_set_bankptr(1, &RAM[bankaddress]);

	/* bit 5 = char bank */
	if (charbank != ((data & 0x20) >> 5))
	{
		charbank = (data & 0x20) >> 5;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	/* bit 6 - 7 unknown */
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index * 2;
	int tile = videoram[offs + 1] + ((videoram[offs] & 0x07) << 8) + (charbank << 11);
	int color = (videoram[offs] & 0xf8) >> 3;

	SET_TILE_INFO(0, tile, color, 0);
}

VIDEO_START( hexa )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}


VIDEO_UPDATE( hexa )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
