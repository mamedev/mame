#include "driver.h"
#include "includes/iqblock.h"


UINT8 *iqblock_bgvideoram;
//UINT8 *iqblock_fgscroll;
UINT8 *iqblock_fgvideoram;
int iqblock_videoenable;
int iqblock_video_type;

static tilemap *bg_tilemap,*fg_tilemap;


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = iqblock_bgvideoram[tile_index] + (iqblock_bgvideoram[tile_index + 0x800] << 8);
	SET_TILE_INFO(
			0,
			code &(iqblock_video_type ? 0x1fff : 0x3fff),
			iqblock_video_type? (2*(code >> 13)+1) : (4*(code >> 14)+3),
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = iqblock_fgvideoram[tile_index];
	SET_TILE_INFO(
			1,
			code & 0x7f,
			(code & 0x80) ? 3 : 0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( iqblock )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,     8, 8,64,32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,32,64, 8);

	tilemap_set_transparent_pen(bg_tilemap,0);
	tilemap_set_scroll_cols(fg_tilemap,64);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( iqblock_fgvideoram_w )
{
	iqblock_fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE8_HANDLER( iqblock_bgvideoram_w )
{
	iqblock_bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x7ff);
}

READ8_HANDLER( iqblock_bgvideoram_r )
{
	return iqblock_bgvideoram[offset];
}

WRITE8_HANDLER( iqblock_fgscroll_w )
{
	tilemap_set_scrolly(fg_tilemap,offset,data);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( iqblock )
{
	if (!iqblock_videoenable) return 0;
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	return 0;
}

