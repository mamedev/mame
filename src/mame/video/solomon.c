#include "emu.h"
#include "includes/solomon.h"

WRITE8_HANDLER( solomon_videoram_w )
{
	solomon_state *state = space->machine->driver_data<solomon_state>();

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( solomon_colorram_w )
{
	solomon_state *state = space->machine->driver_data<solomon_state>();

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( solomon_videoram2_w )
{
	solomon_state *state = space->machine->driver_data<solomon_state>();

	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( solomon_colorram2_w )
{
	solomon_state *state = space->machine->driver_data<solomon_state>();

	state->colorram2[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( solomon_flipscreen_w )
{
	if (flip_screen_get(space->machine) != (data & 0x01))
	{
		flip_screen_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	solomon_state *state = machine->driver_data<solomon_state>();
	int attr = state->colorram2[tile_index];
	int code = state->videoram2[tile_index] + 256 * (attr & 0x07);
	int color = ((attr & 0x70) >> 4);
	int flags = ((attr & 0x80) ? TILE_FLIPX : 0) | ((attr & 0x08) ? TILE_FLIPY : 0);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	solomon_state *state = machine->driver_data<solomon_state>();
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + 256 * (attr & 0x07);
	int color = (attr & 0x70) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( solomon )
{
	solomon_state *state = machine->driver_data<solomon_state>();

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	solomon_state *state = machine->driver_data<solomon_state>();
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = machine->generic.spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int code = spriteram[offs] + 16 * (spriteram[offs + 1] & 0x10);
		int color = (spriteram[offs + 1] & 0x0e) >> 1;
		int flipx = spriteram[offs + 1] & 0x40;
		int flipy =	spriteram[offs + 1] & 0x80;
		int sx = spriteram[offs + 3];
		int sy = 241 - spriteram[offs + 2];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 242 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect,
			machine->gfx[2],
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

VIDEO_UPDATE( solomon )
{
	solomon_state *state = screen->machine->driver_data<solomon_state>();
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
