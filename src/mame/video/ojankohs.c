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

PALETTE_INIT_MEMBER(ojankohs_state,ojankoy)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	int bit0, bit1, bit2, bit3, bit4, r, g, b;

	for (i = 0; i < palette.entries(); i++)
	{
		bit0 = BIT(color_prom[0], 2);
		bit1 = BIT(color_prom[0], 3);
		bit2 = BIT(color_prom[0], 4);
		bit3 = BIT(color_prom[0], 5);
		bit4 = BIT(color_prom[0], 6);
		r = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;
		bit0 = BIT(color_prom[palette.entries()], 5);
		bit1 = BIT(color_prom[palette.entries()], 6);
		bit2 = BIT(color_prom[palette.entries()], 7);
		bit3 = BIT(color_prom[0], 0);
		bit4 = BIT(color_prom[0], 1);
		g = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;
		bit0 = BIT(color_prom[palette.entries()], 0);
		bit1 = BIT(color_prom[palette.entries()], 1);
		bit2 = BIT(color_prom[palette.entries()], 2);
		bit3 = BIT(color_prom[palette.entries()], 3);
		bit4 = BIT(color_prom[palette.entries()], 4);
		b = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

WRITE8_MEMBER(ojankohs_state::ojankohs_palette_w)
{
	int r, g, b;

	m_paletteram[offset] = data;

	offset &= 0x7fe;

	r = (m_paletteram[offset + 0] & 0x7c) >> 2;
	g = ((m_paletteram[offset + 0] & 0x03) << 3) | ((m_paletteram[offset + 1] & 0xe0) >> 5);
	b = (m_paletteram[offset + 1] & 0x1f) >> 0;

	m_palette->set_pen_color(offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_MEMBER(ojankohs_state::ccasino_palette_w)
{
	int r, g, b;

	/* get top 8 bits of the I/O port address */
	offset = (offset << 8) | (space.device().state().state_int(Z80_BC) >> 8);

	m_paletteram[offset] = data;

	offset &= 0x7fe;

	r = (m_paletteram[offset + 0] & 0x7c) >> 2;
	g = ((m_paletteram[offset + 0] & 0x03) << 3) | ((m_paletteram[offset + 1] & 0xe0) >> 5);
	b = (m_paletteram[offset + 1] & 0x1f) >> 0;

	m_palette->set_pen_color(offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_MEMBER(ojankohs_state::ojankoc_palette_w)
{
	int r, g, b, color;

	if (m_paletteram[offset] == data)
		return;

	m_paletteram[offset] = data;
	m_screen_refresh = 1;

	color = (m_paletteram[offset & 0x1e] << 8) | m_paletteram[offset | 0x01];

	r = (color >> 10) & 0x1f;
	g = (color >>  5) & 0x1f;
	b = (color >>  0) & 0x1f;

	m_palette->set_pen_color(offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}


/******************************************************************************

    Tilemap system

******************************************************************************/

WRITE8_MEMBER(ojankohs_state::ojankohs_videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ojankohs_state::ojankohs_colorram_w)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ojankohs_state::ojankohs_gfxreg_w)
{
	if (m_gfxreg != data)
	{
		m_gfxreg = data;
		m_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(ojankohs_state::ojankohs_flipscreen_w)
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

TILE_GET_INFO_MEMBER(ojankohs_state::ojankohs_get_tile_info)
{
	int tile = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x0f) << 8);
	int color = (m_colorram[tile_index] & 0xe0) >> 5;

	if (m_colorram[tile_index] & 0x10)
	{
		tile |= (m_gfxreg & 0x07) << 12;
		color |= (m_gfxreg & 0xe0) >> 2;
	}

	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(ojankohs_state::ojankoy_get_tile_info)
{
	int tile = m_videoram[tile_index] | (m_videoram[tile_index + 0x1000] << 8);
	int color = m_colorram[tile_index] & 0x3f;
	int flipx = ((m_colorram[tile_index] & 0x40) >> 6) ? TILEMAP_FLIPX : 0;
	int flipy = ((m_colorram[tile_index] & 0x80) >> 7) ? TILEMAP_FLIPY : 0;

	SET_TILE_INFO_MEMBER(0, tile, color, (flipx | flipy));
}


/******************************************************************************

    Pixel system

******************************************************************************/

void ojankohs_state::ojankoc_flipscreen( address_space &space, int data )
{
	int x, y;
	UINT8 color1, color2;

	m_flipscreen = BIT(data, 7);

	if (m_flipscreen == m_flipscreen_old)
		return;

	for (y = 0; y < 0x40; y++)
	{
		for (x = 0; x < 0x100; x++)
		{
			color1 = m_videoram[0x0000 + ((y * 256) + x)];
			color2 = m_videoram[0x3fff - ((y * 256) + x)];
			ojankoc_videoram_w(space, 0x0000 + ((y * 256) + x), color2);
			ojankoc_videoram_w(space, 0x3fff - ((y * 256) + x), color1);

			color1 = m_videoram[0x4000 + ((y * 256) + x)];
			color2 = m_videoram[0x7fff - ((y * 256) + x)];
			ojankoc_videoram_w(space, 0x4000 + ((y * 256) + x), color2);
			ojankoc_videoram_w(space, 0x7fff - ((y * 256) + x), color1);
		}
	}

	m_flipscreen_old = m_flipscreen;
}

WRITE8_MEMBER(ojankohs_state::ojankoc_videoram_w)
{
	int i;
	UINT8 x, y, xx, px, py ;
	UINT8 color, color1, color2;

	m_videoram[offset] = data;

	color1 = m_videoram[offset & 0x3fff];
	color2 = m_videoram[offset | 0x4000];

	y = offset >> 6;
	x = (offset & 0x3f) << 2;
	xx = 0;

	if (m_flipscreen)
	{
		x = 0xfc - x;
		y = 0xff - y;
		xx = 3;
	}

	for (i = 0; i < 4; i++)
	{
		color = ((color1 & 0x01) >> 0) | ((color1 & 0x10) >> 3) | ((color2 & 0x01) << 2) | ((color2 & 0x10) >> 1);

		px = x + (i ^ xx);
		py = y;

		m_tmpbitmap.pix16(py, px) = color;

		color1 >>= 1;
		color2 >>= 1;
	}
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

VIDEO_START_MEMBER(ojankohs_state,ojankohs)
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ojankohs_state::ojankohs_get_tile_info),this), TILEMAP_SCAN_ROWS,  8, 4, 64, 64);
//  m_videoram = auto_alloc_array(machine(), UINT8, 0x1000);
//  m_colorram = auto_alloc_array(machine(), UINT8, 0x1000);
//  m_paletteram = auto_alloc_array(machine(), UINT8, 0x800);
}

VIDEO_START_MEMBER(ojankohs_state,ojankoy)
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ojankohs_state::ojankoy_get_tile_info),this), TILEMAP_SCAN_ROWS,  8, 4, 64, 64);
//  m_videoram = auto_alloc_array(machine(), UINT8, 0x2000);
//  m_colorram = auto_alloc_array(machine(), UINT8, 0x1000);
}

VIDEO_START_MEMBER(ojankohs_state,ojankoc)
{
	m_screen->register_screen_bitmap(m_tmpbitmap);
	m_videoram.allocate(0x8000);
	m_paletteram.allocate(0x20);

	save_item(NAME(m_tmpbitmap));
}


/******************************************************************************

    Display refresh

******************************************************************************/

UINT32 ojankohs_state::screen_update_ojankohs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->set_scrollx(0, m_scrollx);
	m_tilemap->set_scrolly(0, m_scrolly);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 ojankohs_state::screen_update_ojankoc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	if (m_screen_refresh)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		/* redraw bitmap */
		for (offs = 0; offs < 0x8000; offs++)
		{
			ojankoc_videoram_w(space, offs, m_videoram[offs]);
		}
		m_screen_refresh = 0;
	}

	copybitmap(bitmap, m_tmpbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
