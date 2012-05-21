/* video/pass.c - see drivers/pass.c for more info */

#include "emu.h"
#include "includes/pass.h"

/* background tilemap stuff */

static TILE_GET_INFO( get_pass_bg_tile_info )
{
	pass_state *state = machine.driver_data<pass_state>();
	int tileno, fx;

	tileno = state->m_bg_videoram[tile_index] & 0x1fff;
	fx = (state->m_bg_videoram[tile_index] & 0xc000) >> 14;
	SET_TILE_INFO(1, tileno, 0, TILE_FLIPYX(fx));

}

WRITE16_MEMBER(pass_state::pass_bg_videoram_w)
{

	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

/* foreground 'sprites' tilemap stuff */

static TILE_GET_INFO( get_pass_fg_tile_info )
{
	pass_state *state = machine.driver_data<pass_state>();
	int tileno, flip;

	tileno = state->m_fg_videoram[tile_index] & 0x3fff;
	flip = (state->m_fg_videoram[tile_index] & 0xc000) >>14;

	SET_TILE_INFO(0, tileno, 0, TILE_FLIPYX(flip));

}

WRITE16_MEMBER(pass_state::pass_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

/* video update / start */

VIDEO_START( pass )
{
	pass_state *state = machine.driver_data<pass_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_pass_bg_tile_info, tilemap_scan_rows, 8, 8,  64, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_pass_fg_tile_info, tilemap_scan_rows, 4, 4, 128, 64);

	state->m_fg_tilemap->set_transparent_pen(255);
}

SCREEN_UPDATE_IND16( pass )
{
	pass_state *state = screen.machine().driver_data<pass_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);

	return 0;
}
