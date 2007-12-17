/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *exedexes_bg_scroll;

UINT8 *exedexes_nbg_yscroll;
UINT8 *exedexes_nbg_xscroll;

static int chon,objon,sc1on,sc2on;

#define TileMap(offs) (memory_region(REGION_GFX5)[offs])
#define BackTileMap(offs) (memory_region(REGION_GFX5)[offs+0x4000])

static tilemap *bg_tilemap, *fg_tilemap, *tx_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Exed Exes has three 256x4 palette PROMs (one per gun), three 256x4 lookup
  table PROMs (one for characters, one for sprites, one for background tiles)
  and one 256x4 sprite palette bank selector PROM.

  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( exedexes )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->drv->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[2*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[2*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[2*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[2*machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	color_prom += 2*machine->drv->total_colors;
	/* color_prom now points to the beginning of the lookup table */

	/* characters use colors 192-207 */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = (*color_prom++) + 192;

	/* 32x32 tiles use colors 0-15 */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = (*color_prom++);

	/* 16x16 tiles use colors 64-79 */
	for (i = 0;i < TOTAL_COLORS(2);i++)
		COLOR(2,i) = (*color_prom++) + 64;

	/* sprites use colors 128-191 in four banks */
	for (i = 0;i < TOTAL_COLORS(3);i++)
	{
		COLOR(3,i) = color_prom[0] + 128 + 16 * color_prom[256];
		color_prom++;
	}
}

WRITE8_HANDLER( exedexes_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap, offset);
}

WRITE8_HANDLER( exedexes_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap, offset);
}

WRITE8_HANDLER( exedexes_c804_w )
{
	/* bits 0 and 1 are coin counters */
	coin_counter_w(0, data & 0x01);
	coin_counter_w(1, data & 0x02);

	coin_lockout_w(0, data & 0x04);
	coin_lockout_w(1, data & 0x08);

	/* bit 7 is text enable */
	chon = data & 0x80;

	/* other bits seem to be unused */
}

WRITE8_HANDLER( exedexes_gfxctrl_w )
{
	/* bit 4 is bg enable */
	sc2on = data & 0x10;

	/* bit 5 is fg enable */
	sc1on = data & 0x20;

	/* bit 6 is sprite enable */
	objon = data & 0x40;

	/* other bits seem to be unused */
}


static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *tilerom = memory_region(REGION_GFX5);

	int attr = tilerom[tile_index];
	int code = attr & 0x3f;
	int color = tilerom[tile_index + (8 * 8)];
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = memory_region(REGION_GFX5)[tile_index];

	SET_TILE_INFO(2, code, 0, 0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	int code = videoram[tile_index] + 2 * (colorram[tile_index] & 0x80);
	int color = colorram[tile_index] & 0x3f;

	SET_TILE_INFO(0, code, color, 0);
}

static TILEMAP_MAPPER( exedexes_bg_tilemap_scan )
{
	/* logical (col,row) -> memory offset */
	return ((col * 32 & 0xe0) >> 5) + ((row * 32 & 0xe0) >> 2) + ((col * 32 & 0x3f00) >> 1) + 0x4000;
}

static TILEMAP_MAPPER( exedexes_fg_tilemap_scan )
{
	/* logical (col,row) -> memory offset */
	return ((col * 16 & 0xf0) >> 4) + (row * 16 & 0xf0) + (col * 16 & 0x700) + ((row * 16 & 0x700) << 3);
}

VIDEO_START( exedexes )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, exedexes_bg_tilemap_scan,
		TILEMAP_TYPE_PEN, 32, 32, 64, 64);

	fg_tilemap = tilemap_create(get_fg_tile_info, exedexes_fg_tilemap_scan,
		TILEMAP_TYPE_PEN, 16, 16, 128, 128);

	tx_tilemap = tilemap_create(get_tx_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_COLORTABLE, 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
	tilemap_set_transparent_pen(tx_tilemap, 207);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int priority)
{
	int offs;

	if (!objon) return;

	priority = priority ? 0x40 : 0x00;

	for (offs = spriteram_size - 32;offs >= 0;offs -= 32)
	{
		if ((buffered_spriteram[offs + 1] & 0x40) == priority)
		{
			int code,color,flipx,flipy,sx,sy;

			code = buffered_spriteram[offs];
			color = buffered_spriteram[offs + 1] & 0x0f;
			flipx = buffered_spriteram[offs + 1] & 0x10;
			flipy = buffered_spriteram[offs + 1] & 0x20;
			sx = buffered_spriteram[offs + 3] - ((buffered_spriteram[offs + 1] & 0x80) << 1);
			sy = buffered_spriteram[offs + 2];

			drawgfx(bitmap,machine->gfx[3],
					code,
					color,
					flipx,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_PEN,0);
		}
	}
}

VIDEO_UPDATE( exedexes )
{
	if (sc2on)
	{
		tilemap_set_scrollx(bg_tilemap, 0, ((exedexes_bg_scroll[1]) << 8) + exedexes_bg_scroll[0]);
		tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	}
	else
	{
		fillbitmap(bitmap, machine->pens[0], cliprect);
	}

	draw_sprites(machine, bitmap, cliprect, 1);

	if (sc1on)
	{
		tilemap_set_scrollx(fg_tilemap, 0, ((exedexes_nbg_yscroll[1]) << 8) + exedexes_nbg_yscroll[0]);
		tilemap_set_scrolly(fg_tilemap, 0, ((exedexes_nbg_xscroll[1]) << 8) + exedexes_nbg_xscroll[0]);
		tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	}

	draw_sprites(machine, bitmap, cliprect, 0);

	if (chon)
	{
		tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 0);
	}
	return 0;
}

VIDEO_EOF( exedexes )
{
	buffer_spriteram_w(0,0);
}
