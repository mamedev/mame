/***************************************************************************

    Atari Runaway video emulation

****************************************************************************/

#include "emu.h"
#include "includes/runaway.h"


WRITE8_MEMBER(runaway_state::runaway_paletteram_w)
{
	int R =
		0x21 * ((~data >> 2) & 1) +
		0x47 * ((~data >> 3) & 1) +
		0x97 * ((~data >> 4) & 1);

	int G =
		0x21 * ((~data >> 5) & 1) +
		0x47 * ((~data >> 6) & 1) +
		0x97 * ((~data >> 7) & 1);

	int B =
		0x21 * 0 +
		0x47 * ((~data >> 0) & 1) +
		0x97 * ((~data >> 1) & 1);

	palette_set_color(machine(), offset, MAKE_RGB(R, G, B));
}



WRITE8_MEMBER(runaway_state::runaway_video_ram_w)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



WRITE8_MEMBER(runaway_state::runaway_tile_bank_w)
{
	if ((data & 1) != m_tile_bank)
	{
		m_bg_tilemap->mark_all_dirty();
	}

	m_tile_bank = data & 1;
}


static TILE_GET_INFO( runaway_get_tile_info )
{
	runaway_state *state = machine.driver_data<runaway_state>();
	UINT8 code = state->m_video_ram[tile_index];

	SET_TILE_INFO(0, ((code & 0x3f) << 1) | ((code & 0x40) >> 6) | (state->m_tile_bank << 7), 0, (code & 0x80) ? TILE_FLIPY : 0);
}


static TILE_GET_INFO( qwak_get_tile_info )
{
	runaway_state *state = machine.driver_data<runaway_state>();
	UINT8 code = state->m_video_ram[tile_index];

	SET_TILE_INFO(0, ((code & 0x7f) << 1) | ((code & 0x80) >> 7), 0, 0);
}



VIDEO_START( runaway )
{
	runaway_state *state = machine.driver_data<runaway_state>();
	state->m_bg_tilemap = tilemap_create(machine, runaway_get_tile_info, tilemap_scan_rows,  8, 8, 32, 30);

	state->save_item(NAME(state->m_tile_bank));
}


VIDEO_START( qwak )
{
	runaway_state *state = machine.driver_data<runaway_state>();
	state->m_bg_tilemap = tilemap_create(machine, qwak_get_tile_info, tilemap_scan_rows,  8, 8, 32, 30);

	state->save_item(NAME(state->m_tile_bank));
}



SCREEN_UPDATE_IND16( runaway )
{
	runaway_state *state = screen.machine().driver_data<runaway_state>();
	int i;

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	for (i = 0; i < 16; i++)
	{
		unsigned code = state->m_sprite_ram[i] & 0x3f;

		int x = state->m_sprite_ram[i + 0x20];
		int y = state->m_sprite_ram[i + 0x10];

		int flipx = state->m_sprite_ram[i] & 0x40;
		int flipy = state->m_sprite_ram[i] & 0x80;

		code |= (state->m_sprite_ram[i + 0x30] << 2) & 0x1c0;

		drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[1],
			code,
			0,
			flipx, flipy,
			x, 240 - y, 0);

		drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[1],
			code,
			0,
			flipx, flipy,
			x - 256, 240 - y, 0);
	}
	return 0;
}


SCREEN_UPDATE_IND16( qwak )
{
	runaway_state *state = screen.machine().driver_data<runaway_state>();
	int i;

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	for (i = 0; i < 16; i++)
	{
		unsigned code = state->m_sprite_ram[i] & 0x7f;

		int x = state->m_sprite_ram[i + 0x20];
		int y = state->m_sprite_ram[i + 0x10];

		int flipx = 0;
		int flipy = state->m_sprite_ram[i] & 0x80;

		code |= (state->m_sprite_ram[i + 0x30] << 2) & 0x1c0;

		drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[1],
			code,
			0,
			flipx, flipy,
			x, 240 - y, 0);

		drawgfx_transpen(bitmap, cliprect, screen.machine().gfx[1],
			code,
			0,
			flipx, flipy,
			x - 256, 240 - y, 0);
	}
	return 0;
}
