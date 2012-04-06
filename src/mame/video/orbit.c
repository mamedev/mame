/***************************************************************************

Atari Orbit video emulation

***************************************************************************/

#include "emu.h"
#include "includes/orbit.h"

WRITE8_MEMBER(orbit_state::orbit_playfield_w)
{
	m_playfield_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


static TILE_GET_INFO( get_tile_info )
{
	orbit_state *state = machine.driver_data<orbit_state>();
	UINT8 code = state->m_playfield_ram[tile_index];
	int flags = 0;

	if (BIT(code, 6))
		flags |= TILE_FLIPX;
	if (state->m_flip_screen)
		flags |= TILE_FLIPY;

	SET_TILE_INFO(3, code & 0x3f, 0, flags);
}


VIDEO_START( orbit )
{
	orbit_state *state = machine.driver_data<orbit_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 16, 16, 32, 30);
}


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	orbit_state *state = machine.driver_data<orbit_state>();
	const UINT8* p = state->m_sprite_ram;

	int i;

	for (i = 0; i < 16; i++)
	{
		int code = *p++;
		int vpos = *p++;
		int hpos = *p++;
		int flag = *p++;

		int layout =
			((flag & 0xc0) == 0x80) ? 1 :
			((flag & 0xc0) == 0xc0) ? 2 : 0;

		int flip_x = BIT(code, 6);
		int flip_y = BIT(code, 7);

		int zoom_x = 0x10000;
		int zoom_y = 0x10000;

		code &= 0x3f;

		if (flag & 1)
			code |= 0x40;
		if (flag & 2)
			zoom_x *= 2;

		vpos = 240 - vpos;

		hpos <<= 1;
		vpos <<= 1;

		drawgfxzoom_transpen(bitmap, cliprect, machine.gfx[layout], code, 0, flip_x, flip_y,
			hpos, vpos, zoom_x, zoom_y, 0);
	}
}


SCREEN_UPDATE_IND16( orbit )
{
	orbit_state *state = screen.machine().driver_data<orbit_state>();

	state->m_flip_screen = input_port_read(screen.machine(), "DSW2") & 8;

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
