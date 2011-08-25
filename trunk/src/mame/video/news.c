#include "emu.h"
#include "includes/news.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	news_state *state = machine.driver_data<news_state>();
	int code = (state->m_fgram[tile_index * 2] << 8) | state->m_fgram[tile_index * 2 + 1];
	SET_TILE_INFO(
			0,
			code & 0x0fff,
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	news_state *state = machine.driver_data<news_state>();
	int code = (state->m_bgram[tile_index * 2] << 8) | state->m_bgram[tile_index * 2 + 1];
	int color = (code & 0xf000) >> 12;

	code &= 0x0fff;
	if ((code & 0x0e00) == 0x0e00)
		code = (code & 0x1ff) | (state->m_bgpic << 9);

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
	news_state *state = machine.driver_data<news_state>();

	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->m_fg_tilemap, 0);

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( news_fgram_w )
{
	news_state *state = space->machine().driver_data<news_state>();

	state->m_fgram[offset] = data;
	tilemap_mark_tile_dirty(state->m_fg_tilemap, offset / 2);
}

WRITE8_HANDLER( news_bgram_w )
{
	news_state *state = space->machine().driver_data<news_state>();

	state->m_bgram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset / 2);
}

WRITE8_HANDLER( news_bgpic_w )
{
	news_state *state = space->machine().driver_data<news_state>();

	if (state->m_bgpic != data)
	{
		state->m_bgpic = data;
		tilemap_mark_all_tiles_dirty(state->m_bg_tilemap);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE( news )
{
	news_state *state = screen->machine().driver_data<news_state>();
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_fg_tilemap, 0, 0);
	return 0;
}
