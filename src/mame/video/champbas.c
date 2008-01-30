/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

static UINT8 gfxbank;
static UINT8 palette_bank;

static tilemap *bg_tilemap;
static UINT8 *pen_mask;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( champbas )
{
	int i;

	pen_mask = auto_malloc(128 * sizeof(UINT8));
	memset(pen_mask, 0, 128 * sizeof(UINT8));

	for (i = 0; i < machine->drv->total_colors; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		UINT8 pen = ((i & 0x100) >> 4) | (color_prom[0x20 + (i & 0xff)] & 0x0f);

		/* red component */
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[pen] >> 6) & 0x01;
		bit2 = (color_prom[pen] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));

		/* set the mask, color 0 is transparent */
		if (color_prom[pen] == 0)
			pen_mask[i >> 2] |= (1 << (i & 0x03));
	}
}

WRITE8_HANDLER( champbas_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( champbas_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( champbas_gfxbank_w )
{
	if (gfxbank != (data & 0x01))
	{
		gfxbank = data & 0x01;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( champbas_palette_bank_w )
{
	if (palette_bank != (data & 0x01))
	{
		palette_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( champbas_flipscreen_w )
{
	if (flip_screen != data)
	{
		flip_screen_set(data);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index];
	int color = (palette_bank << 6) | 0x20 | (colorram[tile_index] & 0x1f);

	SET_TILE_INFO(gfxbank, code, color, 0);
}

VIDEO_START( champbas )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size - 2; offs >= 0; offs -= 2)
	{
		int code = spriteram[offs] >> 2;
		int color = (palette_bank << 6) | (spriteram[offs + 1] & 0x3f);
		int flipx = spriteram[offs] & 0x01;
		int flipy = spriteram[offs] & 0x02;
		int sx = ((256 + 16 - spriteram_2[offs + 1]) & 0xff) - 16;
		int sy = spriteram_2[offs] - 16;

		drawgfx(bitmap,
			machine->gfx[2 + gfxbank],
			code, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_PENS, pen_mask[color]);
	}
}

VIDEO_UPDATE( champbas )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
