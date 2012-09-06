/***************************************************************************

Atari Triple Hunt video emulation

***************************************************************************/

#include "emu.h"
#include "includes/triplhnt.h"


TILE_GET_INFO_MEMBER(triplhnt_state::get_tile_info)
{
	int code = m_playfield_ram[tile_index] & 0x3f;

	SET_TILE_INFO_MEMBER(2, code, code == 0x3f ? 1 : 0, 0);
}


VIDEO_START( triplhnt )
{
	triplhnt_state *state = machine.driver_data<triplhnt_state>();
	machine.primary_screen->register_screen_bitmap(state->m_helper);

	state->m_bg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(triplhnt_state::get_tile_info),state), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
}


static TIMER_CALLBACK( triplhnt_hit_callback )
{
	triplhnt_set_collision(machine, param);
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	triplhnt_state *state = machine.driver_data<triplhnt_state>();
	int i;

	int hit_line = 999;
	int hit_code = 999;

	for (i = 0; i < 16; i++)
	{
		rectangle rect;

		int j = (state->m_orga_ram[i] & 15) ^ 15;

		/* software sorts sprites by x and stores order in orga RAM */

		int hpos = state->m_hpos_ram[j] ^ 255;
		int vpos = state->m_vpos_ram[j] ^ 255;
		int code = state->m_code_ram[j] ^ 255;

		if (hpos == 255)
			continue;

		/* sprite placement might be wrong */

		if (state->m_sprite_zoom)
		{
			rect.set(hpos - 16, hpos - 16 + 63, 196 - vpos, 196 - vpos + 63);
		}
		else
		{
			rect.set(hpos - 16, hpos - 16 + 31, 224 - vpos, 224 - vpos + 31);
		}

		/* render sprite to auxiliary bitmap */

		drawgfx_opaque(state->m_helper, cliprect, machine.gfx[state->m_sprite_zoom],
			2 * code + state->m_sprite_bank, 0, code & 8, 0,
			rect.min_x, rect.min_y);

		rect &= cliprect;

		/* check for collisions and copy sprite */

		{
			int x;
			int y;

			for (x = rect.min_x; x <= rect.max_x; x++)
			{
				for (y = rect.min_y; y <= rect.max_y; y++)
				{
					pen_t a = state->m_helper.pix16(y, x);
					pen_t b = bitmap.pix16(y, x);

					if (a == 2 && b == 7)
					{
						hit_code = j;
						hit_line = y;
					}

					if (a != 1)
						bitmap.pix16(y, x) = a;
				}
			}
		}
	}

	if (hit_line != 999 && hit_code != 999)
		machine.scheduler().timer_set(machine.primary_screen->time_until_pos(hit_line), FUNC(triplhnt_hit_callback), hit_code);
}


SCREEN_UPDATE_IND16( triplhnt )
{
	triplhnt_state *state = screen.machine().driver_data<triplhnt_state>();
	device_t *discrete = screen.machine().device("discrete");

	state->m_bg_tilemap->mark_all_dirty();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen.machine(), bitmap, cliprect);

	discrete_sound_w(discrete, TRIPLHNT_BEAR_ROAR_DATA, state->m_playfield_ram[0xfa] & 15);
	discrete_sound_w(discrete, TRIPLHNT_SHOT_DATA, state->m_playfield_ram[0xfc] & 15);
	return 0;
}
