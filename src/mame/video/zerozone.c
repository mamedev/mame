/***************************************************************************

  video/zerozone.c

***************************************************************************/

#include "driver.h"

UINT16 *zerozone_videoram;
static UINT16 zerozone_tilebank;

static tilemap *zerozone_tilemap;

WRITE16_HANDLER( zerozone_tilemap_w )
{
	COMBINE_DATA(&zerozone_videoram[offset]);
	tilemap_mark_tile_dirty(zerozone_tilemap,offset);
}


WRITE16_HANDLER(zerozone_tilebank_w)
{
//  popmessage ("Data %04x",data);
	zerozone_tilebank = data & 0x7;
	tilemap_mark_all_tiles_dirty(zerozone_tilemap);
}

static TILE_GET_INFO( get_zerozone_tile_info )
{
	int tileno,colour;
	tileno = zerozone_videoram[tile_index] & 0x07ff;
	colour = zerozone_videoram[tile_index] & 0xf000;

	if (zerozone_videoram[tile_index] & 0x0800) tileno += zerozone_tilebank * 0x800;

	SET_TILE_INFO(0,tileno,colour>>12,0);
}

VIDEO_START( zerozone )
{
	// i'm not 100% sure it should be opaque, pink title screen looks strange in las vegas girls
	// but if its transparent other things look incorrect
	zerozone_tilemap = tilemap_create(get_zerozone_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,      8, 8, 64,32);
}

VIDEO_UPDATE( zerozone )
{
	tilemap_draw(bitmap,cliprect,zerozone_tilemap,0,0);
	return 0;
}
