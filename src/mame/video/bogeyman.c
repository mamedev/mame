#include "driver.h"

UINT8 *bogeyman_videoram2, *bogeyman_colorram2;

static tilemap *bg_tilemap, *fg_tilemap;

PALETTE_INIT( bogeyman )
{
	int i;

	/* first 16 colors are RAM */

	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[256] >> 0) & 0x01;
		bit2 = (color_prom[256] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[256] >> 2) & 0x01;
		bit2 = (color_prom[256] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i+16,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( bogeyman_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( bogeyman_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( bogeyman_videoram2_w )
{
	bogeyman_videoram2[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( bogeyman_colorram2_w )
{
	bogeyman_colorram2[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( bogeyman_paletteram_w )
{
	/* RGB output is inverted */
	paletteram_BBGGGRRR_w(offset, ~data);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int gfxbank = ((((attr & 0x01) << 8) + videoram[tile_index]) / 0x80) + 3;
	int code = videoram[tile_index] & 0x7f;
	int color = (attr >> 1) & 0x07;

	SET_TILE_INFO(gfxbank, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int attr = bogeyman_colorram2[tile_index];
	int tile = bogeyman_videoram2[tile_index] | ((attr & 0x03) << 8);
	int gfxbank = tile / 0x200;
	int code = tile & 0x1ff;

	SET_TILE_INFO(gfxbank, code, 0, 0);
}

VIDEO_START( bogeyman )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 16, 16, 16, 16);

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0; offs < spriteram_size; offs += 4)
	{
		int attr = spriteram[offs];

		if (attr & 0x01)
		{
			int code = spriteram[offs + 1] + ((attr & 0x40) << 2);
			int color = (attr & 0x08) >> 3;
			int flipx = !(attr & 0x04);
			int flipy = attr & 0x02;
			int sx = spriteram[offs + 3];
			int sy = (240 - spriteram[offs + 2]) & 0xff;
			int multi = attr & 0x10;

			if (multi) sy -= 16;

			if (flip_screen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx(bitmap, machine->gfx[2],
				code, color,
				flipx, flipy,
				sx, sy,
				cliprect,
				TRANSPARENCY_PEN, 0);

			if (multi)
			{
				drawgfx(bitmap,machine->gfx[2],
					code + 1, color,
					flipx, flipy,
					sx, sy + (flip_screen ? -16 : 16),
					cliprect,
					TRANSPARENCY_PEN, 0);
			}
		}
	}
}

VIDEO_UPDATE( bogeyman )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
