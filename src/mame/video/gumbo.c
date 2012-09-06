/* Gumbo video */

#include "emu.h"
#include "includes/gumbo.h"


WRITE16_MEMBER(gumbo_state::gumbo_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(gumbo_state::get_gumbo_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}


WRITE16_MEMBER(gumbo_state::gumbo_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(gumbo_state::get_gumbo_fg_tile_info)
{
	int tileno = m_fg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(1, tileno, 1, 0);
}


VIDEO_START( gumbo )
{
	gumbo_state *state = machine.driver_data<gumbo_state>();
	state->m_bg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(gumbo_state::get_gumbo_bg_tile_info),state), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	state->m_fg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(gumbo_state::get_gumbo_fg_tile_info),state), TILEMAP_SCAN_ROWS, 4, 4, 128, 64);
	state->m_fg_tilemap->set_transparent_pen(0xff);
}

SCREEN_UPDATE_IND16( gumbo )
{
	gumbo_state *state = screen.machine().driver_data<gumbo_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
