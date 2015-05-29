// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"

#include "includes/labyrunr.h"

PALETTE_INIT_MEMBER(labyrunr_state, labyrunr)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int pal;

	for (pal = 0; pal < 8; pal++)
	{
		/* chars, no lookup table */
		if (pal & 1)
		{
			int i;

			for (i = 0; i < 0x100; i++)
				palette.set_pen_indirect((pal << 8) | i, (pal << 4) | (i & 0x0f));
		}
		/* sprites */
		else
		{
			int i;

			for (i = 0; i < 0x100; i++)
			{
				UINT8 ctabentry;

				if (color_prom[i] == 0)
					ctabentry = 0;
				else
					ctabentry = (pal << 4) | (color_prom[i] & 0x0f);

				palette.set_pen_indirect((pal << 8) | i, ctabentry);
			}
		}
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(labyrunr_state::get_tile_info0)
{
	UINT8 ctrl_3 = m_k007121->ctrlram_r(generic_space(), 3);
	UINT8 ctrl_4 = m_k007121->ctrlram_r(generic_space(), 4);
	UINT8 ctrl_5 = m_k007121->ctrlram_r(generic_space(), 5);
	UINT8 ctrl_6 = m_k007121->ctrlram_r(generic_space(), 6);
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
			code + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16)+(attr & 7),
			0);
}

TILE_GET_INFO_MEMBER(labyrunr_state::get_tile_info1)
{
	UINT8 ctrl_3 = m_k007121->ctrlram_r(generic_space(), 3);
	UINT8 ctrl_4 = m_k007121->ctrlram_r(generic_space(), 4);
	UINT8 ctrl_5 = m_k007121->ctrlram_r(generic_space(), 5);
	UINT8 ctrl_6 = m_k007121->ctrlram_r(generic_space(), 6);
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
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void labyrunr_state::video_start()
{
	m_layer0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(labyrunr_state::get_tile_info0),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_layer1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(labyrunr_state::get_tile_info1),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_layer0->set_transparent_pen(0);
	m_layer1->set_transparent_pen(0);

	m_clip0 = m_screen->visible_area();
	m_clip0.min_x += 40;

	m_clip1 = m_screen->visible_area();
	m_clip1.max_x = 39;
	m_clip1.min_x = 0;

	m_layer0->set_scroll_cols(32);
}



/***************************************************************************

  Memory Handlers

***************************************************************************/

WRITE8_MEMBER(labyrunr_state::labyrunr_vram1_w)
{
	m_videoram1[offset] = data;
	m_layer0->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(labyrunr_state::labyrunr_vram2_w)
{
	m_videoram2[offset] = data;
	m_layer1->mark_tile_dirty(offset & 0x3ff);
}



/***************************************************************************

  Screen Refresh

***************************************************************************/

UINT32 labyrunr_state::screen_update_labyrunr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT8 ctrl_0 = m_k007121->ctrlram_r(space, 0);
	rectangle finalclip0, finalclip1;

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (~m_k007121->ctrlram_r(space, 3) & 0x20)
	{
		int i;

		finalclip0 = m_clip0;
		finalclip1 = m_clip1;

		finalclip0 &= cliprect;
		finalclip1 &= cliprect;

		m_layer0->set_scrollx(0, ctrl_0 - 40);
		m_layer1->set_scrollx(0, 0);

		for(i = 0; i < 32; i++)
		{
			/* enable colscroll */
			if((m_k007121->ctrlram_r(space, 1) & 6) == 6) // it's probably just one bit, but it's only used once in the game so I don't know which it's
				m_layer0->set_scrolly((i + 2) & 0x1f, m_k007121->ctrlram_r(space, 2) + m_scrollram[i]);
			else
				m_layer0->set_scrolly((i + 2) & 0x1f, m_k007121->ctrlram_r(space, 2));
		}

		m_layer0->draw(screen, bitmap, finalclip0, TILEMAP_DRAW_OPAQUE, 0);
		m_k007121->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(0), m_palette, m_spriteram,(m_k007121->ctrlram_r(space, 6) & 0x30) * 2, 40,0,screen.priority(),(m_k007121->ctrlram_r(space, 3) & 0x40) >> 5);
		/* we ignore the transparency because layer1 is drawn only at the top of the screen also covering sprites */
		m_layer1->draw(screen, bitmap, finalclip1, TILEMAP_DRAW_OPAQUE, 0);
	}
	else
	{
		int use_clip3[2] = { 0, 0 };
		rectangle finalclip3;

		/* custom cliprects needed for the weird effect used in the endinq sequence to hide and show the needed part of text */
		finalclip0.min_y = finalclip1.min_y = cliprect.min_y;
		finalclip0.max_y = finalclip1.max_y = cliprect.max_y;

		if(m_k007121->ctrlram_r(space, 1) & 1)
		{
			finalclip0.min_x = cliprect.max_x - ctrl_0 + 8;
			finalclip0.max_x = cliprect.max_x;

			if(ctrl_0 >= 40)
			{
				finalclip1.min_x = cliprect.min_x;
			}
			else
			{
				use_clip3[0] = 1;

				finalclip1.min_x = 40 - ctrl_0;
			}

			finalclip1.max_x = cliprect.max_x - ctrl_0 + 8;

		}
		else
		{
			if(ctrl_0 >= 40)
			{
				finalclip0.min_x = cliprect.min_x;
			}
			else
			{
				use_clip3[1] = 1;

				finalclip0.min_x = 40 - ctrl_0;
			}

			finalclip0.max_x = cliprect.max_x - ctrl_0 + 8;

			finalclip1.min_x = cliprect.max_x - ctrl_0 + 8;
			finalclip1.max_x = cliprect.max_x;
		}

		if(use_clip3[0] || use_clip3[1])
		{
			finalclip3.min_y = cliprect.min_y;
			finalclip3.max_y = cliprect.max_y;
			finalclip3.min_x = cliprect.min_x;
			finalclip3.max_x = 40 - ctrl_0 - 8;
		}

		m_layer0->set_scrollx(0, ctrl_0 - 40);
		m_layer1->set_scrollx(0, ctrl_0 - 40);

		m_layer0->draw(screen, bitmap, finalclip0, 0, 1);
		if(use_clip3[0])
			m_layer0->draw(screen, bitmap, finalclip3, 0, 1);

		m_layer1->draw(screen, bitmap, finalclip1, 0, 1);
		if(use_clip3[1])
			m_layer1->draw(screen, bitmap, finalclip3, 0, 1);

		m_k007121->sprites_draw(bitmap, cliprect, m_gfxdecode->gfx(0), m_palette, m_spriteram, (m_k007121->ctrlram_r(space, 6) & 0x30) * 2,40,0,screen.priority(),(m_k007121->ctrlram_r(space, 3) & 0x40) >> 5);
	}
	return 0;
}
