/* video/pass.c - see drivers/pass.c for more info */

#include "driver.h"

static tilemap *pass_bg_tilemap;
static tilemap *pass_fg_tilemap;

/* in drivers/pass.c */
extern UINT16 *pass_bg_videoram;
extern UINT16 *pass_fg_videoram;
/* end in drivers/pass.c */

/* background tilemap stuff */

static TILE_GET_INFO( get_pass_bg_tile_info )
{
	int tileno,fx;

	tileno = pass_bg_videoram[tile_index] & 0x1fff;
	fx = (pass_bg_videoram[tile_index] & 0xc000) >> 14;
	SET_TILE_INFO(1,tileno,0,TILE_FLIPYX(fx));

}

WRITE16_HANDLER( pass_bg_videoram_w )
{
	pass_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(pass_bg_tilemap,offset);
}

/* foreground 'sprites' tilemap stuff */

static TILE_GET_INFO( get_pass_fg_tile_info )
{
	int tileno, flip;

	tileno = pass_fg_videoram[tile_index] & 0x3fff;
	flip = (pass_fg_videoram[tile_index] & 0xc000) >>14;

	SET_TILE_INFO(0,tileno,0,TILE_FLIPYX(flip));

}

WRITE16_HANDLER( pass_fg_videoram_w )
{
	pass_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(pass_fg_tilemap,offset);
}

/* video update / start */

VIDEO_UPDATE( pass )
{
	tilemap_draw(bitmap,cliprect,pass_bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,pass_fg_tilemap,0,0);

	return 0;
}

VIDEO_START( pass )
{
	pass_bg_tilemap = tilemap_create(get_pass_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 8, 8,64,32);
	pass_fg_tilemap = tilemap_create(get_pass_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN, 4, 4,128,64);

	tilemap_set_transparent_pen(pass_fg_tilemap,255);
}
