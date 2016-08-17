// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "emu.h"
#include "includes/fastlane.h"


PALETTE_INIT_MEMBER(fastlane_state, fastlane)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int pal;

	for (pal = 0; pal < 0x10; pal++)
	{
		int i;

		for (i = 0; i < 0x400; i++)
		{
			UINT8 ctabentry = (i & 0x3f0) | color_prom[(pal << 4) | (i & 0x0f)];
			palette.set_pen_indirect((pal << 10) | i, ctabentry);
		}
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(fastlane_state::get_tile_info0)
{
	UINT8 ctrl_3 = m_k007121->ctrlram_r(generic_space(), 3);
	UINT8 ctrl_4 = m_k007121->ctrlram_r(generic_space(), 4);
	UINT8 ctrl_5 = m_k007121->ctrlram_r(generic_space(), 5);
	int attr = m_videoram1[tile_index];
	int code = m_videoram1[tile_index + 0x400];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	SET_TILE_INFO_MEMBER(0,
			code+bank*256,
			1 + 64 * (attr & 0x0f),
			0);
}

TILE_GET_INFO_MEMBER(fastlane_state::get_tile_info1)
{
	UINT8 ctrl_3 = m_k007121->ctrlram_r(generic_space(), 3);
	UINT8 ctrl_4 = m_k007121->ctrlram_r(generic_space(), 4);
	UINT8 ctrl_5 = m_k007121->ctrlram_r(generic_space(), 5);
	int attr = m_videoram2[tile_index];
	int code = m_videoram2[tile_index + 0x400];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0+2)) & 0x02) |
			((attr >> (bit1+1)) & 0x04) |
			((attr >> (bit2  )) & 0x08) |
			((attr >> (bit3-1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	SET_TILE_INFO_MEMBER(0,
			code+bank*256,
			0 + 64 * (attr & 0x0f),
			0);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void fastlane_state::video_start()
{
	m_layer0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fastlane_state::get_tile_info0),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_layer1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fastlane_state::get_tile_info1),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_layer0->set_scroll_rows(32);

	m_clip0 = m_screen->visible_area();
	m_clip0.min_x += 40;

	m_clip1 = m_screen->visible_area();
	m_clip1.max_x = 39;
	m_clip1.min_x = 0;
}

/***************************************************************************

  Memory Handlers

***************************************************************************/

WRITE8_MEMBER(fastlane_state::fastlane_vram1_w)
{
	m_videoram1[offset] = data;
	m_layer0->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(fastlane_state::fastlane_vram2_w)
{
	m_videoram2[offset] = data;
	m_layer1->mark_tile_dirty(offset & 0x3ff);
}



/***************************************************************************

  Screen Refresh

***************************************************************************/

UINT32 fastlane_state::screen_update_fastlane(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle finalclip0 = m_clip0, finalclip1 = m_clip1;
	int i, xoffs;

	finalclip0 &= cliprect;
	finalclip1 &= cliprect;

	/* set scroll registers */
	address_space &space = machine().driver_data()->generic_space();
	xoffs = m_k007121->ctrlram_r(space, 0);
	for (i = 0; i < 32; i++)
		m_layer0->set_scrollx(i, m_k007121_regs[0x20 + i] + xoffs - 40);

	m_layer0->set_scrolly(0, m_k007121->ctrlram_r(space, 2));

	m_layer0->draw(screen, bitmap, finalclip0, 0, 0);
	m_k007121->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(0), m_palette, m_spriteram, 0, 40, 0, screen.priority(), (UINT32)-1);
	m_layer1->draw(screen, bitmap, finalclip1, 0, 0);
	return 0;
}
