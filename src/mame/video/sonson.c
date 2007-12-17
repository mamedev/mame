/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Son Son has two 32x8 palette PROMs and two 256x4 lookup table PROMs (one
  for characters, one for sprites).
  The palette PROMs are connected to the RGB output this way:

  I don't know the exact values of the resistors between the PROMs and the
  RGB output. I assumed these values (the same as Commando)
  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

  bit 7 -- unused
        -- unused
        -- unused
        -- unused
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( sonson )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		/* red component */
		bit0 = (color_prom[i + machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + machine->drv->total_colors] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	color_prom += 2*machine->drv->total_colors;
	/* color_prom now points to the beginning of the lookup table */

	/* characters use colors 0-15 */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = *(color_prom++) & 0x0f;

	/* sprites use colors 16-31 */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = (*(color_prom++) & 0x0f) + 0x10;
}

WRITE8_HANDLER( sonson_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( sonson_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( sonson_scroll_w )
{
	int row;

	for (row = 5; row < 32; row++)
	{
		tilemap_set_scrollx(bg_tilemap, row, data);
	}
}

WRITE8_HANDLER( sonson_flipscreen_w )
{
	if (flip_screen != (~data & 0x01))
	{
		flip_screen_set(~data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + 256 * (attr & 0x03);
	int color = attr >> 2;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( sonson )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_scroll_rows(bg_tilemap, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int offs;

	for (offs = spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int code = spriteram[offs + 2] + ((spriteram[offs + 1] & 0x20) << 3);
		int color = spriteram[offs + 1] & 0x1f;
		int flipx = ~spriteram[offs + 1] & 0x40;
		int flipy = ~spriteram[offs + 1] & 0x80;
		int sx = spriteram[offs + 3];
		int sy = spriteram[offs + 0];

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap, machine->gfx[1],
			code, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( sonson )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
