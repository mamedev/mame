// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

  Knuckle Joe

***************************************************************************/

#include "emu.h"
#include "includes/kncljoe.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

void kncljoe_state::kncljoe_palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x80; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (int i = 0; i < 0x10; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = 0;
		bit1 = BIT(color_prom[i + 0x300], 6);
		bit2 = BIT(color_prom[i + 0x300], 7);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i + 0x300], 3);
		bit1 = BIT(color_prom[i + 0x300], 4);
		bit2 = BIT(color_prom[i + 0x300], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i + 0x300], 0);
		bit1 = BIT(color_prom[i + 0x300], 1);
		bit2 = BIT(color_prom[i + 0x300], 2);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i + 0x80, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x320;

	// chars
	for (int i = 0; i < 0x80; i++)
		palette.set_pen_indirect(i, i);

	// sprite lookup table
	for (int i = 0; i < 0x80; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x80;
		palette.set_pen_indirect(i + 0x80, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(kncljoe_state::get_bg_tile_info)
{
	int attr = m_videoram[2 * tile_index + 1];
	int code = m_videoram[2 * tile_index] + ((attr & 0xc0) << 2) + (m_tile_bank << 10);

	tileinfo.set(0,
			code,
			attr & 0xf,
			TILE_FLIPXY((attr & 0x30) >> 4));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void kncljoe_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(kncljoe_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap->set_scroll_rows(4);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void kncljoe_state::kncljoe_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void kncljoe_state::kncljoe_control_w(uint8_t data)
{
	/*
	        0x01    screen flip
	        0x02    coin counter#1
	        0x04    sprite bank
	        0x10    character bank
	        0x20    coin counter#2

	        reset when IN0 - Coin 1 goes low (active)
	        set after IN0 - Coin 1 goes high AND the credit has been added
	*/
	m_flipscreen = data & 0x01;
	machine().tilemap().set_flip_all(m_flipscreen ? TILEMAP_FLIPX : TILEMAP_FLIPY);

	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);

	if (m_tile_bank != BIT(data, 4))
	{
		m_tile_bank = BIT(data, 4);
		m_bg_tilemap->mark_all_dirty();
	}

	m_sprite_bank = BIT(data, 2);
}

void kncljoe_state::kncljoe_scroll_w(offs_t offset, uint8_t data)
{
	m_scrollregs[offset] = data;
	int scrollx = m_scrollregs[0] | m_scrollregs[1] << 8;
	m_bg_tilemap->set_scrollx(0, scrollx);
	m_bg_tilemap->set_scrollx(1, scrollx);
	m_bg_tilemap->set_scrollx(2, scrollx);
	m_bg_tilemap->set_scrollx(3, 0);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void kncljoe_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(1 + m_sprite_bank);

	for (int i = 0; i < 4; i++)
	{
		// clip vertical strip for each layer
		rectangle clip = cliprect;
		clip.min_y = m_flipscreen ? (191 - i * 64) : (i * 64 + 1);
		clip.max_y = clip.min_y + 63;
		clip &= cliprect;

		for (int j = 0x7c; j >= 0; j -= 4)
		{
			int offs = bitswap<2>(~i, 0, 1) << 7 | j;
			int sy = m_spriteram[offs] + 1;
			int sx = m_spriteram[offs + 3];
			int attr = m_spriteram[offs + 1];
			int code = m_spriteram[offs + 2] | ((attr & 0x10) << 5) | ((attr & 0x20) << 3);
			int flipx = attr & 0x40;
			int flipy = !(attr & 0x80);
			int color = attr & 0x0f;

			if (m_flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;
				sx = 240 - sx;
				sy = 240 - sy;
			}

			if (sx >= 256-8)
				sx -= 256;

			gfx->transpen(bitmap,clip,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
		}
	}
}

uint32_t kncljoe_state::screen_update_kncljoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
