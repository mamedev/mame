#include "driver.h"

static tilemap *bg_tilemap, *fg_tilemap, *tx_tilemap;
static UINT8 bg_tile_bank, fg_tile_bank;
UINT8 *lkage_scroll, *lkage_vreg;

/*
    lkage_scroll[0x00]: text layer horizontal scroll
    lkage_scroll[0x01]: text layer vertical scroll
    lkage_scroll[0x02]: foreground layer horizontal scroll
    lkage_scroll[0x03]: foreground layer vertical scroll
    lkage_scroll[0x04]: background layer horizontal scroll
    lkage_scroll[0x05]: background layer vertical scroll

    lkage_vreg[0]: 0x00,0x04
        0x04: fg tile bank select
        0x08: ?

    lkage_vreg[1]: 0x7d
        0xf0: background/foreground palette select
        0x08: bg tile bank select
        0x07: priority config?

    lkage_vreg[2]: 0xf3
        0x03: flip screen x/y
        0xf0: normally 1111, but 1001 and 0001 inbetween stages (while the
        backgrounds are are being redrawn). These bits are probably used to enable
        individual layers, but we have no way of knowing the mapping.

    lkage_vreg:
        04 7d f3 : title screen 101
        0c 7d f3 : high score   101
        04 06 f3 : attract#1    110
        04 1e f3 : attract#2    110
        04 1e f3 : attract#3    110
        00 4e f3 : attract#4    110
*/

WRITE8_HANDLER( lkage_videoram_w )
{
	videoram[offset] = data;

	switch( offset/0x400 )
	{
	case 0:
		tilemap_mark_tile_dirty(tx_tilemap,offset & 0x3ff);
		break;

	case 1:
		tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
		break;

	case 2:
		tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
		break;

	default:
		break;
	}
} /* lkage_videoram_w */

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index + 0x800] + 256 * (bg_tile_bank?5:1);
	SET_TILE_INFO( 0/*gfx*/, code, 0/*color*/, 0/*flags*/ );
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = videoram[tile_index + 0x400] + 256 * (fg_tile_bank?1:0);
	SET_TILE_INFO( 0/*gfx*/, code, 0/*color*/, 0/*flags*/);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	int code = videoram[tile_index];
	SET_TILE_INFO( 0/*gfx*/, code, 0/*color*/, 0/*flags*/);
}

VIDEO_START( lkage )
{
	bg_tile_bank = fg_tile_bank = 0;

	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     8,8,32,32);
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(tx_tilemap,0);

	tilemap_set_scrolldx(bg_tilemap,-5,-5+24);
	tilemap_set_scrolldx(fg_tilemap,-3,-3+24);
	tilemap_set_scrolldx(tx_tilemap,-1,-1+24);
} /* VIDEO_START( lkage ) */


static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	const UINT8 *source = spriteram;
	const UINT8 *finish = source+0x60;
	while( source<finish )
	{
		int attributes = source[2];
		/* 0x01: horizontal flip
         * 0x02: vertical flip
         * 0x04: bank select
         * 0x08: sprite size
         * 0x70: color
         * 0x80: priority
         */
		int priority_mask = 0;
		int color = (attributes>>4)&7;
		int flipx = attributes&0x01;
		int flipy = attributes&0x02;
		int height = (attributes&0x08) ? 2 : 1;
		int sx = source[0]-15;
		int sy = 256-16*height-source[1];
		int sprite_number = source[3] + ((attributes & 0x04) << 6);
		int y;

		if( attributes&0x80 )
		{
			priority_mask = (0xf0|0xcc );
		}
		else
		{
			priority_mask = (0xf0);
		}

		if (flip_screen_x)
		{
			sx = 239 - sx - 24;
			flipx = !flipx;
		}
		if( flip_screen_y )
		{
			sy = 254 - 16*height - sy;
			flipy = !flipy;
		}
		if (height == 2 && !flipy)
		{
			sprite_number ^= 1;
		}

		for (y = 0;y < height;y++)
		{
			pdrawgfx(
				bitmap,
				machine->gfx[1],
				sprite_number ^ y,
				color,
				flipx,flipy,
				sx&0xff,
				sy + 16*y,
				cliprect,
				TRANSPARENCY_PEN,0,
				priority_mask );
		}
		source+=4;
	}
} /* draw_sprites */

VIDEO_UPDATE( lkage )
{
	int bank;

	flip_screen_x_set(~lkage_vreg[2] & 0x01);
	flip_screen_y_set(~lkage_vreg[2] & 0x02);

	bank = lkage_vreg[1]&0x08;
	if( bg_tile_bank != bank )
	{
		bg_tile_bank = bank;
		tilemap_mark_all_tiles_dirty( bg_tilemap );
	}

	bank = lkage_vreg[0]&0x04;
	if( fg_tile_bank != bank )
	{
		fg_tile_bank = bank;
		tilemap_mark_all_tiles_dirty( fg_tilemap );
	}

	tilemap_set_palette_offset( bg_tilemap, 0x300 + (lkage_vreg[1]&0xf0) );
	tilemap_set_palette_offset( fg_tilemap, 0x200 + (lkage_vreg[1]&0xf0) );
	tilemap_set_palette_offset( tx_tilemap, 0x110 );

	tilemap_set_scrollx(tx_tilemap,0,lkage_scroll[0]);
	tilemap_set_scrolly(tx_tilemap,0,lkage_scroll[1]);

	tilemap_set_scrollx(fg_tilemap,0,lkage_scroll[2]);
	tilemap_set_scrolly(fg_tilemap,0,lkage_scroll[3]);

	tilemap_set_scrollx(bg_tilemap,0,lkage_scroll[4]);
	tilemap_set_scrolly(bg_tilemap,0,lkage_scroll[5]);

	fillbitmap(priority_bitmap, 0, cliprect);
	if ((lkage_vreg[2] & 0xf0) == 0xf0)
	{
		tilemap_draw( bitmap,cliprect,bg_tilemap,0,1 );
		tilemap_draw( bitmap,cliprect,fg_tilemap,0,(lkage_vreg[1]&2)?2:4 );
		tilemap_draw( bitmap,cliprect,tx_tilemap,0,4 );
	}
	else
	{
		tilemap_draw( bitmap,cliprect,tx_tilemap,TILEMAP_DRAW_OPAQUE,0);
	}
	draw_sprites(machine, bitmap,cliprect );
	return 0;
}
