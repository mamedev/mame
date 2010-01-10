/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/bombjack.h"

WRITE8_HANDLER( bombjack_videoram_w )
{
	bombjack_state *state = (bombjack_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( bombjack_colorram_w )
{
	bombjack_state *state = (bombjack_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( bombjack_background_w )
{
	bombjack_state *state = (bombjack_state *)space->machine->driver_data;

	if (state->background_image != data)
	{
		state->background_image = data;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}

WRITE8_HANDLER( bombjack_flipscreen_w )
{
	if (flip_screen_get(space->machine) != (data & 0x01))
	{
		flip_screen_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	bombjack_state *state = (bombjack_state *)machine->driver_data;
	UINT8 *tilerom = memory_region(machine, "gfx4");

	int offs = (state->background_image & 0x07) * 0x200 + tile_index;
	int code = (state->background_image & 0x10) ? tilerom[offs] : 0;
	int attr = tilerom[offs + 0x100];
	int color = attr & 0x0f;
	int flags = (attr & 0x80) ? TILE_FLIPY : 0;

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	bombjack_state *state = (bombjack_state *)machine->driver_data;
	int code = state->videoram[tile_index] + 16 * (state->colorram[tile_index] & 0x10);
	int color = state->colorram[tile_index] & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( bombjack )
{
	bombjack_state *state = (bombjack_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 16, 16);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	bombjack_state *state = (bombjack_state *)machine->driver_data;
	int offs;

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{

/*
 abbbbbbb cdefgggg hhhhhhhh iiiiiiii

 a        use big sprites (32x32 instead of 16x16)
 bbbbbbb  sprite code
 c        x flip
 d        y flip (used only in death sequence?)
 e        ? (set when big sprites are selected)
 f        ? (set only when the bonus (B) materializes?)
 gggg     color
 hhhhhhhh x position
 iiiiiiii y position
*/
		int sx,sy,flipx,flipy;


		sx = state->spriteram[offs + 3];

		if (state->spriteram[offs] & 0x80)
			sy = 225 - state->spriteram[offs + 2];
		else
			sy = 241 - state->spriteram[offs + 2];

		flipx = state->spriteram[offs + 1] & 0x40;
		flipy = state->spriteram[offs + 1] & 0x80;

		if (flip_screen_get(machine))
		{
			if (state->spriteram[offs + 1] & 0x20)
			{
				sx = 224 - sx;
				sy = 224 - sy;
			}
			else
			{
				sx = 240 - sx;
				sy = 240 - sy;
			}
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[(state->spriteram[offs] & 0x80) ? 3 : 2],
				state->spriteram[offs] & 0x7f,
				state->spriteram[offs + 1] & 0x0f,
				flipx,flipy,
				sx,sy,0);
	}
}

VIDEO_UPDATE( bombjack )
{
	bombjack_state *state = (bombjack_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
