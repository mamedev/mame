// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/tail2nos.h"


#define TOTAL_CHARS 0x400

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(tail2nos_state::get_tile_info)
{
	UINT16 code = m_txvideoram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			(code & 0x1fff) + (m_txbank << 13),
			((code & 0xe000) >> 13) + m_txpalette * 16,
			0);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

K051316_CB_MEMBER(tail2nos_state::zoom_callback)
{
	*code |= ((*color & 0x03) << 8);
	*color = 32 + ((*color & 0x38) >> 3);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void tail2nos_state::tail2nos_postload()
{
	m_tx_tilemap->mark_all_dirty();

	m_k051316->gfx(0)->mark_all_dirty();
}

void tail2nos_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tail2nos_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tx_tilemap->set_transparent_pen(15);

	machine().save().register_postload(save_prepost_delegate(FUNC(tail2nos_state::tail2nos_postload), this));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(tail2nos_state::tail2nos_txvideoram_w)
{
	COMBINE_DATA(&m_txvideoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tail2nos_state::tail2nos_zoomdata_w)
{
	int oldword = m_zoomram[offset];
	COMBINE_DATA(&m_zoomram[offset]);
	// tell the K051316 device the data changed
	if (oldword != m_zoomram[offset])
		m_k051316->mark_gfx_dirty(offset * 2);
}

WRITE16_MEMBER(tail2nos_state::tail2nos_gfxbank_w)
{
	// -------- --pe-b-b
	// p = palette bank
	// b = tile bank
	// e = video enable

	if (ACCESSING_BITS_0_7)
	{
		int bank;
		bank = 0;
		/* bits 0 and 2 select char bank */
		if (data & 0x04) bank |= 2;
		if (data & 0x01) bank |= 1;


		if (m_txbank != bank)
		{
			m_txbank = bank;
			m_tx_tilemap->mark_all_dirty();
		}

		/* bit 5 seems to select palette bank (used on startup) */
		if (data & 0x20)
			bank = 7;
		else
			bank = 3;

		if (m_txpalette != bank)
		{
			m_txpalette = bank;
			m_tx_tilemap->mark_all_dirty();
		}

		/* bit 4 seems to be video enable */
		m_video_enable = data & 0x10;
	}
}


/***************************************************************************

    Display Refresh

***************************************************************************/

void tail2nos_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT16 *spriteram = m_spriteram;
	int offs;


	for (offs = 0; offs < m_spriteram.bytes() / 2; offs += 4)
	{
		int sx, sy, flipx, flipy, code, color;

		sx = spriteram[offs + 1];
		if (sx >= 0x8000)
			sx -= 0x10000;
		sy = 0x10000 - spriteram[offs + 0];
		if (sy >= 0x8000)
			sy -= 0x10000;
		code = spriteram[offs + 2] & 0x07ff;
		color = (spriteram[offs + 2] & 0xe000) >> 13;
		flipx = spriteram[offs + 2] & 0x1000;
		flipy = spriteram[offs + 2] & 0x0800;

		m_gfxdecode->gfx(1)->transpen(bitmap,/* placement relative to zoom layer verified on the real thing */
				cliprect,
				code,
				40 + color,
				flipx,flipy,
				sx+3,sy+1,15);
	}
}

UINT32 tail2nos_state::screen_update_tail2nos(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video_enable)
	{
		m_k051316->zoom_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap, cliprect);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(0, cliprect);

	return 0;
}
