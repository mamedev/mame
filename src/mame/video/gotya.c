#include "driver.h"

UINT8 *gotya_scroll;
UINT8 *gotya_videoram2;

static int scroll_bit_8;

static tilemap *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm using Pac Man resistor values

***************************************************************************/
PALETTE_INIT( gotya )
{
	int i;

	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0; i < machine->drv->total_colors; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */

		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;

		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */

		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;

		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */

		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;

		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));

		color_prom++;
	}

	color_prom += 0x18;
	/* color_prom now points to the beginning of the lookup table */

	/* character lookup table */
	/* sprites use the same color lookup table as characters */

	for (i = 0; i < TOTAL_COLORS(0); i++)
	{
		COLOR(0, i) = *(color_prom++) & 0x07;
	}
}

WRITE8_HANDLER( gotya_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( gotya_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( gotya_video_control_w )
{
	/* bit 0 - scroll bit 8
       bit 1 - flip screen
       bit 2 - sound disable ??? */

	scroll_bit_8 = data & 0x01;

	if (flip_screen != (data & 0x02))
	{
		flip_screen_set(data & 0x02);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index];
	int color = colorram[tile_index] & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

static TILEMAP_MAPPER( tilemap_scan_rows_thehand )
{
	/* logical (col,row) -> memory offset */
	row = 31-row;
	col = 63-col;
	return ((row)*(num_cols>>1)) + (col&31) + ((col>>5)*0x400);
}

VIDEO_START( gotya )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows_thehand,
		TILEMAP_TYPE_PEN, 8, 8, 64, 32);
}

static void draw_status_row(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int sx, int col)
{
	int row;

	if (flip_screen)
	{
		sx = 35 - sx;
	}

	for (row = 29; row >= 0; row--)
	{
		int sy;

		if (flip_screen)
		{
			sy = row;
		}
		else
		{
			sy = 31 - row;
		}

		drawgfx(bitmap,machine->gfx[0],
			gotya_videoram2[row * 32 + col],
			gotya_videoram2[row * 32 + col + 0x10] & 0x0f,
			flip_screen_x, flip_screen_y,
			8 * sx, 8 * sy,
			cliprect,
			TRANSPARENCY_NONE, 0);
	}
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 2; offs < 0x0e; offs += 2)
	{
		int code = spriteram[offs + 0x01] >> 2;
		int color = spriteram[offs + 0x11] & 0x0f;
		int sx = 256 - spriteram[offs + 0x10] + (spriteram[offs + 0x01] & 0x01) * 256;
		int sy = spriteram[offs + 0x00];

		if (flip_screen)
		{
			sy = 240 - sy;
		}

		drawgfx(bitmap,machine->gfx[1],
			code, color,
			flip_screen_x, flip_screen_y,
			sx, sy,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}

static void draw_status(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	draw_status_row(machine, bitmap, cliprect, 0,  1);
	draw_status_row(machine, bitmap, cliprect, 1,  0);
	draw_status_row(machine, bitmap, cliprect, 2,  2);	/* these two are blank, but I dont' know if the data comes */
	draw_status_row(machine, bitmap, cliprect, 33, 13);	/* from RAM or 'hardcoded' into the hardware. Likely the latter */
	draw_status_row(machine, bitmap, cliprect, 35, 14);
	draw_status_row(machine, bitmap, cliprect, 34, 15);
}

VIDEO_UPDATE( gotya )
{
	tilemap_set_scrollx(bg_tilemap, 0, -(*gotya_scroll + (scroll_bit_8 * 256)) - 2 * 8);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	draw_status(machine, bitmap, cliprect);
	return 0;
}
