/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/mikie.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mikie has three 256x4 palette PROMs (one per gun) and two 256x4 lookup
  table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( mikie )
{
	static const int resistances[4] = { 2200, 1000, 470, 220 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, rweights, 470, 0,
			4, resistances, gweights, 470, 0,
			4, resistances, bweights, 470, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
		r = combine_4_weights(rweights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = combine_4_weights(gweights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table,*/
	color_prom += 0x300;

	/* characters use colors 0x10-0x1f of each 0x20 color bank, while sprites use colors 0-0x0f */
	for (i = 0; i < 0x200; i++)
	{
		int j;

		for (j = 0; j < 8; j++)
		{
			UINT8 ctabentry = (j << 5) | ((~i & 0x100) >> 4) | (color_prom[i] & 0x0f);
			colortable_entry_set_value(machine->colortable, ((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

WRITE8_HANDLER( mikie_videoram_w )
{
	mikie_state *state = space->machine->driver_data<mikie_state>();

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( mikie_colorram_w )
{
	mikie_state *state = space->machine->driver_data<mikie_state>();

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( mikie_palettebank_w )
{
	mikie_state *state = space->machine->driver_data<mikie_state>();

	if (state->palettebank != (data & 0x07))
	{
		state->palettebank = data & 0x07;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( mikie_flipscreen_w )
{
	if (flip_screen_get(space->machine) != (data & 0x01))
	{
		flip_screen_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mikie_state *state = machine->driver_data<mikie_state>();
	int code = state->videoram[tile_index] + ((state->colorram[tile_index] & 0x20) << 3);
	int color = (state->colorram[tile_index] & 0x0f) + 16 * state->palettebank;
	int flags = ((state->colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((state->colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);
	if (state->colorram[tile_index] & 0x10)
		tileinfo->category = 1;
	else
		tileinfo->category = 0;

	SET_TILE_INFO(0, code, color, flags);


}

VIDEO_START( mikie )
{
	mikie_state *state = machine->driver_data<mikie_state>();
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	mikie_state *state = machine->driver_data<mikie_state>();
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int gfxbank = (spriteram[offs + 2] & 0x40) ? 2 : 1;
		int code = (spriteram[offs + 2] & 0x3f) + ((spriteram[offs + 2] & 0x80) >> 1) + ((spriteram[offs] & 0x40) << 1);
		int color = (spriteram[offs] & 0x0f) + 16 * state->palettebank;
		int sx = spriteram[offs + 3];
		int sy = 244 - spriteram[offs + 1];
		int flipx = ~spriteram[offs] & 0x10;
		int flipy = spriteram[offs] & 0x20;

		if (flip_screen_get(machine))
		{
			sy = 242 - sy;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect,
			machine->gfx[gfxbank],
			code, color,
			flipx,flipy,
			sx,sy, 0);
	}
}

VIDEO_UPDATE( mikie )
{
	mikie_state *state = screen->machine->driver_data<mikie_state>();
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_CATEGORY(0), 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_CATEGORY(1), 0);
	return 0;
}
