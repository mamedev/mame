#include "driver.h"

#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

UINT8 *c1943_scrollx;
UINT8 *c1943_scrolly;
UINT8 *c1943_bgscrollx;

static int chon, objon, sc1on, sc2on;

static tilemap *bg2_tilemap, *bg_tilemap, *fg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  1943 has three 256x4 palette PROMs (one per gun) and a lot ;-) of 256x4
  lookup table PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( 1943 )
{
	int i;

	for (i = 0; i < machine->drv->total_colors; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;

		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + machine->drv->total_colors] >> 3) & 0x01;

		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*machine->drv->total_colors] >> 3) & 0x01;

		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}

	color_prom += 3*machine->drv->total_colors;
	/* color_prom now points to the beginning of the lookup table */

	/* characters use colors 64-79 */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = *(color_prom++) + 64;
	color_prom += 128;	/* skip the bottom half of the PROM - not used */

	/* foreground tiles use colors 0-63 */
	for (i = 0;i < TOTAL_COLORS(1);i++)
	{
		/* color 0 MUST map to pen 0 in order for transparency to work */
		if (i % machine->gfx[1]->color_granularity == 0)
			COLOR(1,i) = 0;
		else
			COLOR(1,i) = color_prom[0] + 16 * (color_prom[256] & 0x03);
		color_prom++;
	}
	color_prom += TOTAL_COLORS(1);

	/* background tiles use colors 0-63 */
	for (i = 0;i < TOTAL_COLORS(2);i++)
	{
		COLOR(2,i) = color_prom[0] + 16 * (color_prom[256] & 0x03);
		color_prom++;
	}
	color_prom += TOTAL_COLORS(2);

	/* sprites use colors 128-255 */
	/* bit 3 of BMPROM.07 selects priority over the background, but we handle */
	/* it differently for speed reasons */
	for (i = 0;i < TOTAL_COLORS(3);i++)
	{
		COLOR(3,i) = color_prom[0] + 16 * (color_prom[256] & 0x07) + 128;
		color_prom++;
	}
	color_prom += TOTAL_COLORS(3);
}

WRITE8_HANDLER( c1943_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( c1943_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( c1943_c804_w )
{
	int bankaddress;
	UINT8 *RAM = memory_region(REGION_CPU1);

	/* bits 0 and 1 are coin counters */
	coin_counter_w(0, data & 0x01);
	coin_counter_w(1, data & 0x02);

	/* bits 2, 3 and 4 select the ROM bank */
	bankaddress = 0x10000 + (data & 0x1c) * 0x1000;
	memory_set_bankptr(1, &RAM[bankaddress]);

	/* bit 5 resets the sound CPU - we ignore it */

	/* bit 6 flips screen */
	flip_screen_set(data & 0x40);

	/* bit 7 enables characters */
	chon = data & 0x80;
}

WRITE8_HANDLER( c1943_d806_w )
{
	/* bit 4 enables bg 1 */
	sc1on = data & 0x10;

	/* bit 5 enables bg 2 */
	sc2on = data & 0x20;

	/* bit 6 enables sprites */
	objon = data & 0x40;
}

static TILE_GET_INFO( c1943_get_bg2_tile_info )
{
	UINT8 *tilerom = memory_region(REGION_GFX5) + 0x8000;

	int offs = tile_index * 2;
	int attr = tilerom[offs + 1];
	int code = tilerom[offs];
	int color = (attr & 0x3c) >> 2;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	SET_TILE_INFO(2, code, color, flags);
}

static TILE_GET_INFO( c1943_get_bg_tile_info )
{
	UINT8 *tilerom = memory_region(REGION_GFX5);

	int offs = tile_index * 2;
	int attr = tilerom[offs + 1];
	int code = tilerom[offs] + ((attr & 0x01) << 8);
	int color = (attr & 0x3c) >> 2;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( c1943_get_fg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0xe0) << 3);
	int color = attr & 0x1f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( 1943 )
{
	bg2_tilemap = tilemap_create(c1943_get_bg2_tile_info, tilemap_scan_cols,
		TILEMAP_TYPE_PEN, 32, 32, 2048, 8);

	bg_tilemap = tilemap_create(c1943_get_bg_tile_info, tilemap_scan_cols,
		TILEMAP_TYPE_PEN, 32, 32, 2048, 8);

	fg_tilemap = tilemap_create(c1943_get_fg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_transparent_pen(bg_tilemap, 0);
	tilemap_set_transparent_pen(fg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int priority)
{
	int offs;

	for (offs = spriteram_size - 32; offs >= 0; offs -= 32)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs] + ((attr & 0xe0) << 3);
		int color = attr & 0x0f;
		int sx = spriteram[offs + 3] - ((attr & 0x10) << 4);
		int sy = spriteram[offs + 2];

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		// the priority is actually selected by bit 3 of BMPROM.07
		if (priority)
		{
			if (color != 0x0a && color != 0x0b)
			{
				drawgfx(bitmap, machine->gfx[3], code, color, flip_screen, flip_screen,
					sx, sy, cliprect, TRANSPARENCY_PEN, 0);
			}
		}
		else
		{
			if (color == 0x0a || color == 0x0b)
			{
				drawgfx(bitmap, machine->gfx[3], code, color, flip_screen, flip_screen,
					sx, sy, cliprect, TRANSPARENCY_PEN, 0);
			}
		}
	}
}

VIDEO_UPDATE( 1943 )
{
	tilemap_set_scrollx(bg2_tilemap, 0, c1943_bgscrollx[0] + 256 * c1943_bgscrollx[1]);
	tilemap_set_scrollx(bg_tilemap, 0, c1943_scrollx[0] + 256 * c1943_scrollx[1]);
	tilemap_set_scrolly(bg_tilemap, 0, c1943_scrolly[0]);

	if (sc2on)
		tilemap_draw(bitmap, cliprect, bg2_tilemap, 0, 0);
	else
		fillbitmap(bitmap, get_black_pen(machine), cliprect);

	if (objon) draw_sprites(machine, bitmap, cliprect, 0);
	if (sc1on) tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	if (objon) draw_sprites(machine, bitmap, cliprect, 1);
	if (chon)  tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
