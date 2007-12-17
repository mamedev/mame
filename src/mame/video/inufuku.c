/******************************************************************************

    Video Hardware for Video System Games.

    Quiz & Variety Sukusuku Inufuku
    (c)1998 Video System Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2003/08/09 -

    based on other Video System drivers

******************************************************************************/

#include "driver.h"


extern UINT16 *inufuku_spriteram1;
extern UINT16 *inufuku_spriteram2;
extern size_t inufuku_spriteram1_size;

extern UINT16 *inufuku_bg_videoram;
extern UINT16 *inufuku_bg_rasterram;
extern UINT16 *inufuku_text_videoram;

static int inufuku_bg_scrollx, inufuku_bg_scrolly;
static int inufuku_text_scrollx, inufuku_text_scrolly;
static int inufuku_bg_raster;
static int inufuku_bg_palettebank;
static int inufuku_text_palettebank;

static tilemap *inufuku_bg_tilemap;
static tilemap *inufuku_text_tilemap;


/******************************************************************************

    Memory handlers

******************************************************************************/

WRITE16_HANDLER( inufuku_palettereg_w )
{
	switch (offset) {
		case 0x02:	inufuku_bg_palettebank = (data & 0xf000) >> 12;
					tilemap_mark_all_tiles_dirty(inufuku_bg_tilemap);
					break;
		case 0x03:	inufuku_text_palettebank = (data & 0xf000) >> 12;
					tilemap_mark_all_tiles_dirty(inufuku_text_tilemap);
					break;
	}
}

WRITE16_HANDLER( inufuku_scrollreg_w )
{
	switch (offset) {
		case 0x00:	inufuku_bg_scrollx = data + 1; break;
		case 0x01:	inufuku_bg_scrolly = data + 0; break;
		case 0x02:	inufuku_text_scrollx = data - 3; break;
		case 0x03:	inufuku_text_scrolly = data + 1; break;
		case 0x04:	inufuku_bg_raster = (data & 0x0200) ? 0 : 1; break;
	}
}


/******************************************************************************

    Sprite routines

******************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = (inufuku_spriteram1_size / 16) - 1; offs >= 0; offs--) {

		if ((inufuku_spriteram1[offs] & 0x8000) == 0x0000) {

			int attr_start;
			int map_start;
			int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;
			int priority, priority_mask;

			attr_start = 4 * (inufuku_spriteram1[offs] & 0x03ff);

			/*
                attr_start + 0x0000
                ---- ---x xxxx xxxx oy
                ---- xxx- ---- ---- ysize
                xxxx ---- ---- ---- zoomy

                attr_start + 0x0001
                ---- ---x xxxx xxxx ox
                ---- xxx- ---- ---- xsize
                xxxx ---- ---- ---- zoomx

                attr_start + 0x0002
                -x-- ---- ---- ---- flipx
                x--- ---- ---- ---- flipy
                --xx xxxx ---- ---- color
                --xx ---- ---- ---- priority?
                ---- ---- xxxx xxxx unused?

                attr_start + 0x0003
                -xxx xxxx xxxx xxxx map start
                x--- ---- ---- ---- unused?
            */

			ox = (inufuku_spriteram1[attr_start + 1] & 0x01ff) + 0;
			xsize = (inufuku_spriteram1[attr_start + 1] & 0x0e00) >> 9;
			zoomx = (inufuku_spriteram1[attr_start + 1] & 0xf000) >> 12;
			oy = (inufuku_spriteram1[attr_start + 0] & 0x01ff) + 1;
			ysize = (inufuku_spriteram1[attr_start + 0] & 0x0e00) >> 9;
			zoomy = (inufuku_spriteram1[attr_start + 0] & 0xf000) >> 12;
			flipx = inufuku_spriteram1[attr_start + 2] & 0x4000;
			flipy = inufuku_spriteram1[attr_start + 2] & 0x8000;
			color = (inufuku_spriteram1[attr_start + 2] & 0x3f00) >> 8;
			priority = (inufuku_spriteram1[attr_start + 2] & 0x3000) >> 12;
			map_start = (inufuku_spriteram1[attr_start + 3] & 0x7fff) << 1;

			switch (priority) {
				default:
				case 0: priority_mask = 0x00; break;
				case 3:	priority_mask = 0xfe; break;
				case 2:	priority_mask = 0xfc; break;
				case 1:	priority_mask = 0xf0; break;
			}

			ox += (xsize * zoomx + 2) / 4;
			oy += (ysize * zoomy + 2) / 4;

			zoomx = 32 - zoomx;
			zoomy = 32 - zoomy;

			for (y = 0; y <= ysize; y++) {
				int sx, sy;

				if (flipy)
					sy = (oy + zoomy * (ysize - y) / 2 + 16) & 0x1ff;
				else
					sy = (oy + zoomy * y / 2 + 16) & 0x1ff;

				for (x = 0; x <= xsize; x++) {
					int code;

					if (flipx)
						sx = (ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff;
					else
						sx = (ox + zoomx * x / 2 + 16) & 0x1ff;

					code  = ((inufuku_spriteram2[map_start] & 0x0007) << 16) + inufuku_spriteram2[map_start + 1];

					pdrawgfxzoom(bitmap, machine->gfx[2],
							code,
							color,
							flipx, flipy,
							sx - 16, sy - 16,
							cliprect, TRANSPARENCY_PEN, 15,
							zoomx << 11, zoomy << 11,
							priority_mask);

					map_start += 2;
				}
			}
		}
	}
}


