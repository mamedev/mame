// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari G1 hardware

****************************************************************************/

#include "emu.h"
#include "atarirle.h"
#include "atarig1.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarig1_state::get_alpha_tile_info)
{
	uint16_t const data = m_alpha_tilemap->basemem_read(tile_index);
	int const code = data & 0xfff;
	int const color = (data >> 12) & 0x0f;
	bool const opaque = BIT(data, 15);
	tileinfo.set(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(atarig1_state::get_playfield_tile_info)
{
	uint16_t const data = m_playfield_tilemap->basemem_read(tile_index);
	int const code = (m_playfield_tile_bank << 12) | (data & 0xfff);
	int const color = (data >> 12) & 7;
	tileinfo.set(0, code, color, BIT(data, 15));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void atarig1_state::video_start()
{
	/* blend the playfields and free the temporary one */
	blend_gfx(0, 2, 0x0f, 0x10);

	/* reset statics */
	m_pfscroll_xoffset = m_is_pitfight ? 2 : 0;

	/* state saving */
	save_item(NAME(m_current_control));
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_playfield_xscroll));
	save_item(NAME(m_playfield_yscroll));
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(atarig1_state::scanline_update)
{
	int const scanline = param;

	//if (scanline == 0) logerror("-------\n");

	/* keep in range */
	int offset = (scanline / 8) * 64 + 48;
	if (offset >= 0x800)
		return;
	m_screen->update_partial(std::max(scanline - 1, 0));

	/* update the playfield scrolls */
	for (int i = 0; i < 8; i++)
	{
		/* first word controls horizontal scroll */
		uint16_t word = m_alpha_tilemap->basemem_read(offset++);
		if (BIT(word, 15))
		{
			int const newscroll = ((word >> 6) + m_pfscroll_xoffset) & 0x1ff;
			if (newscroll != m_playfield_xscroll)
			{
				m_screen->update_partial(std::max(scanline + i - 1, 0));
				m_playfield_tilemap->set_scrollx(0, newscroll);
				m_playfield_xscroll = newscroll;
			}
		}

		/* second word controls vertical scroll and tile bank */
		word = m_alpha_tilemap->basemem_read(offset++);
		if (BIT(word, 15))
		{
			int const newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int const newbank = word & 7;
			if (newscroll != m_playfield_yscroll)
			{
				m_screen->update_partial(std::max(scanline + i - 1, 0));
				m_playfield_tilemap->set_scrolly(0, newscroll);
				m_playfield_yscroll = newscroll;
			}
			if (newbank != m_playfield_tile_bank)
			{
				m_screen->update_partial(std::max(scanline + i - 1, 0));
				m_playfield_tilemap->mark_all_dirty();
				m_playfield_tile_bank = newbank;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t atarig1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw the playfield */
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* copy the motion objects on top */
	copybitmap_trans(bitmap, m_rle->vram(0), 0, 0, 0, 0, cliprect, 0);

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
