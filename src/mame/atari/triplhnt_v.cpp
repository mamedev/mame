// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Triple Hunt video emulation

***************************************************************************/

#include "emu.h"
#include "includes/triplhnt.h"


TILE_GET_INFO_MEMBER(triplhnt_state::get_tile_info)
{
	int code = m_playfield_ram[tile_index] & 0x3f;

	tileinfo.set(2, code, code == 0x3f ? 1 : 0, 0);
}


void triplhnt_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(triplhnt_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);

	m_hit_timer = timer_alloc(FUNC(triplhnt_state::set_collision), this);

	save_item(NAME(m_cmos));
	save_item(NAME(m_da_latch));
	save_item(NAME(m_cmos_latch));
	save_item(NAME(m_hit_code));
	save_item(NAME(m_sprite_zoom));
	save_item(NAME(m_sprite_bank));
}


void triplhnt_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int hit_line = 999;
	int hit_code = 999;

	for (int i = 0; i < 16; i++)
	{
		rectangle rect;

		int j = (m_orga_ram[i] & 15) ^ 15;

		/* software sorts sprites by x and stores order in orga RAM */

		int hpos = m_hpos_ram[j] ^ 255;
		int vpos = m_vpos_ram[j] ^ 255;
		int code = m_code_ram[j] ^ 255;

		if (hpos == 255)
			continue;

		/* sprite placement might be wrong */

		if (m_sprite_zoom)
		{
			rect.set(hpos - 16, hpos - 16 + 63, 196 - vpos, 196 - vpos + 63);
		}
		else
		{
			rect.set(hpos - 16, hpos - 16 + 31, 224 - vpos, 224 - vpos + 31);
		}

		/* render sprite to auxiliary bitmap */

		m_gfxdecode->gfx(m_sprite_zoom)->opaque(m_helper,cliprect,
			2 * code + m_sprite_bank, 0, code & 8, 0,
			rect.left(), rect.top());

		rect &= cliprect;

		/* check for collisions and copy sprite */
		for (int x = rect.left(); x <= rect.right(); x++)
		{
			for (int y = rect.top(); y <= rect.bottom(); y++)
			{
				pen_t const a = m_helper.pix(y, x);
				pen_t const b = bitmap.pix(y, x);

				if (a == 2 && b == 7)
				{
					hit_code = j;
					hit_line = y;
				}

				if (a != 1)
					bitmap.pix(y, x) = a;
			}
		}
	}

	if (hit_line != 999 && hit_code != 999)
		m_hit_timer->adjust(m_screen->time_until_pos(hit_line), hit_code);
}


uint32_t triplhnt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->mark_all_dirty();

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	m_discrete->write(TRIPLHNT_BEAR_ROAR_DATA, m_playfield_ram[0xfa] & 15);
	m_discrete->write(TRIPLHNT_SHOT_DATA, m_playfield_ram[0xfc] & 15);
	return 0;
}