/******************************************************************************

    Tilemap callbacks

******************************************************************************/

static TILE_GET_INFO( get_inufuku_bg_tile_info )
{
	SET_TILE_INFO(
			0,
			inufuku_bg_videoram[tile_index],
			inufuku_bg_palettebank,
			0);
}

static TILE_GET_INFO( get_inufuku_text_tile_info )
{
	SET_TILE_INFO(
			1,
			inufuku_text_videoram[tile_index],
			inufuku_text_palettebank,
			0);
}

READ16_HANDLER( inufuku_bg_videoram_r )
{
	return inufuku_bg_videoram[offset];
}

WRITE16_HANDLER( inufuku_bg_videoram_w )
{
	COMBINE_DATA(&inufuku_bg_videoram[offset]);
	tilemap_mark_tile_dirty(inufuku_bg_tilemap, offset);
}

READ16_HANDLER( inufuku_text_videoram_r )
{
	return inufuku_text_videoram[offset];
}

WRITE16_HANDLER( inufuku_text_videoram_w )
{
	COMBINE_DATA(&inufuku_text_videoram[offset]);
	tilemap_mark_tile_dirty(inufuku_text_tilemap, offset);
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

VIDEO_START( inufuku )
{
	inufuku_bg_tilemap = tilemap_create(get_inufuku_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);
	inufuku_text_tilemap = tilemap_create(get_inufuku_text_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 64);

	tilemap_set_transparent_pen(inufuku_bg_tilemap, 255);
	tilemap_set_transparent_pen(inufuku_text_tilemap, 255);
}


/******************************************************************************

    Display refresh

******************************************************************************/

VIDEO_UPDATE( inufuku )
{
	int i;

	fillbitmap(bitmap, get_black_pen(machine), cliprect);
	fillbitmap(priority_bitmap, 0, NULL);

	if (inufuku_bg_raster) {
		tilemap_set_scroll_rows(inufuku_bg_tilemap, 512);
		for (i = 0; i < 256; i++) tilemap_set_scrollx(inufuku_bg_tilemap, (inufuku_bg_scrolly + i) & 0x1ff, inufuku_bg_rasterram[i]);
	}
	else {
		tilemap_set_scroll_rows(inufuku_bg_tilemap, 1);
		tilemap_set_scrollx(inufuku_bg_tilemap, 0, inufuku_bg_scrollx);
	}
	tilemap_set_scrolly(inufuku_bg_tilemap, 0, inufuku_bg_scrolly);
	tilemap_draw(bitmap, cliprect, inufuku_bg_tilemap, 0, 0);

	tilemap_set_scrollx(inufuku_text_tilemap, 0, inufuku_text_scrollx);
	tilemap_set_scrolly(inufuku_text_tilemap, 0, inufuku_text_scrolly);
	tilemap_draw(bitmap, cliprect, inufuku_text_tilemap, 0, 4);

	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
