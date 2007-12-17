#include "driver.h"

static int battlex_scroll_lsb;
static int battlex_scroll_msb;

static tilemap *bg_tilemap;

PALETTE_INIT( battlex )
{
	int i,col;

	for (col = 0;col < 8;col++)
	{
		for (i = 0;i < 16;i++)
		{
			int data = i | col;
			int g = pal1bit(data >> 0);
			int b = pal1bit(data >> 1);
			int r = pal1bit(data >> 2);

#if 0
			/* from Tim's shots, bit 3 seems to have no effect (see e.g. Laser Ship on title screen) */
			if (i & 8)
			{
				r /= 2;
				g /= 2;
				b /= 2;
			}
#endif

			palette_set_color(machine, i + 16 * col,MAKE_RGB(r,g,b));
		}
	}
}

WRITE8_HANDLER( battlex_palette_w )
{
	palette_set_color_rgb(Machine,16*8 + offset,pal1bit(data >> 2),pal1bit(data >> 0),pal1bit(data >> 1));
}

WRITE8_HANDLER( battlex_scroll_x_lsb_w )
{
	battlex_scroll_lsb = data;
}

WRITE8_HANDLER( battlex_scroll_x_msb_w )
{
	battlex_scroll_msb = data;
}

WRITE8_HANDLER( battlex_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( battlex_flipscreen_w )
{
	/* bit 4 is used, but for what? */

	/* bit 7 is flip screen */

	if (flip_screen != (data & 0x80))
	{
		flip_screen_set(data & 0x80);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int tile = videoram[tile_index*2] | (((videoram[tile_index*2+1] & 0x01)) << 8);
	int color = (videoram[tile_index*2+1] & 0x0e) >> 1;

	SET_TILE_INFO(0,tile,color,0);
}

VIDEO_START( battlex )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 64, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	const gfx_element *gfx = machine->gfx[1];
	UINT8 *source = spriteram;
	UINT8 *finish = spriteram + 0x200;

	while( source<finish )
	{
		int sx = (source[0] & 0x7f) * 2 - (source[0] & 0x80) * 2;
		int sy = source[3];
		int tile = source[2] & 0x7f;
		int color = source[1] & 0x07;	/* bits 3,4,5 also used during explosions */
		int flipy = source[1] & 0x80;
		int flipx = source[1] & 0x40;

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap,gfx,tile,color,flipx,flipy,sx,sy,cliprect,TRANSPARENCY_PEN,0);

		source += 4;
	}

}

VIDEO_UPDATE(battlex)
{
	tilemap_set_scrollx(bg_tilemap, 0, battlex_scroll_lsb | (battlex_scroll_msb << 8));
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
