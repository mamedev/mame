/* Gumbo video */

#include "emu.h"
#include "includes/gumbo.h"


WRITE16_HANDLER( gumbo_bg_videoram_w )
{
	gumbo_state *state = space->machine().driver_data<gumbo_state>();
	COMBINE_DATA(&state->m_bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

static TILE_GET_INFO( get_gumbo_bg_tile_info )
{
	gumbo_state *state = machine.driver_data<gumbo_state>();
	int tileno = state->m_bg_videoram[tile_index];
	SET_TILE_INFO(0, tileno, 0, 0);
}


WRITE16_HANDLER( gumbo_fg_videoram_w )
{
	gumbo_state *state = space->machine().driver_data<gumbo_state>();
	COMBINE_DATA(&state->m_fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset);
}

static TILE_GET_INFO( get_gumbo_fg_tile_info )
{
	gumbo_state *state = machine.driver_data<gumbo_state>();
	int tileno = state->m_fg_videoram[tile_index];
	SET_TILE_INFO(1, tileno, 1, 0);
}


VIDEO_START( gumbo )
{
	gumbo_state *state = machine.driver_data<gumbo_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_gumbo_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_gumbo_fg_tile_info, tilemap_scan_rows, 4, 4, 128, 64);
	tilemap_set_transparent_pen(state->m_fg_tilemap, 0xff);
}

SCREEN_UPDATE( gumbo )
{
	gumbo_state *state = screen->machine().driver_data<gumbo_state>();
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);
	return 0;
}
