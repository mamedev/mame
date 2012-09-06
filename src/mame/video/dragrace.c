/***************************************************************************

Atari Drag Race video emulation

***************************************************************************/

#include "emu.h"
#include "includes/dragrace.h"


TILE_GET_INFO_MEMBER(dragrace_state::get_tile_info)
{
	UINT8 code = m_playfield_ram[tile_index];
	int num = 0;
	int col = 0;

	num = code & 0x1f;

	if ((code & 0xc0) == 0x40)
		num |= 0x20;

	switch (code & 0xA0)
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
	case 0xA0:
		col = (code & 0x40) ? 3 : 2;
		break;
	}

	SET_TILE_INFO_MEMBER(((code & 0xA0) == 0x80) ? 1 : 0, num, col, 0);
}


VIDEO_START( dragrace )
{
	dragrace_state *state = machine.driver_data<dragrace_state>();
	state->m_bg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(dragrace_state::get_tile_info),state), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
}


SCREEN_UPDATE_IND16( dragrace )
{
	dragrace_state *state = screen.machine().driver_data<dragrace_state>();
	int y;

	state->m_bg_tilemap->mark_all_dirty();

	for (y = 0; y < 256; y += 4)
	{
		rectangle rect = cliprect;

		int xl = state->m_position_ram[y + 0] & 15;
		int xh = state->m_position_ram[y + 1] & 15;
		int yl = state->m_position_ram[y + 2] & 15;
		int yh = state->m_position_ram[y + 3] & 15;

		state->m_bg_tilemap->set_scrollx(0, 16 * xh + xl - 8);
		state->m_bg_tilemap->set_scrolly(0, 16 * yh + yl);

		if (rect.min_y < y + 0) rect.min_y = y + 0;
		if (rect.max_y > y + 3) rect.max_y = y + 3;

		state->m_bg_tilemap->draw(bitmap, rect, 0, 0);
	}
	return 0;
}
