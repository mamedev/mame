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
	state->m_fg_tilemap->set_transparent_pen(0);

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(news_state::news_fgram_w)
{

	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(news_state::news_bgram_w)
{

	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(news_state::news_bgpic_w)
{

	if (m_bgpic != data)
	{
		m_bgpic = data;
		m_bg_tilemap->mark_all_dirty();
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( news )
{
	news_state *state = screen.machine().driver_data<news_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
