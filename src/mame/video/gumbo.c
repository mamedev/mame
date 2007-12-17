/* Gumbo video */

#include "driver.h"

extern UINT16 *gumbo_bg_videoram;
extern UINT16 *gumbo_fg_videoram;

static tilemap *gumbo_bg_tilemap;
static tilemap *gumbo_fg_tilemap;

WRITE16_HANDLER( gumbo_bg_videoram_w )
{
	COMBINE_DATA(&gumbo_bg_videoram[offset]);
	tilemap_mark_tile_dirty(gumbo_bg_tilemap,offset);
}

static TILE_GET_INFO( get_gumbo_bg_tile_info )
{
	int tileno;
	tileno = gumbo_bg_videoram[tile_index];
	SET_TILE_INFO(0,tileno,0,0);
}


WRITE16_HANDLER( gumbo_fg_videoram_w )
{
	COMBINE_DATA(&gumbo_fg_videoram[offset]);
	tilemap_mark_tile_dirty(gumbo_fg_tilemap,offset);
}

static TILE_GET_INFO( get_gumbo_fg_tile_info )
{
	int tileno;
	tileno = gumbo_fg_videoram[tile_index];
	SET_TILE_INFO(1,tileno,1,0);
}


VIDEO_START( gumbo )
{
	gumbo_bg_tilemap = tilemap_create(get_gumbo_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,      8, 8, 64,32);
	gumbo_fg_tilemap = tilemap_create(get_gumbo_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 4, 4,128,64);
	tilemap_set_transparent_pen(gumbo_fg_tilemap,0xff);
}

VIDEO_UPDATE( gumbo )
{
	tilemap_draw(bitmap,cliprect,gumbo_bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,gumbo_fg_tilemap,0,0);
	return 0;
}
