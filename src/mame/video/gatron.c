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


WRITE8_MEMBER(gatron_state::gat_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(gatron_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
/*  - bits -
    7654 3210
    xxxx xxxx   tiles code.

    only one color code
*/

	int code = videoram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void gatron_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gatron_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 8, 16, 48, 16);
}

UINT32 gatron_state::screen_update_gat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

PALETTE_INIT_MEMBER(gatron_state, gatron)
{
}
