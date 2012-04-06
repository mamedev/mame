/***************************************************************************

    Atari Night Driver hardware

***************************************************************************/

#include "emu.h"
#include "includes/nitedrvr.h"

WRITE8_MEMBER(nitedrvr_state::nitedrvr_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(nitedrvr_state::nitedrvr_hvc_w)
{

	m_hvc[offset & 0x3f] = data;

	if ((offset & 0x30) == 0x30)
		watchdog_reset_w(space, 0, 0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	nitedrvr_state *state = machine.driver_data<nitedrvr_state>();
	int code = state->m_videoram[tile_index] & 0x3f;

	SET_TILE_INFO(0, code, 0, 0);
}



VIDEO_START( nitedrvr )
{
	nitedrvr_state *state = machine.driver_data<nitedrvr_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_box( bitmap_ind16 &bitmap, int bx, int by, int ex, int ey )
{
	int x, y;

	for (y = by; y < ey; y++)
	{
		for (x = bx; x < ex; x++)
			if ((y < 256) && (x < 256))
				bitmap.pix16(y, x) = 1;
	}

	return;
}

static void draw_roadway( running_machine &machine, bitmap_ind16 &bitmap )
{
	nitedrvr_state *state = machine.driver_data<nitedrvr_state>();
	int roadway;

	for (roadway = 0; roadway < 16; roadway++)
	{
		int bx, by, ex, ey;

		bx = state->m_hvc[roadway];
		by = state->m_hvc[roadway + 16];
		ex = bx + ((state->m_hvc[roadway + 32] & 0xf0) >> 4);
		ey = by + (16 - (state->m_hvc[roadway + 32] & 0x0f));

		draw_box(bitmap, bx, by, ex, ey);
	}
}

SCREEN_UPDATE_IND16( nitedrvr )
{
	nitedrvr_state *state = screen.machine().driver_data<nitedrvr_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_roadway(screen.machine(), bitmap);
	return 0;
}
