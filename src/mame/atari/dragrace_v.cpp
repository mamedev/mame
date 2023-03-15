// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Drag Race video emulation

***************************************************************************/

#include "emu.h"
#include "dragrace.h"


TILE_GET_INFO_MEMBER(dragrace_state::get_tile_info)
{
	uint8_t code = m_playfield_ram[tile_index];
	int num = code & 0x1f;
	int col = 0;

	if ((code & 0xc0) == 0x40)
		num |= 0x20;

	switch (code & 0xa0)
	{
		case 0x00:
			col = 0;
			break;
		case 0x20:
			col = 1;
			break;
		case 0x80:
			col = (code & 0x40) ? 1 : 0;
			break;
		case 0xa0:
			col = (code & 0x40) ? 3 : 2;
			break;
	}

	tileinfo.set(((code & 0xa0) == 0x80) ? 1 : 0, num, col, 0);
}


void dragrace_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dragrace_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
}


uint32_t dragrace_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->mark_all_dirty();

	for (int y = 0; y < 256; y += 4)
	{
		rectangle rect = cliprect;

		int xl = m_position_ram[y + 0] & 15;
		int xh = m_position_ram[y + 1] & 15;
		int yl = m_position_ram[y + 2] & 15;
		int yh = m_position_ram[y + 3] & 15;

		m_bg_tilemap->set_scrollx(0, 16 * xh + xl - 8);
		m_bg_tilemap->set_scrolly(0, 16 * yh + yl);

		rect.sety((std::max)(rect.top(), y + 0), (std::min)(rect.bottom(), y + 3));

		m_bg_tilemap->draw(screen, bitmap, rect, 0, 0);
	}

	return 0;
}
