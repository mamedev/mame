/***************************************************************************

Tehkan World Cup - (c) Tehkan 1985


Ernesto Corvi
ernesto@imagina.com

Roberto Juan Fresca
robbiex@rocketmail.com

***************************************************************************/

#include "driver.h"

UINT8 *tehkanwc_videoram2;
static UINT8 scroll_x[2];
static UINT8 led0,led1;

static tilemap *bg_tilemap, *fg_tilemap;

WRITE8_HANDLER( tehkanwc_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( tehkanwc_colorram_w )
{
	space->machine->generic.colorram.u8[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( tehkanwc_videoram2_w )
{
	tehkanwc_videoram2[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( tehkanwc_scroll_x_w )
{
	scroll_x[offset] = data;
}

WRITE8_HANDLER( tehkanwc_scroll_y_w )
{
	tilemap_set_scrolly(bg_tilemap, 0, data);
}

WRITE8_HANDLER( tehkanwc_flipscreen_x_w )
{
	flip_screen_x_set(space->machine, data & 0x40);
}

WRITE8_HANDLER( tehkanwc_flipscreen_y_w )
{
	flip_screen_y_set(space->machine, data & 0x40);
}

WRITE8_HANDLER( gridiron_led0_w )
{
	led0 = data;
}
WRITE8_HANDLER( gridiron_led1_w )
{
	led1 = data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index * 2;
	int attr = tehkanwc_videoram2[offs + 1];
	int code = tehkanwc_videoram2[offs] + ((attr & 0x30) << 4);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(2, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int attr = machine->generic.colorram.u8[tile_index];
	int code = machine->generic.videoram.u8[tile_index] + ((attr & 0x10) << 4);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	tileinfo->category = (attr & 0x20) ? 0 : 1;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( tehkanwc )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 16, 8, 32, 32);

	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}

/*
   Gridiron Fight has a LED display on the control panel, to let each player
   choose the formation without letting the other know.

    ---0---
   |       |
   5       1
   |       |
    ---6---
   |       |
   4       2
   |       |
    ---3---

   bit 7 = enable (0 = display off)
 */

static void gridiron_draw_led(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 led,int player)
{
	if (led&0x80)
		output_set_digit_value(player, led&0x7f);
		else
		output_set_digit_value(player, 0x00);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	for (offs = 0;offs < machine->generic.spriteram_size;offs += 4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs] + ((attr & 0x08) << 5);
		int color = attr & 0x07;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram[offs + 2] + ((attr & 0x20) << 3) - 128;
		int sy = spriteram[offs + 3];

		if (flip_screen_x_get(machine))
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flip_screen_y_get(machine))
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine->gfx[1],
			code, color, flipx, flipy, sx, sy, 0);
	}
}

VIDEO_UPDATE( tehkanwc )
{
	tilemap_set_scrollx(bg_tilemap, 0, scroll_x[0] + 256 * scroll_x[1]);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 1, 0);
	gridiron_draw_led(screen->machine, bitmap, cliprect, led0, 0);
	gridiron_draw_led(screen->machine, bitmap, cliprect, led1, 1);
	return 0;
}
