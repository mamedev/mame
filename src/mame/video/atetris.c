/***************************************************************************

    Atari Tetris hardware

***************************************************************************/

#include "driver.h"
#include "atetris.h"


static tilemap *bg_tilemap;


/*************************************
 *
 *  Tilemap callback
 *
 *************************************/

static TILE_GET_INFO( get_tile_info )
{
	int code = videoram[tile_index * 2] | ((videoram[tile_index * 2 + 1] & 7) << 8);
	int color = (videoram[tile_index * 2 + 1] & 0xf0) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

WRITE8_HANDLER( atetris_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atetris )
{
	bg_tilemap = tilemap_create(get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 64,32);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( atetris )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0,0);
	return 0;
}
