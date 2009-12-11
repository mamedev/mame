#include "driver.h"
#include "includes/gunsmoke.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Gunsmoke has three 256x4 palette PROMs (one per gun) and a lot ;-) of
  256x4 lookup table PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( gunsmoke )
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

	/* characters use colors 0x40-0x4f */
	for (i = 0; i < 0x80; i++)
	{
		UINT8 ctabentry = color_prom[i] | 0x40;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* background tiles use colors 0-0x3f */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] | ((color_prom[i + 0x100] & 0x03) << 4);
		colortable_entry_set_value(machine->colortable, i - 0x80, ctabentry);
	}

	/* sprites use colors 0x80-0xff */
	for (i = 0x300; i < 0x400; i++)
	{
		UINT8 ctabentry = color_prom[i] | ((color_prom[i + 0x100] & 0x07) << 4) | 0x80;
		colortable_entry_set_value(machine->colortable, i - 0x180, ctabentry);
	}
}

WRITE8_HANDLER( gunsmoke_videoram_w )
{
	gunsmoke_state *state = (gunsmoke_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( gunsmoke_colorram_w )
{
	gunsmoke_state *state = (gunsmoke_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( gunsmoke_c804_w )
{
	gunsmoke_state *state = (gunsmoke_state *)space->machine->driver_data;

	/* bits 0 and 1 are for coin counters */
	coin_counter_w(space->machine, 1, data & 0x01);
	coin_counter_w(space->machine, 0, data & 0x02);

	/* bits 2 and 3 select the ROM bank */
	memory_set_bank(space->machine, "bank1", (data & 0x0c) >> 2);

	/* bit 5 resets the sound CPU? - we ignore it */

	/* bit 6 flips screen */
	flip_screen_set(space->machine, data & 0x40);

	/* bit 7 enables characters? */
	state->chon = data & 0x80;
}

WRITE8_HANDLER( gunsmoke_d806_w )
{
	gunsmoke_state *state = (gunsmoke_state *)space->machine->driver_data;

	/* bits 0-2 select the sprite 3 bank */
	state->sprite3bank = data & 0x07;

	/* bit 4 enables bg 1? */
	state->bgon = data & 0x10;

	/* bit 5 enables sprites? */
	state->objon = data & 0x20;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *tilerom = memory_region(machine, "gfx4");

	int offs = tile_index * 2;
	int attr = tilerom[offs + 1];
	int code = tilerom[offs] + ((attr & 0x01) << 8);
	int color = (attr & 0x3c) >> 2;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	gunsmoke_state *state = (gunsmoke_state *)machine->driver_data;
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + ((attr & 0xe0) << 2);
	int color = attr & 0x1f;

	tileinfo->group = color;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( gunsmoke )
{
	gunsmoke_state *state = (gunsmoke_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols,  32, 32, 2048, 8);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,  8, 8, 32, 32);

	colortable_configure_tilemap_groups(machine->colortable, state->fg_tilemap, machine->gfx[0], 0x4f);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	gunsmoke_state *state = (gunsmoke_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = state->spriteram_size - 32; offs >= 0; offs -= 32)
	{
		int attr = spriteram[offs + 1];
		int bank = (attr & 0xc0) >> 6;
		int code = spriteram[offs];
		int color = attr & 0x0f;
		int flipx = 0;
		int flipy = attr & 0x10;
		int sx = spriteram[offs + 3] - ((attr & 0x20) << 3);
		int sy = spriteram[offs + 2];

		if (bank == 3)
			bank += state->sprite3bank;

		code += 256 * bank;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine->gfx[2], code, color, flipx, flipy, sx, sy, 0);
	}
}

VIDEO_UPDATE( gunsmoke )
{
	gunsmoke_state *state = (gunsmoke_state *)screen->machine->driver_data;
	tilemap_set_scrollx(state->bg_tilemap, 0, state->scrollx[0] + 256 * state->scrollx[1]);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scrolly[0]);

	if (state->bgon)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	else
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	if (state->objon)
		draw_sprites(screen->machine, bitmap, cliprect);

	if (state->chon)
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	return 0;
}
