#include "emu.h"
#include "includes/blockade.h"

WRITE8_MEMBER(blockade_state::blockade_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);

	if (input_port_read(machine(), "IN3") & 0x80)
	{
		logerror("blockade_videoram_w: scanline %d\n", machine().primary_screen->vpos());
		device_spin_until_interrupt(&space.device());
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	blockade_state *state = machine.driver_data<blockade_state>();
	int code = state->m_videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( blockade )
{
	blockade_state *state = machine.driver_data<blockade_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

SCREEN_UPDATE_IND16( blockade )
{
	blockade_state *state = screen.machine().driver_data<blockade_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
