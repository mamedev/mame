/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *exedexes_bg_scroll;

UINT8 *exedexes_nbg_yscroll;
UINT8 *exedexes_nbg_xscroll;

static int chon,objon,sc1on,sc2on;

#define TileMap(offs) (memory_region(machine, "gfx5")[offs])
#define BackTileMap(offs) (memory_region(machine, "gfx5")[offs+0x4000])

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

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters use colors 0xc0-0xcf */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] | 0xc0;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* 32x32 tiles use colors 0-0x0f */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i];
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* 16x16 tiles use colors 0x40-0x4f */
	for (i = 0x200; i < 0x300; i++)
	{
		UINT8 ctabentry = color_prom[i] | 0x40;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites use colors 0x80-0xbf in four banks */
	for (i = 0x300; i < 0x400; i++)
	{
		UINT8 ctabentry = color_prom[i] | (color_prom[i + 0x100] << 4) | 0x80;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
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
	UINT8 *tilerom = memory_region(machine, "gfx5");

	int attr = tilerom[tile_index];
	int code = attr & 0x3f;
	int color = tilerom[tile_index + (8 * 8)];
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = memory_region(machine, "gfx5")[tile_index];

	SET_TILE_INFO(2, code, 0, 0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	int code = videoram[tile_index] + 2 * (colorram[tile_index] & 0x80);
	int color = colorram[tile_index] & 0x3f;

	tileinfo->group = color;

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
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, exedexes_bg_tilemap_scan,
		 32, 32, 64, 64);

	fg_tilemap = tilemap_create(machine, get_fg_tile_info, exedexes_fg_tilemap_scan,
		 16, 16, 128, 128);

	tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, 0);
	colortable_configure_tilemap_groups(machine->colortable, tx_tilemap, machine->gfx[0], 0xcf);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
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

			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					code,
					color,
					flipx,flipy,
					sx,sy,0);
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
		bitmap_fill(bitmap, cliprect, 0);

	draw_sprites(screen->machine, bitmap, cliprect, 1);

	if (sc1on)
	{
		tilemap_set_scrollx(fg_tilemap, 0, ((exedexes_nbg_yscroll[1]) << 8) + exedexes_nbg_yscroll[0]);
		tilemap_set_scrolly(fg_tilemap, 0, ((exedexes_nbg_xscroll[1]) << 8) + exedexes_nbg_xscroll[0]);
		tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	}

	draw_sprites(screen->machine, bitmap, cliprect, 0);

	if (chon)
		tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 0);

	return 0;
}

VIDEO_EOF( exedexes )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram_w(space, 0, 0);
}
