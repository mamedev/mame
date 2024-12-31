// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari GX2 hardware

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
#include "atarirle.h"
#include "atarigx2.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(atarigx2_state::get_alpha_tile_info)
{
	uint16_t const data = m_alpha_tilemap->basemem_read(tile_index);
	int const code = data & 0xfff;
	int const color = (data >> 12) & 0x0f;
	bool const opaque = BIT(data, 15);
	tileinfo.set(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


TILE_GET_INFO_MEMBER(atarigx2_state::get_playfield_tile_info)
{
	uint16_t const data = m_playfield_tilemap->basemem_read(tile_index);
	int const code = (m_playfield_tile_bank << 12) | (data & 0xfff);
	int const color = (m_playfield_base >> 5) + ((m_playfield_color_bank << 3) & 0x18) + ((data >> 12) & 7);
	tileinfo.set(0, code, color, BIT(data, 15));
	tileinfo.category = (m_playfield_color_bank >> 2) & 7;
}


TILEMAP_MAPPER_MEMBER(atarigx2_state::atarigx2_playfield_scan)
{
	int const bank = 1 - (col / (num_cols / 2));
	return bank * (num_rows * num_cols / 2) + row * (num_cols / 2) + (col % (num_cols / 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void atarigx2_state::video_start()
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

void atarigx2_state::atarigx2_mo_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("MOCONT = %d (scan = %d)\n", data, m_screen->vpos());

	/* set the control value */
	COMBINE_DATA(&m_current_control);
}


TIMER_DEVICE_CALLBACK_MEMBER(atarigx2_state::scanline_update)
{
	int const scanline = param;

	if (scanline == 0) logerror("-------\n");

	/* keep in range */
	int offset = (scanline / 8) * 64 + 48;
	if (offset >= 0x800)
		return;

	/* update the playfield scrolls */
	for (int i = 0; i < 8; i++)
	{
		uint16_t word = m_alpha_tilemap->basemem_read(offset++);
		if (BIT(word, 15))
		{
			int const newscroll = (word >> 5) & 0x3ff;
			int const newbank = (word >> 0) & 0x1f;
			if (newscroll != m_playfield_xscroll)
			{
				if (scanline + i > 0)
					m_screen->update_partial(scanline + i - 1);
				m_playfield_tilemap->set_scrollx(0, newscroll);
				m_playfield_xscroll = newscroll;
			}
			if (newbank != m_playfield_color_bank)
			{
				if (scanline + i > 0)
					m_screen->update_partial(scanline + i - 1);
				m_playfield_tilemap->mark_all_dirty();
				m_playfield_color_bank = newbank;
			}
		}

		word = m_alpha_tilemap->basemem_read(offset++);
		if (BIT(word, 15))
		{
			int const newscroll = ((word >> 6) - (scanline + i)) & 0x1ff;
			int const newbank = word & 15;
			if (newscroll != m_playfield_yscroll)
			{
				if (scanline + i > 0)
					m_screen->update_partial(scanline + i - 1);
				m_playfield_tilemap->set_scrolly(0, newscroll);
				m_playfield_yscroll = newscroll;
			}
			if (newbank != m_playfield_tile_bank)
			{
				if (scanline + i > 0)
					m_screen->update_partial(scanline + i - 1);
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

uint32_t atarigx2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
		int const left    = cliprect.left();
		int const top     = cliprect.top();
		int const right   = cliprect.right() + 1;
		int const bottom  = cliprect.bottom() + 1;

		/* now blend with the playfield */
		for (int y = top; y < bottom; y++)
		{
			uint16_t *const pf = &bitmap.pix(y);
			uint16_t const *const mo = &mo_bitmap.pix(y);
			uint8_t const *const pri = &priority_bitmap.pix(y);
			for (int x = left; x < right; x++)
				if (mo[x] && (mo[x] >> atari_rle_objects_device::PRIORITY_SHIFT) >= pri[x])
					pf[x] = mo[x] & atari_rle_objects_device::DATA_MASK;
		}
	}

	/* add the alpha on top */
	m_alpha_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
