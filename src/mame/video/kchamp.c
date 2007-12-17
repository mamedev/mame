/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static tilemap *bg_tilemap;

PALETTE_INIT( kchamp )
{
	int i, red, green, blue;

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		red = color_prom[i];
		green = color_prom[machine->drv->total_colors+i];
		blue = color_prom[2*machine->drv->total_colors+i];

		palette_set_color_rgb(machine,i,pal4bit(red),pal4bit(green),pal4bit(blue));

		*(colortable++) = i;
	}
}

WRITE8_HANDLER( kchamp_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( kchamp_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( kchamp_flipscreen_w )
{
	flip_screen_set(data & 0x01);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] + ((colorram[tile_index] & 7) << 8);
	int color = (colorram[tile_index] >> 3) & 0x1f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( kchamp )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

/*
        Sprites
        -------
        Offset          Encoding
            0             YYYYYYYY
            1             TTTTTTTT
            2             FGGTCCCC
            3             XXXXXXXX
*/

static void kchamp_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

    for (offs = 0; offs < 0x100; offs += 4)
	{
		int attr = spriteram[offs + 2];
        int bank = 1 + ((attr & 0x60) >> 5);
        int code = spriteram[offs + 1] + ((attr & 0x10) << 4);
        int color = attr & 0x0f;
		int flipx = 0;
        int flipy = attr & 0x80;
        int sx = spriteram[offs + 3] - 8;
        int sy = 247 - spriteram[offs];

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

        drawgfx(bitmap, machine->gfx[bank], code, color, flipx, flipy, sx, sy,
            cliprect, TRANSPARENCY_PEN, 0);
	}
}

static void kchampvs_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

    for (offs = 0; offs < 0x100; offs += 4)
	{
		int attr = spriteram[offs + 2];
        int bank = 1 + ((attr & 0x60) >> 5);
        int code = spriteram[offs + 1] + ((attr & 0x10) << 4);
        int color = attr & 0x0f;
		int flipx = 0;
        int flipy = attr & 0x80;
        int sx = spriteram[offs + 3];
        int sy = 240 - spriteram[offs];

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

        drawgfx(bitmap, machine->gfx[bank], code, color, flipx, flipy, sx, sy,
            cliprect, TRANSPARENCY_PEN, 0);
	}
}


VIDEO_UPDATE( kchamp )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	kchamp_draw_sprites(machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( kchampvs )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	kchampvs_draw_sprites(machine, bitmap, cliprect);
	return 0;
}
