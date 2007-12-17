/***************************************************************************

    Battle Lane Vol. 5

***************************************************************************/

#include "driver.h"

UINT8 *battlane_spriteram;
UINT8 *battlane_tileram;

static tilemap *bg_tilemap;

static int battlane_video_ctrl;
extern int battlane_cpu_control;

static mame_bitmap *screen_bitmap;

/*
    Video control register

        0x80    = low bit of blue component (taken when writing to palette)
        0x0e    = Bitmap plane (bank?) select  (0-7)
        0x01    = Scroll MSB
*/

WRITE8_HANDLER( battlane_palette_w )
{
	int r, g, b;
	int bit0, bit1, bit2;

	/* red component */

	bit0 = (~data >> 0) & 0x01;
	bit1 = (~data >> 1) & 0x01;
	bit2 = (~data >> 2) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	/* green component */

	bit0 = (~data >> 3) & 0x01;
	bit1 = (~data >> 4) & 0x01;
	bit2 = (~data >> 5) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	/* blue component */

	bit0 = (~battlane_video_ctrl >> 7) & 0x01;
	bit1 = (~data >> 6) & 0x01;
	bit2 = (~data >> 7) & 0x01;
	b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	palette_set_color(Machine, offset, MAKE_RGB(r, g, b));
}

WRITE8_HANDLER( battlane_scrollx_w )
{
	tilemap_set_scrollx(bg_tilemap, 0, ((battlane_video_ctrl & 0x01) << 8) + data);
}

WRITE8_HANDLER( battlane_scrolly_w )
{
	tilemap_set_scrolly(bg_tilemap, 0, ((battlane_cpu_control & 0x01) << 8) + data);
}

WRITE8_HANDLER( battlane_tileram_w )
{
    battlane_tileram[offset] = data;
	//tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( battlane_spriteram_w )
{
    battlane_spriteram[offset] = data;
}

WRITE8_HANDLER( battlane_bitmap_w )
{
	int i, orval;

    orval = (~battlane_video_ctrl >> 1) & 0x07;

	if (!orval)
		orval = 7;

	for (i = 0; i < 8; i++)
	{
		if (data & 1 << i)
		{
			*BITMAP_ADDR8(screen_bitmap, offset % 0x100, (offset / 0x100) * 8 + i) |= orval;
		}
		else
		{
			*BITMAP_ADDR8(screen_bitmap, offset % 0x100, (offset / 0x100) * 8 + i) &= ~orval;
		}
	}
}

WRITE8_HANDLER( battlane_video_ctrl_w )
{
	battlane_video_ctrl = data;
}

static TILE_GET_INFO( get_tile_info_bg )
{
	int code = battlane_tileram[tile_index];
	int attr = battlane_tileram[tile_index + 0x400];
	int gfxn = (attr & 0x01) + 1;
	int color = (attr >> 1) & 0x03;

	SET_TILE_INFO(gfxn, code, color, 0);
}

static TILEMAP_MAPPER( battlane_tilemap_scan_rows_2x2 )
{
	/*
            Tilemap Memory Organization

         0              15 16            31
        +-----------------+----------------+
        |0              15|256             |0
        |                 |                |
        |     screen 0    |    screen 1    |
        |                 |                |
        |240           255|             511|15
        +-----------------+----------------+
        |512              |768             |16
        |                 |                |
        |     screen 2    |    screen 3    |
        |                 |                |
        |              767|            1023|31
        +-----------------+-----------------

    */

	return (row & 0xf) * 16 + (col & 0xf) + (row & 0x10) * 32 + (col & 0x10) * 16;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
VIDEO_START( battlane )
{
	bg_tilemap = tilemap_create(get_tile_info_bg, battlane_tilemap_scan_rows_2x2,
		TILEMAP_TYPE_PEN, 16, 16, 32, 32);

	screen_bitmap = auto_bitmap_alloc(32 * 8, 32 * 8, BITMAP_FORMAT_INDEXED8);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs, attr, code, color, sx, sy, flipx, flipy, dy;

	for (offs = 0; offs < 0x100; offs += 4)
	{
		/*
            0x80 = Bank 2
            0x40 =
            0x20 = Bank 1
            0x10 = Y Double
            0x08 = Color
            0x04 = X Flip
            0x02 = Y Flip
            0x01 = Sprite Enable
        */

		attr = battlane_spriteram[offs + 1];
		code = battlane_spriteram[offs + 3];

		code += 256 * ((attr >> 6) & 0x02);
		code += 256 * ((attr >> 5) & 0x01);

		if (attr & 0x01)
		{
			color = (attr >> 3) & 0x01;

			sx = battlane_spriteram[offs + 2];
			sy = battlane_spriteram[offs];

			flipx = attr & 0x04;
			flipy = attr & 0x02;

			if (!flip_screen)
            {
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx(bitmap,machine->gfx[0],
				code,
				color,
				flipx, flipy,
				sx, sy,
				cliprect,
				TRANSPARENCY_PEN, 0);

			if (attr & 0x10)  /* Double Y direction */
			{
				dy = flipy ? 16 : -16;

				drawgfx(bitmap,machine->gfx[0],
					code + 1,
					color,
					flipx, flipy,
					sx, sy + dy,
					cliprect,
					TRANSPARENCY_PEN, 0);
			}
		}
	}
}

static void draw_fg_bitmap(running_machine *machine, mame_bitmap *bitmap )
{
	int x, y, data;

	for (y = 0; y < 32 * 8; y++)
	{
		for (x = 0; x < 32 * 8; x++)
		{
			data = *BITMAP_ADDR8(screen_bitmap, y, x);

			if (data)
			{
				if (flip_screen)
				{
					*BITMAP_ADDR16(bitmap, 255 - y, 255 - x) = machine->pens[data];
				}
				else
				{
					*BITMAP_ADDR16(bitmap, y, x) = machine->pens[data];
				}
			}
		}
	}
}

VIDEO_UPDATE( battlane )
{
	tilemap_mark_all_tiles_dirty(bg_tilemap); // HACK

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	draw_fg_bitmap(machine, bitmap);
	return 0;
}
