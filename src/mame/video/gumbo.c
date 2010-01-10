/* Gumbo video */

#include "emu.h"
#include "includes/gumbo.h"


WRITE16_HANDLER( gumbo_bg_videoram_w )
{
	gumbo_state *state = (gumbo_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

static TILE_GET_INFO( get_gumbo_bg_tile_info )
{
	gumbo_state *state = (gumbo_state *)machine->driver_data;
	int tileno = state->bg_videoram[tile_index];
	SET_TILE_INFO(0, tileno, 0, 0);
}


WRITE16_HANDLER( gumbo_fg_videoram_w )
{
	gumbo_state *state = (gumbo_state *)space->machine->driver_data;
	COMBINE_DATA(&state->fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

static TILE_GET_INFO( get_gumbo_fg_tile_info )
{
	gumbo_state *state = (gumbo_state *)machine->driver_data;
	int tileno = state->fg_videoram[tile_index];
	SET_TILE_INFO(1, tileno, 1, 0);
}


VIDEO_START( gumbo )
{
	gumbo_state *state = (gumbo_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_gumbo_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->fg_tilemap = tilemap_create(machine, get_gumbo_fg_tile_info, tilemap_scan_rows, 4, 4, 128, 64);
	tilemap_set_transparent_pen(state->fg_tilemap, 0xff);
}

VIDEO_UPDATE( gumbo )
{
	gumbo_state *state = (gumbo_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
