#include "driver.h"
#include "news.h"


UINT8 *news_fgram;
UINT8 *news_bgram;

static int bgpic;
static tilemap *fg_tilemap, *bg_tilemap;



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = (news_fgram[tile_index*2] << 8) | news_fgram[tile_index*2+1];
	SET_TILE_INFO(
			0,
			code & 0x0fff,
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = (news_bgram[tile_index*2] << 8) | news_bgram[tile_index*2+1];
	int color = (code & 0xf000) >> 12;

	code &= 0x0fff;
	if ((code & 0x0e00) == 0x0e00) code = (code & 0x1ff) | (bgpic << 9);

	SET_TILE_INFO(
			0,
			code,
			color,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( news )
{

	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32, 32);
	tilemap_set_transparent_pen(fg_tilemap,0);

	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32, 32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( news_fgram_w )
{
	news_fgram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset/2);
}

WRITE8_HANDLER( news_bgram_w )
{
	news_bgram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset/2);
}

WRITE8_HANDLER( news_bgpic_w )
{
	if (bgpic != data)
	{
		bgpic = data;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( news )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	return 0;
}
