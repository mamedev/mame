// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari G1 hardware

****************************************************************************/

#include "emu.h"
#include "video/atarirle.h"
#include "includes/atarig1.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarig1_state::get_alpha_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(atarig1_state::get_playfield_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = (m_playfield_tile_bank << 12) | (data & 0xfff);
	int color = (data >> 12) & 7;
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(atarig1_state,atarig1)
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

void atarig1_state::scanline_update(screen_device &screen, int scanline)
{
	int i;

	//if (scanline == 0) logerror("-------\n");

	/* keep in range */
	int offset = (scanline / 8) * 64 + 48;
	if (offset >= 0x800)
		return;
	screen.update_partial(MAX(scanline - 1, 0));

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT16 word;

		/* first word controls horizontal scroll */
		word = m_alpha_tilemap->basemem_read(offset++);
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) + m_pfscroll_xoffset) & 0x1ff;
			if (newscroll != m_playfield_xscroll)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				m_playfield_tilemap->set_scrollx(0, newscroll);
				m_playfield_xscroll = newscroll;
			}
		}

		/* second word controls vertical scroll and tile bank */
		word = m_alpha_tilemap->basemem_read(offset++);
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 7;
			if (newscroll != m_playfield_yscroll)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
				m_playfield_tilemap->set_scrolly(0, newscroll);
				m_playfield_yscroll = newscroll;
			}
			if (newbank != m_playfield_tile_bank)
			{
				screen.update_partial(MAX(scanline + i - 1, 0));
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

UINT32 atarig1_state::screen_update_atarig1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw the playfield */
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* copy the motion objects on top */
	copybitmap_trans(bitmap, m_rle->vram(0), 0, 0, 0, 0, cliprect, 0);

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
