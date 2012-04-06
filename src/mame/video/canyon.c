/***************************************************************************

Atari Canyon Bomber video emulation

***************************************************************************/

#include "emu.h"
#include "includes/canyon.h"


WRITE8_MEMBER(canyon_state::canyon_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	canyon_state *state = machine.driver_data<canyon_state>();
	UINT8 code = state->m_videoram[tile_index];

	SET_TILE_INFO(0, code & 0x3f, code >> 7, 0);
}


VIDEO_START( canyon )
{
	canyon_state *state = machine.driver_data<canyon_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	canyon_state *state = machine.driver_data<canyon_state>();
	int i;

	for (i = 0; i < 2; i++)
	{
		int x = state->m_videoram[0x3d0 + 2 * i + 0x1];
		int y = state->m_videoram[0x3d0 + 2 * i + 0x8];
		int c = state->m_videoram[0x3d0 + 2 * i + 0x9];

		drawgfx_transpen(bitmap, cliprect,
			machine.gfx[1],
			c >> 3,
			i,
			!(c & 0x80), 0,
			224 - x,
			240 - y, 0);
	}
}


static void draw_bombs( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	canyon_state *state = machine.driver_data<canyon_state>();
	int i;

	for (i = 0; i < 2; i++)
	{
		int sx = 254 - state->m_videoram[0x3d0 + 2 * i + 0x5];
		int sy = 246 - state->m_videoram[0x3d0 + 2 * i + 0xc];

		rectangle rect(sx, sx + 1, sy, sy + 1);
		rect &= cliprect;

		bitmap.fill(1 + 2 * i, rect);
	}
}


SCREEN_UPDATE_IND16( canyon )
{
	canyon_state *state = screen.machine().driver_data<canyon_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen.machine(), bitmap, cliprect);

	draw_bombs(screen.machine(), bitmap, cliprect);

	/* watchdog is disabled during service mode */
	watchdog_enable(screen.machine(), !(input_port_read(screen.machine(), "IN2") & 0x10));

	return 0;
}
