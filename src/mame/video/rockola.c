/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *rockola_videoram2;
UINT8 *rockola_charram;

static int charbank;
static int backcolor;

static tilemap *bg_tilemap, *fg_tilemap;

static rgb_t palette[64];

#define TOTAL_COLORS(gfxn) (Machine->gfx[gfxn]->total_colors * Machine->gfx[gfxn]->color_granularity)
#define COLOR(gfxn,offs) (Machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs)



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Zarzon has a different PROM layout from the others.

***************************************************************************/
PALETTE_INIT( rockola )
{
	int i;

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

		palette[i] = MAKE_RGB(r, g, b);

		color_prom++;
	}

	backcolor = 0;	/* background color can be changed by the game */

	for (i = 0; i < TOTAL_COLORS(0); i++)
		palette_set_color(machine, COLOR(0, i), palette[i]);

	for (i = 0; i < TOTAL_COLORS(1); i++)
	{
		if (i % 4 == 0)
			palette_set_color(machine, COLOR(1, i), palette[4 * backcolor + 0x20]);
		else
			palette_set_color(machine, COLOR(1, i), palette[i + 0x20]);
	}
}

WRITE8_HANDLER( rockola_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( rockola_videoram2_w )
{
	rockola_videoram2[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( rockola_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( rockola_charram_w )
{
	if (rockola_charram[offset] != data)
	{
		rockola_charram[offset] = data;
		tilemap_mark_all_tiles_dirty(fg_tilemap);
	}
}

WRITE8_HANDLER( rockola_flipscreen_w )
{
	int bank;

	/* bits 0-2 select background color */

	if (backcolor != (data & 7))
	{
		int i;

		backcolor = data & 7;

		for (i = 0;i < 32;i += 4)
			palette_set_color(Machine, COLOR(1, i), palette[4 * backcolor + 0x20]);
	}

	/* bit 3 selects char bank */

	bank = (~data & 0x08) >> 3;

	if (charbank != bank)
	{
		charbank = bank;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	/* bit 7 flips screen */

	if (flip_screen != (data & 0x80))
	{
		flip_screen_set(data & 0x80);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( rockola_scrollx_w )
{
	tilemap_set_scrollx(bg_tilemap, 0, data);
}

WRITE8_HANDLER( rockola_scrolly_w )
{
	tilemap_set_scrolly(bg_tilemap, 0, data);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] + 256 * charbank;
	int color = (colorram[tile_index] & 0x38) >> 3;

	SET_TILE_INFO(1, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = rockola_videoram2[tile_index];
	int color = colorram[tile_index] & 0x07;

	decodechar(machine->gfx[0], code, rockola_charram,
		machine->drv->gfxdecodeinfo[0].gfxlayout);

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( rockola )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}

VIDEO_UPDATE( rockola )
{
	tilemap_draw(bitmap, &machine->screen[0].visarea, bg_tilemap, 0, 0);
	tilemap_draw(bitmap, &machine->screen[0].visarea, fg_tilemap, 0, 0);
	return 0;
}

/* Satan of Saturn */

PALETTE_INIT( satansat )
{
	int i;

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

		palette[i] = MAKE_RGB(r, g, b);

		color_prom++;
	}

	backcolor = 0;	/* background color can be changed by the game */

	for (i = 0; i < TOTAL_COLORS(0); i++)
		palette_set_color(machine, COLOR(0, i), palette[4 * (i % 4) + (i / 4)]);

	for (i = 0; i < TOTAL_COLORS(1); i++)
	{
		if (i % 4 == 0)
			palette_set_color(machine, COLOR(1, i), palette[backcolor + 0x10]);
		else
			palette_set_color(machine, COLOR(1, i), palette[4 * (i % 4) + (i / 4) + 0x10]);
	}
}

WRITE8_HANDLER( satansat_b002_w )
{
	/* bit 0 flips screen */

	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	/* bit 1 enables interrups */
	/* it controls only IRQs, not NMIs. Here I am affecting both, which */
	/* is wrong. */

	interrupt_enable_w(0,data & 0x02);

	/* other bits unused */
}

WRITE8_HANDLER( satansat_backcolor_w )
{
	/* bits 0-1 select background color. Other bits unused. */

	if (backcolor != (data & 0x03))
	{
		int i;

		backcolor = data & 0x03;

		for (i = 0; i < 16; i += 4)
			palette_set_color(Machine, COLOR(1, i), palette[backcolor + 0x10]);
	}
}

static TILE_GET_INFO( satansat_get_bg_tile_info )
{
	int code = videoram[tile_index];
	int color = (colorram[tile_index] & 0x0c) >> 2;

	SET_TILE_INFO(1, code, color, 0);
}

static TILE_GET_INFO( satansat_get_fg_tile_info )
{
	int code = rockola_videoram2[tile_index];
	int color = colorram[tile_index] & 0x03;

	decodechar(machine->gfx[0], code, rockola_charram,
		machine->drv->gfxdecodeinfo[0].gfxlayout);

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( satansat )
{
	bg_tilemap = tilemap_create(satansat_get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	fg_tilemap = tilemap_create(satansat_get_fg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
}
