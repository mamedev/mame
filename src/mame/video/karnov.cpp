// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Karnov - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/karnov.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Karnov has two 1024x8 palette PROM.
  I don't know the exact values of the resistors between the RAM and the
  RGB output. I assumed these values (the same as Commando)

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 7 -- unused
        -- unused
        -- unused
        -- unused
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(karnov_state, karnov)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[palette.entries()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

void karnov_state::karnov_flipscreen_w( int data )
{
	m_flipscreen = data;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	flip_screen_set(m_flipscreen);
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

UINT32 karnov_state::screen_update_karnov(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(karnov_state::get_fix_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	save_item(NAME(*m_bitmap_f));

	m_fix_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(karnov_state,wndrplnt)
{
	/* Allocate bitmap & tilemap */
	m_bitmap_f = std::make_unique<bitmap_ind16>(512, 512);
	m_fix_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(karnov_state::get_fix_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	save_item(NAME(*m_bitmap_f));

	m_fix_tilemap->set_transparent_pen(0);
}

/******************************************************************************/
