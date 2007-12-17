/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static tilemap *bg_tilemap;

WRITE8_HANDLER( fenraya_videoram_w )
{
	videoram[(offset&0x3ff)*2]=data;
	videoram[(offset&0x3ff)*2+1]=(offset&0xc00)>>10;
	tilemap_mark_tile_dirty(bg_tilemap,offset&0x3ff);
}

static TILE_GET_INFO( get_tile_info )
{
	int code = videoram[tile_index*2]+(videoram[tile_index*2+1]<<8);
	SET_TILE_INFO(
		0,
		code,
		0,
		0);
}

VIDEO_START( 4enraya )
{
	bg_tilemap = tilemap_create( get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32 );
	video_start_generic(machine);
}

VIDEO_UPDATE( 4enraya)
{
	tilemap_draw(bitmap,cliprect,bg_tilemap, 0,0);
	return 0;
}
