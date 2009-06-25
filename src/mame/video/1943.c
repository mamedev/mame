/***************************************************************************

1943 Video Hardware

This board handles tile/tile and tile/sprite priority with a PROM. Its
working is hardcoded in the driver.

The PROM have address inputs wired as follows:

A0 bg (SCR) opaque
A1 bit 2 of sprite (OBJ) attribute (guess)
A2 bit 3 of sprite (OBJ) attribute (guess)
A3 sprite (OBJ) opaque
A4 fg (CHAR) opaque
A5 wired to mass
A6 wired to mass
A7 wired to mass

2 bits of the output selects the active layer, it can be:
(output & 0x03)
0 bg2 (SCR2)
1 bg (SCR)
2 sprite (OBJ)
3 fg (CHAR)

other 2 bits (output & 0x0c) unknown

***************************************************************************/

#include "driver.h"

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

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters use colors 0x40-0x4f */
	for (i = 0x00; i < 0x80; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x40;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* foreground tiles use colors 0x00-0x3f */
	for (i = 0x80; i < 0x180; i++)
	{
		UINT8 ctabentry = ((color_prom[0x200 + (i - 0x080)] & 0x03) << 4) |
				  		  ((color_prom[0x100 + (i - 0x080)] & 0x0f) << 0);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* background tiles also use colors 0x00-0x3f */
	for (i = 0x180; i < 0x280; i++)
	{
		UINT8 ctabentry = ((color_prom[0x400 + (i - 0x180)] & 0x03) << 4) |
				  		  ((color_prom[0x300 + (i - 0x180)] & 0x0f) << 0);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites use colors 0x80-0xff
       bit 3 of BMPROM.07 selects priority over the background,
       but we handle it differently for speed reasons */
	for (i = 0x280; i < 0x380; i++)
	{
		UINT8 ctabentry = ((color_prom[0x600 + (i - 0x280)] & 0x07) << 4) |
				  		  ((color_prom[0x500 + (i - 0x280)] & 0x0f) << 0) | 0x80;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
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
	UINT8 *RAM = memory_region(space->machine, "maincpu");

	/* bits 0 and 1 are coin counters */
	coin_counter_w(0, data & 0x01);
	coin_counter_w(1, data & 0x02);

	/* bits 2, 3 and 4 select the ROM bank */
	bankaddress = 0x10000 + (data & 0x1c) * 0x1000;
	memory_set_bankptr(space->machine, 1, &RAM[bankaddress]);

	/* bit 5 resets the sound CPU - we ignore it */

	/* bit 6 flips screen */
	flip_screen_set(space->machine, data & 0x40);

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
	UINT8 *tilerom = memory_region(machine, "gfx5") + 0x8000;

	int offs = tile_index * 2;
	int attr = tilerom[offs + 1];
	int code = tilerom[offs];
	int color = (attr & 0x3c) >> 2;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	SET_TILE_INFO(2, code, color, flags);
}

static TILE_GET_INFO( c1943_get_bg_tile_info )
{
	UINT8 *tilerom = memory_region(machine, "gfx5");

	int offs = tile_index * 2;
	int attr = tilerom[offs + 1];
	int code = tilerom[offs] + ((attr & 0x01) << 8);
	int color = (attr & 0x3c) >> 2;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	tileinfo->group = color;
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
	bg2_tilemap = tilemap_create(machine, c1943_get_bg2_tile_info, tilemap_scan_cols,
		 32, 32, 2048, 8);

	bg_tilemap = tilemap_create(machine, c1943_get_bg_tile_info, tilemap_scan_cols,
		 32, 32, 2048, 8);

	fg_tilemap = tilemap_create(machine, c1943_get_fg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	colortable_configure_tilemap_groups(machine->colortable, bg_tilemap, machine->gfx[1], 0x0f);
	tilemap_set_transparent_pen(fg_tilemap, 0);

    state_save_register_global(machine, chon);
    state_save_register_global(machine, objon);
    state_save_register_global(machine, sc1on);
    state_save_register_global(machine, sc2on);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority)
{
	int offs;

	for (offs = spriteram_size - 32; offs >= 0; offs -= 32)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs] + ((attr & 0xe0) << 3);
		int color = attr & 0x0f;
		int sx = spriteram[offs + 3] - ((attr & 0x10) << 4);
		int sy = spriteram[offs + 2];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		// the priority is actually selected by bit 3 of BMPROM.07
		if (priority)
		{
			if (color != 0x0a && color != 0x0b)
			{
				drawgfx_transpen(bitmap, cliprect, machine->gfx[3], code, color, flip_screen_get(machine), flip_screen_get(machine),
					sx, sy, 0);
			}
		}
		else
		{
			if (color == 0x0a || color == 0x0b)
			{
				drawgfx_transpen(bitmap, cliprect, machine->gfx[3], code, color, flip_screen_get(machine), flip_screen_get(machine),
					sx, sy, 0);
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
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	if (objon) draw_sprites(screen->machine, bitmap, cliprect, 0);
	if (sc1on) tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	if (objon) draw_sprites(screen->machine, bitmap, cliprect, 1);
	if (chon)  tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
