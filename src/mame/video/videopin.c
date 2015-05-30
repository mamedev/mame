// license:BSD-3-Clause
// copyright-holders:Sebastien Monassa
/*************************************************************************

    Atari Video Pinball video emulation

*************************************************************************/

#include "emu.h"
#include "includes/videopin.h"





TILEMAP_MAPPER_MEMBER(videopin_state::get_memory_offset)
{
	return num_rows * ((col + 16) % 48) + row;
}


TILE_GET_INFO_MEMBER(videopin_state::get_tile_info)
{
	UINT8 code = m_video_ram[tile_index];

	SET_TILE_INFO_MEMBER(0, code, 0, (code & 0x40) ? TILE_FLIPY : 0);
}


void videopin_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(videopin_state::get_tile_info),this), tilemap_mapper_delegate(FUNC(videopin_state::get_memory_offset),this),  8, 8, 48, 32);

	save_item(NAME(m_ball_x));
	save_item(NAME(m_ball_y));
}


UINT32 videopin_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int col;
	int row;

	m_bg_tilemap->set_scrollx(0, -8);   /* account for delayed loading of shift reg C6 */

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (row = 0; row < 32; row++)
	{
		for (col = 0; col < 48; col++)
		{
			UINT32 offset = m_bg_tilemap->memory_index(col, row);

			if (m_video_ram[offset] & 0x80)   /* ball bit found */
			{
				int x = 8 * col;
				int y = 8 * row;

				int i;
				int j;

				x += 4;   /* account for delayed loading of flip-flop C4 */

				rectangle rect(x, x + 15, y, y + 15);
				rect &= cliprect;

				x -= m_ball_x;
				y -= m_ball_y;

				/* ball placement is still 0.5 pixels off but don't tell anyone */

				for (i = 0; i < 2; i++)
				{
					for (j = 0; j < 2; j++)
					{
						m_gfxdecode->gfx(1)->transpen(bitmap,rect,
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


WRITE8_MEMBER(videopin_state::ball_w)
{
	m_ball_x = data & 15;
	m_ball_y = data >> 4;
}


WRITE8_MEMBER(videopin_state::video_ram_w)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}
