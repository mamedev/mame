#include "emu.h"
#include "includes/news.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	news_state *state = (news_state *)machine->driver_data;
	int code = (state->fgram[tile_index * 2] << 8) | state->fgram[tile_index * 2 + 1];
	SET_TILE_INFO(
			0,
			code & 0x0fff,
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	news_state *state = (news_state *)machine->driver_data;
	int code = (state->bgram[tile_index * 2] << 8) | state->bgram[tile_index * 2 + 1];
	int color = (code & 0xf000) >> 12;

	code &= 0x0fff;
	if ((code & 0x0e00) == 0x0e00)
		code = (code & 0x1ff) | (state->bgpic << 9);

	SET_TILE_INFO(
			0,
			code,
			color,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( news )
{
	news_state *state = (news_state *)machine->driver_data;

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( news_fgram_w )
{
	news_state *state = (news_state *)space->machine->driver_data;

	state->fgram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset / 2);
}

WRITE8_HANDLER( news_bgram_w )
{
	news_state *state = (news_state *)space->machine->driver_data;

	state->bgram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE8_HANDLER( news_bgpic_w )
{
	news_state *state = (news_state *)space->machine->driver_data;

	if (state->bgpic != data)
	{
		state->bgpic = data;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( news )
{
	news_state *state = (news_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
