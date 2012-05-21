/*************************************************************************

    Atari Video Pinball video emulation

*************************************************************************/

#include "emu.h"
#include "includes/videopin.h"





static TILEMAP_MAPPER( get_memory_offset )
{
	return num_rows * ((col + 16) % 48) + row;
}


static TILE_GET_INFO( get_tile_info )
{
	videopin_state *state = machine.driver_data<videopin_state>();
	UINT8 code = state->m_video_ram[tile_index];

	SET_TILE_INFO(0, code, 0, (code & 0x40) ? TILE_FLIPY : 0);
}


VIDEO_START( videopin )
{
	videopin_state *state = machine.driver_data<videopin_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, get_memory_offset,  8, 8, 48, 32);
}


SCREEN_UPDATE_IND16( videopin )
{
	videopin_state *state = screen.machine().driver_data<videopin_state>();
	int col;
	int row;

	state->m_bg_tilemap->set_scrollx(0, -8);   /* account for delayed loading of shift reg C6 */

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	for (row = 0; row < 32; row++)
	{
		for (col = 0; col < 48; col++)
		{
			UINT32 offset = state->m_bg_tilemap->memory_index(col, row);

			if (state->m_video_ram[offset] & 0x80)   /* ball bit found */
			{
				int x = 8 * col;
				int y = 8 * row;

				int i;
				int j;

				x += 4;   /* account for delayed loading of flip-flop C4 */

				rectangle rect(x, x + 15, y, y + 15);
				rect &= cliprect;

				x -= state->m_ball_x;
				y -= state->m_ball_y;

				/* ball placement is still 0.5 pixels off but don't tell anyone */

				for (i = 0; i < 2; i++)
				{
					for (j = 0; j < 2; j++)
					{
						drawgfx_transpen(bitmap, rect, screen.machine().gfx[1],
							0, 0,
							0, 0,
							x + 16 * i,
							y + 16 * j, 0);
					}
				}

				return 0;   /* keep things simple and ignore the rest */
			}
		}
	}
	return 0;
}


WRITE8_MEMBER(videopin_state::videopin_ball_w)
{
	m_ball_x = data & 15;
	m_ball_y = data >> 4;
}


WRITE8_MEMBER(videopin_state::videopin_video_ram_w)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}
