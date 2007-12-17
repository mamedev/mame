/***************************************************************************

    video.c

    Functions to emulate the video hardware of the machine.

    There are only a few differences between the video hardware of Mysterious
    Stones and Mat Mania. The tile bank select bit is different and the sprite
    selection seems to be different as well. Additionally, the palette is stored
    differently. I'm also not sure that the 2nd tile page is really used in
    Mysterious Stones.

***************************************************************************/

#include "driver.h"


UINT8 *mystston_videoram2;

static int mystston_fgcolor, mystston_bgpage;

static tilemap *fg_tilemap, *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mysterious Stones has both palette RAM and a PROM. The PROM is used for
  text.

***************************************************************************/

PALETTE_INIT( mystston )
{
	int i;

	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		// red component

		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;

		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component

		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;

		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component

		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;

		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i + 24, MAKE_RGB(r, g, b));	// first 24 colors are from RAM

		color_prom++;
	}
}

WRITE8_HANDLER( mystston_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset & 0x3ff);
}

WRITE8_HANDLER( mystston_videoram2_w )
{
	mystston_videoram2[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset & 0x1ff);
}

WRITE8_HANDLER( mystston_scroll_w )
{
	tilemap_set_scrolly(bg_tilemap, 0, data);
}

WRITE8_HANDLER( mystston_control_w )
{
	// bits 0 and 1 are foreground text color
	if (mystston_fgcolor != ((data & 0x01) << 1) + ((data & 0x02) >> 1))
	{
		mystston_fgcolor = ((data & 0x01) << 1) + ((data & 0x02) >> 1);
		tilemap_mark_all_tiles_dirty(fg_tilemap);
	}

	// bit 2 is background page select
	mystston_bgpage = (data & 0x04) ? 1:0;

	// bits 4 and 5 are coin counters in flipped order
	coin_counter_w(0, data & 0x20);
	coin_counter_w(1, data & 0x10);

	// bit 7 is screen flip
	flip_screen_set((data & 0x80) ^ ((readinputport(3) & 0x20) ? 0x80:0));
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = mystston_videoram2[tile_index] + ((mystston_videoram2[tile_index + 0x200] & 0x01) << 8);
	int flags = (tile_index & 0x10) ? TILE_FLIPY : 0;

	SET_TILE_INFO(1, code, 0, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = videoram[tile_index] + ((videoram[tile_index + 0x400] & 0x07) << 8);
	int color = mystston_fgcolor;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( mystston )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_cols_flip_x,
		TILEMAP_TYPE_PEN, 16, 16, 16, 32);

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_cols_flip_x,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = 0; offs < spriteram_size; offs += 4)
	{
		int attr = spriteram[offs];

		if (attr & 0x01)
		{
			int code = spriteram[offs + 1] + ((attr & 0x10) << 4);
			int color = (attr & 0x08) >> 3;
			int flipx = attr & 0x04;
			int flipy = attr & 0x02;
			int sx = 240 - spriteram[offs + 3];
			int sy = (240 - spriteram[offs + 2]) & 0xff;

			if (flip_screen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx(bitmap,machine->gfx[2],	code, color, flipx, flipy,
				sx, sy, cliprect, TRANSPARENCY_PEN, 0);
		}
	}
}

VIDEO_UPDATE( mystston )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
