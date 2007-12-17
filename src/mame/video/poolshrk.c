/***************************************************************************

Atari Poolshark video emulation

***************************************************************************/

#include "driver.h"
#include "includes/poolshrk.h"

UINT8* poolshrk_playfield_ram;
UINT8* poolshrk_hpos_ram;
UINT8* poolshrk_vpos_ram;

static tilemap* bg_tilemap;


static TILE_GET_INFO( get_tile_info )
{
	SET_TILE_INFO(1, poolshrk_playfield_ram[tile_index] & 0x3f, 0, 0);
}


VIDEO_START( poolshrk )
{
	bg_tilemap = tilemap_create(get_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(bg_tilemap, 0);
}


VIDEO_UPDATE( poolshrk )
{
	int i;

	tilemap_mark_all_tiles_dirty(bg_tilemap);

	fillbitmap(bitmap, machine->pens[0], cliprect);

	/* draw sprites */

	for (i = 0; i < 16; i++)
	{
		int hpos = poolshrk_hpos_ram[i];
		int vpos = poolshrk_vpos_ram[i];

		drawgfx(bitmap, machine->gfx[0], i, (i == 0) ? 0 : 1, 0, 0,
			248 - hpos, vpos - 15, cliprect, TRANSPARENCY_PEN, 0);
	}

	/* draw playfield */

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
