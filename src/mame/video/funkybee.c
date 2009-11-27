/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "funkybee.h"

PALETTE_INIT( funkybee )
{
	int i;

	/* first, the character/sprite palette */
	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
		color_prom++;
	}
}

WRITE8_HANDLER( funkybee_videoram_w )
{
	funkybee_state *state = (funkybee_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( funkybee_colorram_w )
{
	funkybee_state *state = (funkybee_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( funkybee_gfx_bank_w )
{
	funkybee_state *state = (funkybee_state *)space->machine->driver_data;
	if (state->gfx_bank != (data & 0x01))
	{
		state->gfx_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( funkybee_scroll_w )
{
	funkybee_state *state = (funkybee_state *)space->machine->driver_data;
	tilemap_set_scrollx(state->bg_tilemap, 0, flip_screen_get(space->machine) ? -data : data);
}

WRITE8_HANDLER( funkybee_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x01);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	funkybee_state *state = (funkybee_state *)machine->driver_data;
	int code = state->videoram[tile_index] + ((state->colorram[tile_index] & 0x80) << 1);
	int color = state->colorram[tile_index] & 0x03;

	SET_TILE_INFO(state->gfx_bank, code, color, 0);
}

static TILEMAP_MAPPER( funkybee_tilemap_scan )
{
	/* logical (col,row) -> memory offset */
	return 256 * row + col;
}

VIDEO_START( funkybee )
{
	funkybee_state *state = (funkybee_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, funkybee_tilemap_scan, 8, 8, 32, 32);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	funkybee_state *state = (funkybee_state *)machine->driver_data;
	int offs;

	for (offs = 0x0f; offs >= 0; offs--)
	{
		int offs2 = offs + 0x1e00;
		int attr = state->videoram[offs2];
		int code = (attr >> 2) | ((attr & 2) << 5);
		int color = state->colorram[offs2 + 0x10];
		int flipx = 0;
		int flipy = attr & 0x01;
		int sx = state->videoram[offs2 + 0x10];
		int sy = 224 - state->colorram[offs2];

		if (flip_screen_get(machine))
		{
			sy += 32;
			flipx = !flipx;
		}

		drawgfx_transpen(bitmap,cliprect, machine->gfx[2 + state->gfx_bank],
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

static void draw_columns( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	funkybee_state *state = (funkybee_state *)machine->driver_data;
	int offs;

	for (offs = 0x1f; offs >= 0; offs--)
	{
		int const flip = flip_screen_get(machine);
		int code = state->videoram[0x1c00 + offs];
		int color = state->colorram[0x1f10] & 0x03;
		int sx = flip ? state->videoram[0x1f1f] : state->videoram[0x1f10];
		int sy = offs * 8;

		if (flip)
			sy = 248 - sy;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[state->gfx_bank],
				code, color,
				flip, flip,
				sx, sy,0);

		code = state->videoram[0x1d00 + offs];
		color = state->colorram[0x1f11] & 0x03;
		sx = flip ? state->videoram[0x1f1e] : state->videoram[0x1f11];
		sy = offs * 8;

		if (flip)
			sy = 248 - sy;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[state->gfx_bank],
				code, color,
				flip, flip,
				sx, sy,0);
	}
}

VIDEO_UPDATE( funkybee )
{
	funkybee_state *state = (funkybee_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	draw_columns(screen->machine, bitmap, cliprect);
	return 0;
}
