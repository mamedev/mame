/******************************************************************************

    GAME-A-TRON gambling hardware
    -----------------------------

    *** Video Hardware ***

    Written by Roberto Fresca.


    Games running on this hardware:

    * Poker 4-1,  1983, Game-A-Tron.
    * Pull Tabs,  1983, Game-A-Tron.


*******************************************************************************/


#include "emu.h"
#include "includes/gatron.h"


WRITE8_MEMBER(gatron_state::gat_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	gatron_state *state = machine.driver_data<gatron_state>();
	UINT8 *videoram = state->m_videoram;
/*  - bits -
    7654 3210
    xxxx xxxx   tiles code.

    only one color code
*/

	int code = videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( gat )
{
	gatron_state *state = machine.driver_data<gatron_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 8, 16, 48, 16);
}

SCREEN_UPDATE_IND16( gat )
{
	gatron_state *state = screen.machine().driver_data<gatron_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

PALETTE_INIT( gat )
{
}

