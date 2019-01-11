// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Karnov - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/karnov.h"

void karnov_state::karnov_flipscreen_w( int data )
{
	m_flipscreen = data;
	m_fix_tilemap->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_spritegen->set_flip_screen(m_flipscreen);
}

void karnov_state::draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int my, mx, offs, color, tile, fx, fy;
	int scrollx = m_scroll[0];
	int scrolly = m_scroll[1];

	if (m_flipscreen)
		fx = fy = 1;
	else
		fx = fy = 0;

	mx = -1;
	my = 0;

	for (offs = 0; offs < 0x400; offs ++)
	{
		mx++;
		if (mx == 32)
		{
			mx=0;
			my++;
		}

		tile = m_pf_data[offs];
		color = tile >> 12;
		tile = tile & 0x7ff;
		if (m_flipscreen)
			m_gfxdecode->gfx(1)->opaque(*m_bitmap_f,m_bitmap_f->cliprect(),tile,
				color, fx, fy, 496-16*mx,496-16*my);
		else
			m_gfxdecode->gfx(1)->opaque(*m_bitmap_f,m_bitmap_f->cliprect(),tile,
				color, fx, fy, 16*mx,16*my);
	}

	if (!m_flipscreen)
	{
		scrolly = -scrolly;
		scrollx = -scrollx;
	}
	else
	{
		scrolly = scrolly + 256;
		scrollx = scrollx + 256;
	}

	copyscrollbitmap(bitmap, *m_bitmap_f, 1, &scrollx, 1, &scrolly, cliprect);
}

/******************************************************************************/

uint32_t karnov_state::screen_update_karnov(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(bitmap, cliprect);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x800, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/******************************************************************************/

TILE_GET_INFO_MEMBER(karnov_state::get_fix_tile_info)
{
	int tile = m_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			tile&0xfff,
			tile>>14,
			0);
}

WRITE16_MEMBER(karnov_state::karnov_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_fix_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(karnov_state::karnov_playfield_swap_w)
{
	offset = ((offset & 0x1f) << 5) | ((offset & 0x3e0) >> 5);
	COMBINE_DATA(&m_pf_data[offset]);
}

/******************************************************************************/

VIDEO_START_MEMBER(karnov_state,karnov)
{
	/* Allocate bitmap & tilemap */
	m_bitmap_f = std::make_unique<bitmap_ind16>(512, 512);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(karnov_state::get_fix_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	save_item(NAME(*m_bitmap_f));

	m_fix_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(karnov_state,wndrplnt)
{
	/* Allocate bitmap & tilemap */
	m_bitmap_f = std::make_unique<bitmap_ind16>(512, 512);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(karnov_state::get_fix_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	save_item(NAME(*m_bitmap_f));

	m_fix_tilemap->set_transparent_pen(0);
}

/******************************************************************************/
