// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina, Roberto Fresca
/***************************************************************************

  IDSA 4 En Raya

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/4enraya.h"

WRITE8_MEMBER(_4enraya_state::fenraya_videoram_w)
{
	m_videoram[(offset & 0x3ff) * 2] = data;
	m_videoram[(offset & 0x3ff) * 2 + 1] = (offset & 0xc00) >> 10;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

PALETTE_INIT_MEMBER(_4enraya_state, _4enraya)
{
	/* RGB format */
	for (int i = 0; i < 8; i++)
		m_palette->set_pen_color(i, rgb_t(pal1bit(i >> 0),pal1bit(i >> 1),pal1bit(i >> 2)));
}

TILE_GET_INFO_MEMBER(_4enraya_state::get_tile_info)
{
	int code = m_videoram[tile_index * 2] + (m_videoram[tile_index * 2 + 1] << 8);
	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void _4enraya_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(_4enraya_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

UINT32 _4enraya_state::screen_update_4enraya(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
