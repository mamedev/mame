#include "driver.h"
#include "includes/exprraid.h"


WRITE8_HANDLER( exprraid_videoram_w )
{
	exprraid_state *state = (exprraid_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( exprraid_colorram_w )
{
	exprraid_state *state = (exprraid_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( exprraid_flipscreen_w )
{
	if (flip_screen_get(space->machine) != (data & 0x01))
	{
		flip_screen_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( exprraid_bgselect_w )
{
	exprraid_state *state = (exprraid_state *)space->machine->driver_data;
	if (state->bg_index[offset] != data)
	{
		state->bg_index[offset] = data;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}

WRITE8_HANDLER( exprraid_scrollx_w )
{
	exprraid_state *state = (exprraid_state *)space->machine->driver_data;
	tilemap_set_scrollx(state->bg_tilemap, offset, data);
}

WRITE8_HANDLER( exprraid_scrolly_w )
{
	exprraid_state *state = (exprraid_state *)space->machine->driver_data;
	tilemap_set_scrolly(state->bg_tilemap, 0, data);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	exprraid_state *state = (exprraid_state *)machine->driver_data;
	UINT8 *tilerom = memory_region(machine, "gfx4");

	int data, attr, bank, code, color, flags;
	int quadrant = 0, offs;

	int sx = tile_index % 32;
	int sy = tile_index / 32;

	if (sx >= 16) quadrant++;
	if (sy >= 16) quadrant += 2;

	offs = (sy % 16) * 16 + (sx % 16) + (state->bg_index[quadrant] & 0x3f) * 0x100;

	data = tilerom[offs];
	attr = tilerom[offs + 0x4000];
	bank = (2 * (attr & 0x03) + ((data & 0x80) >> 7)) + 2;
	code = data & 0x7f;
	color = (attr & 0x18) >> 3;
	flags = (attr & 0x04) ? TILE_FLIPX : 0;

	tileinfo->category = ((attr & 0x80) ? 1 : 0);

	SET_TILE_INFO(bank, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	exprraid_state *state = (exprraid_state *)machine->driver_data;
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + ((attr & 0x07) << 8);
	int color = (attr & 0x10) >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( exprraid )
{
	exprraid_state *state = (exprraid_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_scroll_rows(state->bg_tilemap, 2);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	exprraid_state *state = (exprraid_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int attr = state->spriteram[offs + 1];
		int code = state->spriteram[offs + 3] + ((attr & 0xe0) << 3);
		int color = (attr & 0x03) + ((attr & 0x08) >> 1);
		int flipx = (attr & 0x04);
		int flipy = 0;
		int sx = ((248 - state->spriteram[offs + 2]) & 0xff) - 8;
		int sy = state->spriteram[offs];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, 0, machine->gfx[1],
			code, color,
			flipx, flipy,
			sx, sy, 0);

		/* double height */

		if (attr & 0x10)
		{
			drawgfx_transpen(bitmap,cliprect, machine->gfx[1],
				code + 1, color,
				flipx, flipy,
				sx, sy + (flip_screen_get(machine) ? -16 : 16), 0);
		}
	}
}

VIDEO_UPDATE( exprraid )
{
	exprraid_state *state = (exprraid_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 1, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
