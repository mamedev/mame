// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Phil Stroffolino
/***************************************************************************

    video/contra.c

***************************************************************************/

#include "emu.h"
#include "includes/contra.h"


/***************************************************************************
**
**  Contra has palette RAM, but it also has four lookup table PROMs
**
**  0   sprites #0
**  1   tiles   #0
**  2   sprites #1
**  3   tiles   #1
**
***************************************************************************/

PALETTE_INIT_MEMBER(contra_state, contra)
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

TILE_GET_INFO_MEMBER(contra_state::get_fg_tile_info)
{
	UINT8 ctrl_3 = m_k007121_1->ctrlram_r(generic_space(), 3);
	UINT8 ctrl_4 = m_k007121_1->ctrlram_r(generic_space(), 4);
	UINT8 ctrl_5 = m_k007121_1->ctrlram_r(generic_space(), 5);
	UINT8 ctrl_6 = m_k007121_1->ctrlram_r(generic_space(), 6);
	int attr = m_fg_cram[tile_index];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	SET_TILE_INFO_MEMBER(0,
			m_fg_vram[tile_index] + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}

TILE_GET_INFO_MEMBER(contra_state::get_bg_tile_info)
{
	UINT8 ctrl_3 = m_k007121_2->ctrlram_r(generic_space(), 3);
	UINT8 ctrl_4 = m_k007121_2->ctrlram_r(generic_space(), 4);
	UINT8 ctrl_5 = m_k007121_2->ctrlram_r(generic_space(), 5);
	UINT8 ctrl_6 = m_k007121_2->ctrlram_r(generic_space(), 6);
	int attr = m_bg_cram[tile_index];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	// 2009-12 FP: TO BE VERIFIED - old code used ctrl4 from chip 0?!?
	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	SET_TILE_INFO_MEMBER(1,
			m_bg_vram[tile_index] + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}

TILE_GET_INFO_MEMBER(contra_state::get_tx_tile_info)
{
	UINT8 ctrl_5 = m_k007121_1->ctrlram_r(generic_space(), 5);
	UINT8 ctrl_6 = m_k007121_1->ctrlram_r(generic_space(), 6);
	int attr = m_tx_cram[tile_index];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10);

	SET_TILE_INFO_MEMBER(0,
			m_tx_vram[tile_index] + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void contra_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(contra_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(contra_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(contra_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_buffered_spriteram = auto_alloc_array(machine(), UINT8, 0x800);
	m_buffered_spriteram_2 = auto_alloc_array(machine(), UINT8, 0x800);

	m_bg_clip = m_screen->visible_area();
	m_bg_clip.min_x += 40;

	m_fg_clip = m_bg_clip;

	m_tx_clip = m_screen->visible_area();
	m_tx_clip.max_x = 39;
	m_tx_clip.min_x = 0;

	m_fg_tilemap->set_transparent_pen(0);

	save_pointer(NAME(m_buffered_spriteram), 0x800);
	save_pointer(NAME(m_buffered_spriteram_2), 0x800);
}


/***************************************************************************

    Memory handlers

***************************************************************************/

WRITE8_MEMBER(contra_state::contra_fg_vram_w)
{
	m_fg_vram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(contra_state::contra_fg_cram_w)
{
	m_fg_cram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(contra_state::contra_bg_vram_w)
{
	m_bg_vram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(contra_state::contra_bg_cram_w)
{
	m_bg_cram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(contra_state::contra_text_vram_w)
{
	m_tx_vram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(contra_state::contra_text_cram_w)
{
	m_tx_cram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(contra_state::contra_K007121_ctrl_0_w)
{
	UINT8 ctrl_6 = m_k007121_1->ctrlram_r(space, 6);

	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			memcpy(m_buffered_spriteram, m_spriteram + 0x800, 0x800);
		else
			memcpy(m_buffered_spriteram, m_spriteram, 0x800);
	}

	if (offset == 6)
	{
		if (ctrl_6 != data)
			m_fg_tilemap->mark_all_dirty();
	}

	if (offset == 7)
		m_fg_tilemap->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_k007121_1->ctrl_w(space, offset, data);
}

WRITE8_MEMBER(contra_state::contra_K007121_ctrl_1_w)
{
	UINT8 ctrl_6 = m_k007121_2->ctrlram_r(space, 6);

	if (offset == 3)
	{
		if ((data & 0x8) == 0)
			memcpy(m_buffered_spriteram_2, m_spriteram + 0x2800, 0x800);
		else
			memcpy(m_buffered_spriteram_2, m_spriteram + 0x2000, 0x800);
	}
	if (offset == 6)
	{
		if (ctrl_6 != data )
			m_bg_tilemap->mark_all_dirty();
	}
	if (offset == 7)
		m_bg_tilemap->set_flip((data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_k007121_2->ctrl_w(space, offset, data);
}



/***************************************************************************

    Display Refresh

***************************************************************************/

void contra_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int bank )
{
	k007121_device *k007121 = bank ? m_k007121_2 : m_k007121_1;
	address_space &space = machine().driver_data()->generic_space();
	int base_color = (k007121->ctrlram_r(space, 6) & 0x30) * 2;
	const UINT8 *source;

	if (bank == 0)
		source = m_buffered_spriteram;
	else
		source = m_buffered_spriteram_2;

	k007121->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(bank), m_palette, source, base_color, 40, 0, priority_bitmap, (UINT32)-1);
}

UINT32 contra_state::screen_update_contra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT8 ctrl_1_0 = m_k007121_1->ctrlram_r(space, 0);
	UINT8 ctrl_1_2 = m_k007121_1->ctrlram_r(space, 2);
	UINT8 ctrl_2_0 = m_k007121_2->ctrlram_r(space, 0);
	UINT8 ctrl_2_2 = m_k007121_2->ctrlram_r(space, 2);
	rectangle bg_finalclip = m_bg_clip;
	rectangle fg_finalclip = m_fg_clip;
	rectangle tx_finalclip = m_tx_clip;

	bg_finalclip &= cliprect;
	fg_finalclip &= cliprect;
	tx_finalclip &= cliprect;

	m_fg_tilemap->set_scrollx(0, ctrl_1_0 - 40);
	m_fg_tilemap->set_scrolly(0, ctrl_1_2);
	m_bg_tilemap->set_scrollx(0, ctrl_2_0 - 40);
	m_bg_tilemap->set_scrolly(0, ctrl_2_2);

	m_bg_tilemap->draw(screen, bitmap, bg_finalclip, 0 ,0);
	m_fg_tilemap->draw(screen, bitmap, fg_finalclip, 0 ,0);
	draw_sprites(bitmap,cliprect, screen.priority(), 0);
	draw_sprites(bitmap,cliprect, screen.priority(), 1);
	m_tx_tilemap->draw(screen, bitmap, tx_finalclip, 0 ,0);
	return 0;
}
