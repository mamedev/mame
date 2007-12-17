/***************************************************************************

Atari Canyon Bomber video emulation

***************************************************************************/

#include "driver.h"
#include "includes/canyon.h"

static tilemap *bg_tilemap;

UINT8* canyon_videoram;


WRITE8_HANDLER( canyon_videoram_w )
{
	canyon_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 code = canyon_videoram[tile_index];

	SET_TILE_INFO(0, code & 0x3f, code >> 7, 0);
}


VIDEO_START( canyon )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}


static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle* cliprect)
{
	int i;

	for (i = 0; i < 2; i++)
	{
		int x = canyon_videoram[0x3d0 + 2 * i + 0x1];
		int y = canyon_videoram[0x3d0 + 2 * i + 0x8];
		int c = canyon_videoram[0x3d0 + 2 * i + 0x9];

		drawgfx(bitmap, machine->gfx[1],
			c >> 3,
			i,
			!(c & 0x80), 0,
			224 - x,
			240 - y,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}


static void draw_bombs(mame_bitmap *bitmap, const rectangle* cliprect)
{
	int i;

	for (i = 0; i < 2; i++)
	{
		int sx = 254 - canyon_videoram[0x3d0 + 2 * i + 0x5];
		int sy = 246 - canyon_videoram[0x3d0 + 2 * i + 0xc];

		rectangle rect;

		rect.min_x = sx;
		rect.min_y = sy;
		rect.max_x = sx + 1;
		rect.max_y = sy + 1;

		if (rect.min_x < cliprect->min_x) rect.min_x = cliprect->min_x;
		if (rect.min_y < cliprect->min_y) rect.min_y = cliprect->min_y;
		if (rect.max_x > cliprect->max_x) rect.max_x = cliprect->max_x;
		if (rect.max_y > cliprect->max_y) rect.max_y = cliprect->max_y;

		fillbitmap(bitmap, i, &rect);
	}
}


VIDEO_UPDATE( canyon )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	draw_sprites(machine, bitmap, cliprect);

	draw_bombs(bitmap, cliprect);

	/* watchdog is disabled during service mode */
	watchdog_enable(!(readinputport(2) & 0x10));

	return 0;
}
