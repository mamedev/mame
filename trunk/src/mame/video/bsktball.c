/***************************************************************************

    Atari Basketball hardware

***************************************************************************/

#include "emu.h"
#include "includes/bsktball.h"


WRITE8_MEMBER(bsktball_state::bsktball_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	bsktball_state *state = machine.driver_data<bsktball_state>();
	int attr = state->m_videoram[tile_index];
	int code = ((attr & 0x0f) << 2) | ((attr & 0x30) >> 4);
	int color = (attr & 0x40) >> 6;
	int flags = (attr & 0x80) ? TILE_FLIPX : 0;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( bsktball )
{
	bsktball_state *state = machine.driver_data<bsktball_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_sprites( running_machine &machine,  bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bsktball_state *state = machine.driver_data<bsktball_state>();
	int mot;

	for (mot = 0; mot < 16; mot++)
	{
		int pic = state->m_motion[mot * 4];
		int sy = 28 * 8 - state->m_motion[mot * 4 + 1];
		int sx = state->m_motion[mot * 4 + 2];
		int color = state->m_motion[mot * 4 + 3];
		int flipx = (pic & 0x80) >> 7;

		pic = (pic & 0x3f);
		color = (color & 0x3f);

		drawgfx_transpen(bitmap, cliprect, machine.gfx[1], pic, color, flipx, 0, sx, sy, 0);
	}
}

SCREEN_UPDATE_IND16( bsktball )
{
	bsktball_state *state = screen.machine().driver_data<bsktball_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
