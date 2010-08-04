/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/pbaction.h"

WRITE8_HANDLER( pbaction_videoram_w )
{
	pbaction_state *state = space->machine->driver_data<pbaction_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( pbaction_colorram_w )
{
	pbaction_state *state = space->machine->driver_data<pbaction_state>();
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( pbaction_videoram2_w )
{
	pbaction_state *state = space->machine->driver_data<pbaction_state>();
	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( pbaction_colorram2_w )
{
	pbaction_state *state = space->machine->driver_data<pbaction_state>();
	state->colorram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( pbaction_scroll_w )
{
	pbaction_state *state = space->machine->driver_data<pbaction_state>();
	state->scroll = data - 3;
	if (flip_screen_get(space->machine))
		state->scroll = -state->scroll;

	tilemap_set_scrollx(state->bg_tilemap, 0, state->scroll);
	tilemap_set_scrollx(state->fg_tilemap, 0, state->scroll);
}

WRITE8_HANDLER( pbaction_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x01);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	pbaction_state *state = machine->driver_data<pbaction_state>();
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + 0x10 * (attr & 0x70);
	int color = attr & 0x07;
	int flags = (attr & 0x80) ? TILE_FLIPY : 0;

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	pbaction_state *state = machine->driver_data<pbaction_state>();
	int attr = state->colorram2[tile_index];
	int code = state->videoram2[tile_index] + 0x10 * (attr & 0x30);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( pbaction )
{
	pbaction_state *state = machine->driver_data<pbaction_state>();
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	pbaction_state *state = machine->driver_data<pbaction_state>();
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, flipx, flipy;

		/* if next sprite is double size, skip this one */
		if (offs > 0 && spriteram[offs - 4] & 0x80)
			continue;

		sx = spriteram[offs + 3];

		if (spriteram[offs] & 0x80)
			sy = 225 - spriteram[offs + 2];
		else
			sy = 241 - spriteram[offs + 2];

		flipx = spriteram[offs + 1] & 0x40;
		flipy = spriteram[offs + 1] & 0x80;

		if (flip_screen_get(machine))
		{
			if (spriteram[offs] & 0x80)
			{
				sx = 224 - sx;
				sy = 225 - sy;
			}
			else
			{
				sx = 240 - sx;
				sy = 241 - sy;
			}
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[(spriteram[offs] & 0x80) ? 3 : 2],	/* normal or double size */
				spriteram[offs],
				spriteram[offs + 1] & 0x0f,
				flipx,flipy,
				sx + (flip_screen_get(machine) ? state->scroll : -state->scroll), sy,0);
	}
}

VIDEO_UPDATE( pbaction )
{
	pbaction_state *state = screen->machine->driver_data<pbaction_state>();

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
