#include "driver.h"


UINT16 *pirates_tx_tileram, *pirates_spriteram;
UINT16 *pirates_fg_tileram,  *pirates_bg_tileram;
UINT16 *pirates_scroll;

static tilemap *tx_tilemap, *fg_tilemap, *bg_tilemap;

/* Video Hardware */

/* tilemaps */

static TILE_GET_INFO( get_tx_tile_info )
{
	int code = pirates_tx_tileram[tile_index*2];
	int colr = pirates_tx_tileram[tile_index*2+1];

	SET_TILE_INFO(0,code,colr,0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = pirates_fg_tileram[tile_index*2];
	int colr = pirates_fg_tileram[tile_index*2+1]+0x80;

	SET_TILE_INFO(0,code,colr,0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = pirates_bg_tileram[tile_index*2];
	int colr = pirates_bg_tileram[tile_index*2+1]+ 0x100;

	SET_TILE_INFO(0,code,colr,0);
}


/* video start / update */

VIDEO_START(pirates)
{
	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,36,32);

	/* Not sure how big they can be, Pirates uses only 32 columns, Genix 44 */
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,64,32);
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,     8,8,64,32);

	tilemap_set_transparent_pen(tx_tilemap,0);
	tilemap_set_transparent_pen(fg_tilemap,0);
}



WRITE16_HANDLER( pirates_tx_tileram_w )
{
	COMBINE_DATA(pirates_tx_tileram+offset);
	tilemap_mark_tile_dirty(tx_tilemap,offset/2);
}

WRITE16_HANDLER( pirates_fg_tileram_w )
{
	COMBINE_DATA(pirates_fg_tileram+offset);
	tilemap_mark_tile_dirty(fg_tilemap,offset/2);
}

WRITE16_HANDLER( pirates_bg_tileram_w )
{
	COMBINE_DATA(pirates_bg_tileram+offset);
	tilemap_mark_tile_dirty(bg_tilemap,offset/2);
}



static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	const gfx_element *gfx = machine->gfx[1];
	UINT16 *source = pirates_spriteram + 4;
	UINT16 *finish = source + 0x800/2-4;

	while( source<finish )
	{
		int xpos, ypos, flipx, flipy, code, color;

		xpos = source[1] - 32;
		ypos = source[-1];	// indeed...

		if (ypos & 0x8000) break;	/* end-of-list marker */

		code = source[2] >> 2;
		color = source[0] & 0xff;
		flipx = source[2] & 2;
		flipy = source[2] & 1;

		ypos = 0xf2 - ypos;

		drawgfx(bitmap,gfx,
				code,
				color,
				flipx,flipy,
				xpos,ypos,
				cliprect,TRANSPARENCY_PEN,0);

		source+=4;
	}
}

VIDEO_UPDATE(pirates)
{
	tilemap_set_scrollx(bg_tilemap,0,pirates_scroll[0]);
	tilemap_set_scrollx(fg_tilemap,0,pirates_scroll[0]);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}
