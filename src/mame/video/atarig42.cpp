// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari G42 hardware

*****************************************************************************

    MO data has 12 bits total: MVID0-11
    MVID9-11 form the priority
    MVID0-9 form the color bits

    PF data has 13 bits total: PF.VID0-12
    PF.VID10-12 form the priority
    PF.VID0-9 form the color bits

    Upper bits come from the low 5 bits of the HSCROLL value in alpha RAM
    Playfield bank comes from low 2 bits of the VSCROLL value in alpha RAM
    For GX2, there are 4 bits of bank

****************************************************************************/


#include "emu.h"
#include "video/atarirle.h"
#include "includes/atarig42.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarig42_state::get_alpha_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = data & 0xfff;
	int color = (data >> 12) & 0x0f;
	int opaque = data & 0x8000;
	SET_TILE_INFO_MEMBER(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(atarig42_state::get_playfield_tile_info)
{
	UINT16 data = tilemap.basemem_read(tile_index);
	int code = (m_playfield_tile_bank << 12) | (data & 0xfff);
	int color = (m_playfield_base >> 5) + ((m_playfield_color_bank << 3) & 0x18) + ((data >> 12) & 7);
	SET_TILE_INFO_MEMBER(0, code, color, (data >> 15) & 1);
	tileinfo.category = (m_playfield_color_bank >> 2) & 7;
}


TILEMAP_MAPPER_MEMBER(atarig42_state::atarig42_playfield_scan)
{
	int bank = 1 - (col / (num_cols / 2));
	return bank * (num_rows * num_cols / 2) + row * (num_cols / 2) + (col % (num_cols / 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(atarig42_state,atarig42)
{
	/* blend the playfields and free the temporary one */
	blend_gfx(0, 2, 0x0f, 0x30);

	/* save states */
	save_item(NAME(m_current_control));
	save_item(NAME(m_playfield_tile_bank));
	save_item(NAME(m_playfield_color_bank));
	save_item(NAME(m_playfield_xscroll));
	save_item(NAME(m_playfield_yscroll));
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void atarig42_state::scanline_update(screen_device &screen, int scanline)
{
	int i;

	if (scanline == 0) logerror("-------\n");

	/* keep in range */
	int offset = (scanline / 8) * 64 + 48;
	if (offset >= 0x800)
		return;

	/* update the playfield scrolls */
	for (i = 0; i < 8; i++)
	{
		UINT16 word;

		word = m_alpha_tilemap->basemem_read(offset++);
		if (word & 0x8000)
		{
			int newscroll = (word >> 5) & 0x3ff;
			int newbank = word & 0x1f;
			if (newscroll != m_playfield_xscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				m_playfield_tilemap->set_scrollx(0, newscroll);
				m_playfield_xscroll = newscroll;
			}
			if (newbank != m_playfield_color_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				m_playfield_tilemap->mark_all_dirty();
				m_playfield_color_bank = newbank;
			}
		}

		word = m_alpha_tilemap->basemem_read(offset++);
		if (word & 0x8000)
		{
			int newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int newbank = word & 7;
			if (newscroll != m_playfield_yscroll)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
				m_playfield_tilemap->set_scrolly(0, newscroll);
				m_playfield_yscroll = newscroll;
			}
			if (newbank != m_playfield_tile_bank)
			{
				if (scanline + i > 0)
					screen.update_partial(scanline + i - 1);
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

UINT32 atarig42_state::screen_update_atarig42(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind8 &priority_bitmap = screen.priority();

	/* draw the playfield */
	priority_bitmap.fill(0, cliprect);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 1, 1);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 2, 2);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 3, 3);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 4, 4);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 5, 5);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 6, 6);
	m_playfield_tilemap->draw(screen, bitmap, cliprect, 7, 7);

	/* copy the motion objects on top */
	{
		bitmap_ind16 &mo_bitmap = m_rle->vram(0);
		int left    = cliprect.min_x;
		int top     = cliprect.min_y;
		int right   = cliprect.max_x + 1;
		int bottom  = cliprect.max_y + 1;
		int x, y;

		/* now blend with the playfield */
		for (y = top; y < bottom; y++)
		{
			UINT16 *pf = &bitmap.pix16(y);
			UINT16 *mo = &mo_bitmap.pix16(y);
			UINT8 *pri = &priority_bitmap.pix8(y);
			for (x = left; x < right; x++)
				if (mo[x])
				{
					int pfpri = pri[x];
					int mopri = mo[x] >> ATARIRLE_PRIORITY_SHIFT;
					if (mopri >= pfpri)
						pf[x] = mo[x] & ATARIRLE_DATA_MASK;
				}
		}
	}

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
