#include "driver.h"

UINT8 *jailbrek_scroll_x;
UINT8 *jailbrek_scroll_dir;

static tilemap *bg_tilemap;

PALETTE_INIT( jailbrek )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int r = pal4bit(color_prom[i + 0x00] >> 0);
		int g = pal4bit(color_prom[i + 0x00] >> 4);
		int b = pal4bit(color_prom[i + 0x20] >> 0);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x40;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE8_HANDLER( jailbrek_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( jailbrek_colorram_w )
{
	space->machine->generic.colorram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = machine->generic.colorram.u8[tile_index];
	int code = machine->generic.videoram.u8[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( jailbrek )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 64, 32);
	tilemap_set_scrolldx(bg_tilemap, 0, 396 - 256);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int i;

	for (i = 0; i < machine->generic.spriteram_size; i += 4)
	{
		int attr = spriteram[i + 1];	// attributes = ?tyxcccc
		int code = spriteram[i] + ((attr & 0x40) << 2);
		int color = attr & 0x0f;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[i + 2] - ((attr & 0x80) << 1);
		int sy = spriteram[i + 3];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy,
			sx, sy,
			colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 0));
	}
}

VIDEO_UPDATE( jailbrek )
{
	int i;

	// added support for vertical scrolling (credits).  23/1/2002  -BR
	// bit 2 appears to be horizontal/vertical scroll control
	if (jailbrek_scroll_dir[0] & 0x04)
	{
		tilemap_set_scroll_cols(bg_tilemap, 32);
		tilemap_set_scroll_rows(bg_tilemap, 1);
		tilemap_set_scrollx(bg_tilemap, 0, 0);

		for (i = 0; i < 32; i++)
			tilemap_set_scrolly(bg_tilemap, i, ((jailbrek_scroll_x[i + 32] << 8) + jailbrek_scroll_x[i]));
	}
	else
	{
		tilemap_set_scroll_rows(bg_tilemap, 32);
		tilemap_set_scroll_cols(bg_tilemap, 1);
		tilemap_set_scrolly(bg_tilemap, 0, 0);

		for (i = 0; i < 32; i++)
			tilemap_set_scrollx(bg_tilemap, i, ((jailbrek_scroll_x[i + 32] << 8) + jailbrek_scroll_x[i]));
	}

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
