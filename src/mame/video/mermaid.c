#include "driver.h"

static tilemap *bg_tilemap, *fg_tilemap;

UINT8* mermaid_videoram2;
UINT8* mermaid_bg_scrollram;
UINT8* mermaid_fg_scrollram;

static int rougien_gfxbank1, rougien_gfxbank2;

static const rectangle spritevisiblearea =
{
	0*8, 26*8-1,
	2*8, 30*8-1
};

static const rectangle flip_spritevisiblearea =
{
	6*8, 31*8-1,
	2*8, 30*8-1
};

PALETTE_INIT( mermaid )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x41);

	for (i = 0; i < 0x40; i++)
	{
		int r = 0x21 * BIT(color_prom[i], 0) + 0x47 * BIT(color_prom[i], 1) + 0x97 * BIT(color_prom[i], 2);
		int g = 0x21 * BIT(color_prom[i], 3) + 0x47 * BIT(color_prom[i], 4) + 0x97 * BIT(color_prom[i], 5);
		int b =                                0x47 * BIT(color_prom[i], 6) + 0x97 * BIT(color_prom[i], 7);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* blue background */
	colortable_palette_set_color(machine->colortable, 0x40, MAKE_RGB(0, 0, 0xff));

	/* char/sprite palette */
	for (i = 0; i < 0x40; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* background palette */
	colortable_entry_set_value(machine->colortable, 0x40, 0x20);
	colortable_entry_set_value(machine->colortable, 0x41, 0x21);
	colortable_entry_set_value(machine->colortable, 0x42, 0x40);
	colortable_entry_set_value(machine->colortable, 0x43, 0x21);
}

WRITE8_HANDLER( mermaid_videoram2_w )
{
	mermaid_videoram2[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( mermaid_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( mermaid_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( mermaid_flip_screen_x_w )
{
	flip_screen_x_set(data & 0x01);
}

WRITE8_HANDLER( mermaid_flip_screen_y_w )
{
	flip_screen_y_set(data & 0x01);
}

WRITE8_HANDLER( mermaid_bg_scroll_w )
{
	mermaid_bg_scrollram[offset] = data;
	tilemap_set_scrolly(bg_tilemap, offset, data);
}

WRITE8_HANDLER( mermaid_fg_scroll_w )
{
	mermaid_fg_scrollram[offset] = data;
	tilemap_set_scrolly(fg_tilemap, offset, data);
}

WRITE8_HANDLER( rougien_gfxbankswitch1_w )
{
	rougien_gfxbank1 = data & 0x01;
}

WRITE8_HANDLER( rougien_gfxbankswitch2_w )
{
	rougien_gfxbank2 = data & 0x01;
}

READ8_HANDLER( mermaid_collision_r )
{
	/*
        collision register active LO
        Bit 0
        Bit 1 - Sprite - Foreground
        Bit 2 - Sprite - Stream
        Bit 3
        Bit 4
        Bit 5
        Bit 6
        Bit 7
    */

	return 0;	// not implemented
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = mermaid_videoram2[tile_index];
	int sx = tile_index % 32;
	int color = (sx >= 26) ? 0 : 1;

	SET_TILE_INFO(2, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + ((attr & 0x30) << 4);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	code |= rougien_gfxbank1 * 0x2800;
	code |= rougien_gfxbank2 * 0x2400;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( mermaid )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(bg_tilemap, 32);

	fg_tilemap = tilemap_create(get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(fg_tilemap, 32);
	tilemap_set_transparent_pen(fg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 2];
		int bank = (attr & 0x30) >> 4;
		int code = (spriteram[offs] & 0x3f) | (bank << 6);
		int color = attr & 0x0f;
		int flipx = spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;
		int sx = spriteram[offs + 3] + 1;
		int sy = 240 - spriteram[offs + 1];

		code |= rougien_gfxbank1 * 0x2800;
		code |= rougien_gfxbank2 * 0x2400;

		if (flip_screen_x_get())
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y_get())
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		drawgfx(bitmap, machine->gfx[1], code, color, flipx, flipy, sx, sy,
			(flip_screen_x_get() ? &flip_spritevisiblearea : &spritevisiblearea),
			TRANSPARENCY_PEN, 0);
	}
}

VIDEO_UPDATE( mermaid )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
