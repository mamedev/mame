/*************************************************************************

    Atari Super Breakout hardware

*************************************************************************/

#include "driver.h"
#include "includes/sbrkout.h"

UINT8 *sbrkout_horiz_ram;
UINT8 *sbrkout_vert_ram;

static tilemap *bg_tilemap;

WRITE8_HANDLER( sbrkout_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = (videoram[tile_index] & 0x80) ? videoram[tile_index] : 0;

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( sbrkout )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_balls(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int ball;

	for (ball=2; ball>=0; ball--)
	{
		int code = ((sbrkout_vert_ram[ball * 2 + 1] & 0x80) >> 7);
		int sx = 31 * 8 - sbrkout_horiz_ram[ball * 2];
		int sy = 30 * 8 - sbrkout_vert_ram[ball * 2];

		drawgfx(bitmap, machine->gfx[1],
			code, 0,
			0, 0,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( sbrkout )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_balls(machine, bitmap, cliprect);
	return 0;
}
