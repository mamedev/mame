/* Poke Champ */

#include "driver.h"

static tilemap *bg_tilemap;

WRITE8_HANDLER( pokechmp_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( pokechmp_flipscreen_w )
{
	if (flip_screen_get(space->machine) != (data & 0x80))
	{
		flip_screen_set(space->machine, data & 0x80);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = machine->generic.videoram.u8[tile_index*2+1] + ((machine->generic.videoram.u8[tile_index*2] & 0x3f) << 8);
	int color = machine->generic.videoram.u8[tile_index*2] >> 6;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( pokechmp )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	for (offs = 0;offs < machine->generic.spriteram_size;offs += 4)
	{
		if (spriteram[offs] != 0xf8)
		{
			int sx,sy,flipx,flipy;


			sx = 240 - spriteram[offs+2];
			sy = 240 - spriteram[offs];

			flipx = spriteram[offs+1] & 0x04;
			flipy = spriteram[offs+1] & 0x02;
			if (flip_screen_get(machine)) {
				sx=240-sx;
				sy=240-sy;
				if (flipx) flipx=0; else flipx=1;
				if (flipy) flipy=0; else flipy=1;
			}

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					spriteram[offs+3] + ((spriteram[offs+1] & 1) << 8),
					(spriteram[offs+1] & 0xf0) >> 4,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

VIDEO_UPDATE( pokechmp )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
