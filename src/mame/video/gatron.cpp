// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
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


WRITE8_MEMBER(gatron_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(gatron_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    xxxx xxxx   tiles code.

    only one color code
*/

	int code = m_videoram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void gatron_state::video_start()
{
	m_lamps.resolve();

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gatron_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 8, 16, 48, 16);
}

uint32_t gatron_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
