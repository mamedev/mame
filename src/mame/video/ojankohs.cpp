// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, Uki
/******************************************************************************

    Video Hardware for Video System Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/06/10 -
    Driver by Uki 2001/12/10 -

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/ojankohs.h"

/******************************************************************************

    Palette system

******************************************************************************/

void ojankoy_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0 = BIT(color_prom[0], 2);
		int bit1 = BIT(color_prom[0], 3);
		int bit2 = BIT(color_prom[0], 4);
		int bit3 = BIT(color_prom[0], 5);
		int bit4 = BIT(color_prom[0], 6);
		int const r = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;

		bit0 = BIT(color_prom[palette.entries()], 5);
		bit1 = BIT(color_prom[palette.entries()], 6);
		bit2 = BIT(color_prom[palette.entries()], 7);
		bit3 = BIT(color_prom[0], 0);
		bit4 = BIT(color_prom[0], 1);
		int const g = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;

		bit0 = BIT(color_prom[palette.entries()], 0);
		bit1 = BIT(color_prom[palette.entries()], 1);
		bit2 = BIT(color_prom[palette.entries()], 2);
		bit3 = BIT(color_prom[palette.entries()], 3);
		bit4 = BIT(color_prom[palette.entries()], 4);
		int const b = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

void ojankohs_state::palette_w(offs_t offset, uint8_t data)
{
	m_paletteram[offset] = data;

	offset &= 0x7fe;

	int const r = (m_paletteram[offset + 0] & 0x7c) >> 2;
	int const g = ((m_paletteram[offset + 0] & 0x03) << 3) | ((m_paletteram[offset + 1] & 0xe0) >> 5);
	int const b = (m_paletteram[offset + 1] & 0x1f) >> 0;

	m_palette->set_pen_color(offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

void ccasino_state::palette_w(offs_t offset, uint8_t data)
{
	offset = bitswap<11>(offset, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8);

	m_paletteram[offset] = data;

	offset &= 0x7fe;

	int const r = (m_paletteram[offset + 0] & 0x7c) >> 2;
	int const g = ((m_paletteram[offset + 0] & 0x03) << 3) | ((m_paletteram[offset + 1] & 0xe0) >> 5);
	int const b = (m_paletteram[offset + 1] & 0x1f) >> 0;

	m_palette->set_pen_color(offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

void ojankoc_state::palette_w(offs_t offset, uint8_t data)
{
	if (m_paletteram[offset] != data)
	{
		m_paletteram[offset] = data;
		m_screen_refresh = 1;

		int const color = (m_paletteram[offset & 0x1e] << 8) | m_paletteram[offset | 0x01];

		int const r = (color >> 10) & 0x1f;
		int const g = (color >>  5) & 0x1f;
		int const b = (color >>  0) & 0x1f;

		m_palette->set_pen_color(offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}


/******************************************************************************

    Tilemap system

******************************************************************************/

void ojankohs_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void ojankohs_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void ojankohs_state::gfxreg_w(uint8_t data)
{
	if (m_gfxreg != data)
	{
		m_gfxreg = data;
		m_tilemap->mark_all_dirty();
	}
}

void ojankohs_state::flipscreen_w(uint8_t data)
{
	if (m_flipscreen != BIT(data, 0))
	{
		m_flipscreen = BIT(data, 0);

		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

		if (m_flipscreen)
		{
			m_scrollx = -0xe0;
			m_scrolly = -0x20;
		}
		else
		{
			m_scrollx = 0;
			m_scrolly = 0;
		}
	}
}

TILE_GET_INFO_MEMBER(ojankohs_state::get_tile_info)
{
	int tile = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x0f) << 8);
	int color = (m_colorram[tile_index] & 0xe0) >> 5;

	if (m_colorram[tile_index] & 0x10)
	{
		tile |= (m_gfxreg & 0x07) << 12;
		color |= (m_gfxreg & 0xe0) >> 2;
	}

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(ojankoy_state::get_tile_info)
{
	int tile = m_videoram[tile_index] | (m_videoram[tile_index + 0x1000] << 8);
	int color = m_colorram[tile_index] & 0x3f;
	int flipx = ((m_colorram[tile_index] & 0x40) >> 6) ? TILEMAP_FLIPX : 0;
	int flipy = ((m_colorram[tile_index] & 0x80) >> 7) ? TILEMAP_FLIPY : 0;

	tileinfo.set(0, tile, color, (flipx | flipy));
}


/******************************************************************************

    Pixel system

******************************************************************************/

void ojankoc_state::flipscreen(uint8_t data)
{
	m_flipscreen = BIT(data, 7);

	if (m_flipscreen == m_flipscreen_old)
		return;

	for (int y = 0; y < 0x40; y++)
	{
		for (int x = 0; x < 0x100; x++)
		{
			uint8_t color1 = m_videoram[0x0000 + ((y * 256) + x)];
			uint8_t color2 = m_videoram[0x3fff - ((y * 256) + x)];
			videoram_w(0x0000 + ((y * 256) + x), color2);
			videoram_w(0x3fff - ((y * 256) + x), color1);

			color1 = m_videoram[0x4000 + ((y * 256) + x)];
			color2 = m_videoram[0x7fff - ((y * 256) + x)];
			videoram_w(0x4000 + ((y * 256) + x), color2);
			videoram_w(0x7fff - ((y * 256) + x), color1);
		}
	}

	m_flipscreen_old = m_flipscreen;
}

void ojankoc_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;

	uint8_t color1 = m_videoram[offset & 0x3fff];
	uint8_t color2 = m_videoram[offset | 0x4000];

	uint8_t y = offset >> 6;
	uint8_t x = (offset & 0x3f) << 2;
	uint8_t xx = 0;

	if (m_flipscreen)
	{
		x = 0xfc - x;
		y = 0xff - y;
		xx = 3;
	}

	for (int i = 0; i < 4; i++)
	{
		uint8_t color = ((color1 & 0x01) >> 0) | ((color1 & 0x10) >> 3) | ((color2 & 0x01) << 2) | ((color2 & 0x10) >> 1);

		uint8_t px = x + (i ^ xx);
		uint8_t py = y;

		m_tmpbitmap.pix(py, px) = color;

		color1 >>= 1;
		color2 >>= 1;
	}
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

void ojankohs_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ojankohs_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);
}

void ojankoy_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ojankoy_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 4, 64, 64);
}

void ojankoc_state::video_start()
{
	m_screen->register_screen_bitmap(m_tmpbitmap);

	save_item(NAME(m_tmpbitmap));
}


/******************************************************************************

    Display refresh

******************************************************************************/

uint32_t ojankohs_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->set_scrollx(0, m_scrollx);
	m_tilemap->set_scrolly(0, m_scrolly);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t ojankoc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_screen_refresh)
	{
		// redraw bitmap
		for (int offs = 0; offs < 0x8000; offs++)
		{
			videoram_w(offs, m_videoram[offs]);
		}
		m_screen_refresh = 0;
	}

	copybitmap(bitmap, m_tmpbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
