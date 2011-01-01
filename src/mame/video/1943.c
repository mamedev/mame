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

#include "emu.h"
#include "includes/1943.h"

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
	_1943_state *state = space->machine->driver_data<_1943_state>();

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( c1943_colorram_w )
{
	_1943_state *state = space->machine->driver_data<_1943_state>();

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( c1943_c804_w )
{
	_1943_state *state = space->machine->driver_data<_1943_state>();
	int bank;

	/* bits 0 and 1 are coin counters */
	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);

	/* bits 2, 3 and 4 select the ROM bank */
	bank = data & 0x1c;
	memory_set_bank(space->machine, "bank1", bank);
	memory_set_bank(space->machine, "bank2", bank);
	memory_set_bank(space->machine, "bank3", bank);
	memory_set_bank(space->machine, "bank4", bank);

	/* bit 5 resets the sound CPU - we ignore it */

	/* bit 6 flips screen */
	flip_screen_set(space->machine, data & 0x40);

	/* bit 7 enables characters */
	state->char_on = data & 0x80;
}

WRITE8_HANDLER( c1943_d806_w )
{
	_1943_state *state = space->machine->driver_data<_1943_state>();

	/* bit 4 enables bg 1 */
	state->bg1_on = data & 0x10;

	/* bit 5 enables bg 2 */
	state->bg2_on = data & 0x20;

	/* bit 6 enables sprites */
	state->obj_on = data & 0x40;
}

static TILE_GET_INFO( c1943_get_bg2_tile_info )
{
	UINT8 *tilerom = machine->region("gfx5")->base() + 0x8000;

	int offs = tile_index * 2;
	int attr = tilerom[offs + 1];
	int code = tilerom[offs];
	int color = (attr & 0x3c) >> 2;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	SET_TILE_INFO(2, code, color, flags);
}

static TILE_GET_INFO( c1943_get_bg_tile_info )
{
	UINT8 *tilerom = machine->region("gfx5")->base();

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
	_1943_state *state = machine->driver_data<_1943_state>();
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + ((attr & 0xe0) << 3);
	int color = attr & 0x1f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( 1943 )
{
	_1943_state *state = machine->driver_data<_1943_state>();
	state->bg2_tilemap = tilemap_create(machine, c1943_get_bg2_tile_info, tilemap_scan_cols, 32, 32, 2048, 8);
	state->bg_tilemap = tilemap_create(machine, c1943_get_bg_tile_info, tilemap_scan_cols, 32, 32, 2048, 8);
	state->fg_tilemap = tilemap_create(machine, c1943_get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	colortable_configure_tilemap_groups(machine->colortable, state->bg_tilemap, machine->gfx[1], 0x0f);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	state_save_register_global(machine, state->char_on);
	state_save_register_global(machine, state->obj_on);
	state_save_register_global(machine, state->bg1_on);
	state_save_register_global(machine, state->bg2_on);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	_1943_state *state = machine->driver_data<_1943_state>();
	int offs;

	for (offs = state->spriteram_size - 32; offs >= 0; offs -= 32)
	{
		int attr = state->spriteram[offs + 1];
		int code = state->spriteram[offs] + ((attr & 0xe0) << 3);
		int color = attr & 0x0f;
		int sx = state->spriteram[offs + 3] - ((attr & 0x10) << 4);
		int sy = state->spriteram[offs + 2];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		// the priority is actually selected by bit 3 of BMPROM.07
		if (priority)
		{
			if (color != 0x0a && color != 0x0b)
				drawgfx_transpen(bitmap, cliprect, machine->gfx[3], code, color, flip_screen_get(machine), flip_screen_get(machine), sx, sy, 0);
		}
		else
		{
			if (color == 0x0a || color == 0x0b)
				drawgfx_transpen(bitmap, cliprect, machine->gfx[3], code, color, flip_screen_get(machine), flip_screen_get(machine), sx, sy, 0);
		}
	}
}

VIDEO_UPDATE( 1943 )
{
	_1943_state *state = screen->machine->driver_data<_1943_state>();
	tilemap_set_scrollx(state->bg2_tilemap, 0, state->bgscrollx[0] + 256 * state->bgscrollx[1]);
	tilemap_set_scrollx(state->bg_tilemap, 0, state->scrollx[0] + 256 * state->scrollx[1]);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scrolly[0]);

	if (state->bg2_on)
		tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 0);
	else
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	if (state->obj_on)
		draw_sprites(screen->machine, bitmap, cliprect, 0);

	if (state->bg1_on)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	if (state->obj_on)
		draw_sprites(screen->machine, bitmap, cliprect, 1);

	if (state->char_on)
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	return 0;
}
