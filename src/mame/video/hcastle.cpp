// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Haunted Castle video emulation

***************************************************************************/

#include "emu.h"
#include "includes/hcastle.h"


PALETTE_INIT_MEMBER(hcastle_state, hcastle)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int chip;

	for (chip = 0; chip < 2; chip++)
	{
		int pal;

		for (pal = 0; pal < 8; pal++)
		{
			int i;
			int clut = (chip << 1) | (pal & 1);

			for (i = 0; i < 0x100; i++)
			{
				UINT8 ctabentry;

				if (((pal & 0x01) == 0) && (color_prom[(clut << 8) | i] == 0))
					ctabentry = 0;
				else
					ctabentry = (pal << 4) | (color_prom[(clut << 8) | i] & 0x0f);

				palette.set_pen_indirect((chip << 11) | (pal << 8) | i, ctabentry);
			}
		}
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(hcastle_state::tilemap_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 6);    /* skip 0x400 */
}

TILE_GET_INFO_MEMBER(hcastle_state::get_fg_tile_info)
{
	UINT8 ctrl_5 = m_k007121_1->ctrlram_r(generic_space(), 5);
	UINT8 ctrl_6 = m_k007121_1->ctrlram_r(generic_space(), 6);
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int attr = m_pf1_videoram[tile_index];
	int tile = m_pf1_videoram[tile_index + 0x400];
	int color = attr & 0x7;
	int bank =  ((attr & 0x80) >> 7) |
				((attr >> (bit0 + 2)) & 0x02) |
				((attr >> (bit1 + 1)) & 0x04) |
				((attr >> (bit2    )) & 0x08) |
				((attr >> (bit3 - 1)) & 0x10);

	SET_TILE_INFO_MEMBER(0,
			tile + bank * 0x100 + m_pf1_bankbase,
			((ctrl_6 & 0x30) * 2 + 16) + color,
			0);
}

TILE_GET_INFO_MEMBER(hcastle_state::get_bg_tile_info)
{
	UINT8 ctrl_5 = m_k007121_2->ctrlram_r(generic_space(), 5);
	UINT8 ctrl_6 = m_k007121_2->ctrlram_r(generic_space(), 6);
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int attr = m_pf2_videoram[tile_index];
	int tile = m_pf2_videoram[tile_index + 0x400];
	int color = attr & 0x7;
	int bank =  ((attr & 0x80) >> 7) |
				((attr >> (bit0 + 2)) & 0x02) |
				((attr >> (bit1 + 1)) & 0x04) |
				((attr >> (bit2    )) & 0x08) |
				((attr >> (bit3 - 1)) & 0x10);

	SET_TILE_INFO_MEMBER(1,
			tile + bank * 0x100 + m_pf2_bankbase,
			((ctrl_6 & 0x30) * 2 + 16) + color,
			0);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void hcastle_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(hcastle_state::get_fg_tile_info),this), tilemap_mapper_delegate(FUNC(hcastle_state::tilemap_scan),this), 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(hcastle_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(hcastle_state::tilemap_scan),this), 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);
}



/***************************************************************************

    Memory handlers

***************************************************************************/

WRITE8_MEMBER(hcastle_state::hcastle_pf1_video_w)
{
	m_pf1_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0xbff);
}

WRITE8_MEMBER(hcastle_state::hcastle_pf2_video_w)
{
	m_pf2_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0xbff);
}

WRITE8_MEMBER(hcastle_state::hcastle_gfxbank_w)
{
	m_gfx_bank = data;
}

READ8_MEMBER(hcastle_state::hcastle_gfxbank_r)
{
	return m_gfx_bank;
}

WRITE8_MEMBER(hcastle_state::hcastle_pf1_control_w)
{
	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			m_spriteram->copy(0x800, 0x800);
		else
			m_spriteram->copy(0x000, 0x800);
	}
	else if (offset == 7)
	{
		m_fg_tilemap->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}
	m_k007121_1->ctrl_w(space, offset, data);
}

WRITE8_MEMBER(hcastle_state::hcastle_pf2_control_w)
{
	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			m_spriteram2->copy(0x800, 0x800);
		else
			m_spriteram2->copy(0x000, 0x800);
	}
	else if (offset == 7)
	{
		m_bg_tilemap->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}
	m_k007121_2->ctrl_w(space, offset, data);
}

/*****************************************************************************/

void hcastle_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, UINT8 *sbank, int bank )
{
	k007121_device *k007121 = bank ? m_k007121_2 : m_k007121_1;
	address_space &space = machine().driver_data()->generic_space();
	int base_color = (k007121->ctrlram_r(space, 6) & 0x30) * 2;
	int bank_base = (bank == 0) ? 0x4000 * (m_gfx_bank & 1) : 0;

	k007121->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(bank), m_palette, sbank, base_color, 0, bank_base, priority_bitmap, (UINT32)-1);
}

/*****************************************************************************/

UINT32 hcastle_state::screen_update_hcastle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();

	UINT8 ctrl_1_0 = m_k007121_1->ctrlram_r(space, 0);
	UINT8 ctrl_1_1 = m_k007121_1->ctrlram_r(space, 1);
	UINT8 ctrl_1_2 = m_k007121_1->ctrlram_r(space, 2);
	UINT8 ctrl_1_3 = m_k007121_1->ctrlram_r(space, 3);
	UINT8 ctrl_2_0 = m_k007121_2->ctrlram_r(space, 0);
	UINT8 ctrl_2_1 = m_k007121_2->ctrlram_r(space, 1);
	UINT8 ctrl_2_2 = m_k007121_2->ctrlram_r(space, 2);
	UINT8 ctrl_2_3 = m_k007121_2->ctrlram_r(space, 3);

	m_pf1_bankbase = 0x0000;
	m_pf2_bankbase = 0x4000 * ((m_gfx_bank & 2) >> 1);

	if (ctrl_1_3 & 0x01)
		m_pf1_bankbase += 0x2000;
	if (ctrl_2_3 & 0x01)
		m_pf2_bankbase += 0x2000;

	if (m_pf1_bankbase != m_old_pf1)
		m_fg_tilemap->mark_all_dirty();

	if (m_pf2_bankbase != m_old_pf2)
		m_bg_tilemap->mark_all_dirty();

	m_old_pf1 = m_pf1_bankbase;
	m_old_pf2 = m_pf2_bankbase;

	m_bg_tilemap->set_scrolly(0, ctrl_2_2);
	m_bg_tilemap->set_scrollx(0, ((ctrl_2_1 << 8) + ctrl_2_0));
	m_fg_tilemap->set_scrolly(0, ctrl_1_2);
	m_fg_tilemap->set_scrollx(0, ((ctrl_1_1 << 8) + ctrl_1_0));

//  /* Sprite priority */
//  if (ctrl_1_3 & 0x20)
	if ((m_gfx_bank & 0x04) == 0)
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect, screen.priority(), m_spriteram->buffer(), 0);
		draw_sprites(bitmap, cliprect, screen.priority(), m_spriteram2->buffer(), 1);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect, screen.priority(), m_spriteram->buffer(), 0);
		draw_sprites(bitmap, cliprect, screen.priority(), m_spriteram2->buffer(), 1);
	}
	return 0;
}
