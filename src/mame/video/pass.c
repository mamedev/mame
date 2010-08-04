/* video/pass.c - see drivers/pass.c for more info */

#include "emu.h"
#include "includes/pass.h"

/* background tilemap stuff */

static TILE_GET_INFO( get_pass_bg_tile_info )
{
	pass_state *state = machine->driver_data<pass_state>();
	int tileno, fx;

	tileno = state->bg_videoram[tile_index] & 0x1fff;
	fx = (state->bg_videoram[tile_index] & 0xc000) >> 14;
	SET_TILE_INFO(1, tileno, 0, TILE_FLIPYX(fx));

}

WRITE16_HANDLER( pass_bg_videoram_w )
{
	pass_state *state = space->machine->driver_data<pass_state>();

	state->bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

/* foreground 'sprites' tilemap stuff */

static TILE_GET_INFO( get_pass_fg_tile_info )
{
	pass_state *state = machine->driver_data<pass_state>();
	int tileno, flip;

	tileno = state->fg_videoram[tile_index] & 0x3fff;
	flip = (state->fg_videoram[tile_index] & 0xc000) >>14;

	SET_TILE_INFO(0, tileno, 0, TILE_FLIPYX(flip));

}

WRITE16_HANDLER( pass_fg_videoram_w )
{
	pass_state *state = space->machine->driver_data<pass_state>();
	state->fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

/* video update / start */

VIDEO_START( pass )
{
	pass_state *state = machine->driver_data<pass_state>();

	state->bg_tilemap = tilemap_create(machine, get_pass_bg_tile_info, tilemap_scan_rows, 8, 8,  64, 32);
	state->fg_tilemap = tilemap_create(machine, get_pass_fg_tile_info, tilemap_scan_rows, 4, 4, 128, 64);

	tilemap_set_transparent_pen(state->fg_tilemap, 255);
}

VIDEO_UPDATE( pass )
{
	pass_state *state = screen->machine->driver_data<pass_state>();

	tilemap_draw(bitmap,cliprect,state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap,cliprect,state->fg_tilemap, 0, 0);

	return 0;
}
