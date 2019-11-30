// license:BSD-3-Clause
// copyright-holders:Uki
/*******************************************************************************

XX Mission (c) 1986 UPL

Video hardware driver by Uki

    31/Mar/2001 -

*******************************************************************************/

#include "emu.h"
#include "includes/xxmissio.h"


WRITE8_MEMBER(xxmissio_state::scroll_x_w)
{
	m_xscroll = data;
}
WRITE8_MEMBER(xxmissio_state::scroll_y_w)
{
	m_yscroll = data;
}

WRITE8_MEMBER(xxmissio_state::flipscreen_w)
{
	m_flipscreen = data & 0x01;
}

WRITE8_MEMBER(xxmissio_state::bgram_w)
{
	int x = (offset + (m_xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	m_bgram[offset] = data;
}
READ8_MEMBER(xxmissio_state::bgram_r)
{
	int x = (offset + (m_xscroll >> 3)) & 0x1f;
	offset = (offset & 0x7e0) | x;

	return m_bgram[offset];
}

/****************************************************************************/

TILE_GET_INFO_MEMBER(xxmissio_state::get_bg_tile_info)
{
	int code = ((m_bgram[0x400 | tile_index] & 0xc0) << 2) | m_bgram[0x000 | tile_index];
	int color =  m_bgram[0x400 | tile_index] & 0x0f;

	SET_TILE_INFO_MEMBER(2, code, color, 0);
}

TILE_GET_INFO_MEMBER(xxmissio_state::get_fg_tile_info)
{
	int code = m_fgram[0x000 | tile_index];
	int color = m_fgram[0x400 | tile_index] & 0x07;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void xxmissio_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xxmissio_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xxmissio_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);

	m_bg_tilemap->set_scroll_cols(1);
	m_bg_tilemap->set_scroll_rows(1);
	m_bg_tilemap->set_scrolldx(2, 12);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	save_item(NAME(m_flipscreen));
}

rgb_t xxmissio_state::BBGGRRII(uint32_t raw)
{
	uint8_t const i = raw & 3;
	uint8_t const r = ((raw >> 0) & 0x0c) | i;
	uint8_t const g = ((raw >> 2) & 0x0c) | i;
	uint8_t const b = ((raw >> 4) & 0x0c) | i;

	return rgb_t(r | (r << 4), g | (g << 4), b | (b << 4));
}

void xxmissio_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx)
{
	for (int offs = 0; offs < 0x800; offs += 0x20)
	{
		int chr = m_spriteram[offs];
		int col = m_spriteram[offs+3];

		int const fx = BIT(col, 4) ^ m_flipscreen;
		int const fy = BIT(col, 5) ^ m_flipscreen;

		int const x = m_spriteram[offs+1]*2;
		int const y = m_spriteram[offs+2];

		chr += (col & 0x40) << 2;
		col &= 0x07;

		int px, py;
		if (!m_flipscreen)
		{
			px = x-8;
			py = y;
		}
		else
		{
			px = 480-x-6;
			py = 240-y;
		}

		px &= 0x1ff;

		gfx->transpen(bitmap,cliprect,
			chr,
			col,
			fx,fy,
			px,py,0);

		if (px > 0x1e0)
			gfx->transpen(bitmap,cliprect,
				chr,
				col,
				fx,fy,
				px-0x200,py,0);

	}
}


uint32_t xxmissio_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().mark_all_dirty();
	machine().tilemap().set_flip_all(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	m_bg_tilemap->set_scrollx(0, m_xscroll * 2);
	m_bg_tilemap->set_scrolly(0, m_yscroll);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(1));
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
