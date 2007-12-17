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
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( tehkanwc_colorram_w )
{
	colorram[offset] = data;
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
	flip_screen_x_set(data & 0x40);
}

WRITE8_HANDLER( tehkanwc_flipscreen_y_w )
{
	flip_screen_y_set(data & 0x40);
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
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0x10) << 4);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	tileinfo->category = (attr & 0x20) ? 0 : 1;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( tehkanwc )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 16, 8, 32, 32);

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}

/*
   Gridiron Fight has a LED display on the control panel, to let each player
   choose the formation without letting the other know.
   We emulate it by showing a character on the corner of the screen; the
   association between the bits of the port and the led segments is:

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

static void gridiron_draw_led(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT8 led,int player)
{
	int i;


	static UINT8 ledvalues[] =
			{ 0x86, 0xdb, 0xcf, 0xe6, 0xed, 0xfd, 0x87, 0xff, 0xf3, 0xf1 };


	if ((led & 0x80) == 0) return;

	for (i = 0;i < 10;i++)
	{
		if (led == ledvalues[i] ) break;
	}

	if (i < 10)
	{
		if (player == 0)
			drawgfx(bitmap,machine->gfx[0],
					0xc0 + i,
					0x0a,
					0,0,
					0,232,
					cliprect,TRANSPARENCY_NONE,0);
		else
			drawgfx(bitmap,machine->gfx[0],
					0xc0 + i,
					0x03,
					1,1,
					0,16,
					cliprect,TRANSPARENCY_NONE,0);
	}
else logerror("unknown LED %02x for player %d\n",led,player);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs] + ((attr & 0x08) << 5);
		int color = attr & 0x07;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram[offs + 2] + ((attr & 0x20) << 3) - 128;
		int sy = spriteram[offs + 3];

		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flip_screen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap, machine->gfx[1],
			code, color, flipx, flipy, sx, sy,
			cliprect, TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( tehkanwc )
{
	tilemap_set_scrollx(bg_tilemap, 0, scroll_x[0] + 256 * scroll_x[1]);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 1, 0);
	gridiron_draw_led(machine, bitmap, cliprect, led0, 0);
	gridiron_draw_led(machine, bitmap, cliprect, led1, 1);
	return 0;
}
