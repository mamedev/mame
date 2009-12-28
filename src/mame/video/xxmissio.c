/*******************************************************************************

XX Mission (c) 1986 UPL

Video hardware driver by Uki

    31/Mar/2001 -

*******************************************************************************/

#include "driver.h"

UINT8 *xxmissio_bgram;
UINT8 *xxmissio_fgram;
UINT8 *xxmissio_spriteram;

static tilemap_t *bg_tilemap;
static tilemap_t *fg_tilemap;
static UINT8 xscroll;
static UINT8 yscroll;
static UINT8 flipscreen;


WRITE8_DEVICE_HANDLER( xxmissio_scroll_x_w )
{
	xscroll = data;
}
WRITE8_DEVICE_HANDLER( xxmissio_scroll_y_w )
{
	yscroll = data;
}

WRITE8_HANDLER( xxmissio_flipscreen_w )
{
	flipscreen = data & 0x01;
}

WRITE8_HANDLER( xxmissio_bgram_w )
{
	int x = (offset + (xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	xxmissio_bgram[offset] = data;
}
READ8_HANDLER( xxmissio_bgram_r )
{
	int x = (offset + (xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	return xxmissio_bgram[offset];
}

WRITE8_HANDLER( xxmissio_paletteram_w )
{
	paletteram_BBGGRRII_w(space,offset,data);
}

/****************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = ((xxmissio_bgram[0x400 | tile_index] & 0xc0) << 2) | xxmissio_bgram[0x000 | tile_index];
	int color =  xxmissio_bgram[0x400 | tile_index] & 0x0f;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = xxmissio_fgram[0x000 | tile_index];
	int color = xxmissio_fgram[0x400 | tile_index] & 0x07;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( xxmissio )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 8, 32, 32);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 8, 32, 32);

	tilemap_set_scroll_cols(bg_tilemap, 1);
	tilemap_set_scroll_rows(bg_tilemap, 1);
	tilemap_set_scrolldx(bg_tilemap, 2, 12);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}


static void draw_sprites(bitmap_t *bitmap, const rectangle *cliprect, const gfx_element *gfx)
{
	int offs;
	int chr,col;
	int x,y,px,py,fx,fy;

	for (offs=0; offs<0x800; offs +=0x20)
	{
		chr = xxmissio_spriteram[offs];
		col = xxmissio_spriteram[offs+3];

		fx = ((col & 0x10) >> 4) ^ flipscreen;
		fy = ((col & 0x20) >> 5) ^ flipscreen;

		x = xxmissio_spriteram[offs+1]*2;
		y = xxmissio_spriteram[offs+2];

		chr = chr + ((col & 0x40) << 2);
		col = col & 0x07;

		if (flipscreen==0)
		{
			px = x-8;
			py = y;
		}
		else
		{
			px = 480-x-6;
			py = 240-y;
		}

		px &= 0x1ff;

		drawgfx_transpen(bitmap,cliprect,gfx,
			chr,
			col,
			fx,fy,
			px,py,0);

		if (px>0x1e0)
			drawgfx_transpen(bitmap,cliprect,gfx,
				chr,
				col,
				fx,fy,
				px-0x200,py,0);

	}
}


VIDEO_UPDATE( xxmissio )
{
	tilemap_mark_all_tiles_dirty_all(screen->machine);
	tilemap_set_flip_all(screen->machine, flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	tilemap_set_scrollx(bg_tilemap, 0, xscroll * 2);
	tilemap_set_scrolly(bg_tilemap, 0, yscroll);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(bitmap, cliprect, screen->machine->gfx[1]);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}
