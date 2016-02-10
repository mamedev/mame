// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Paul Leaman
// thanks-to: Steven Frew (the author of Slutte)
/***************************************************************************

    Bionic Commando Video Hardware

    This board handles tile/tile and tile/sprite priority with a PROM. Its
    working is complicated and hardcoded in the driver.

    The PROM is a 256x4 chip, with address inputs wired as follows:

    A0 bg opaque
    A1 \
    A2 |  fg pen
    A3 |
    A4 /
    A5 fg has priority over sprites (bit 5 of tile attribute)
    A6 fg has not priority over bg (bits 6 & 7 of tile attribute both set)
    A7 sprite opaque

    The output selects the active layer, it can be:
    0  bg
    1  fg
    2  sprite

***************************************************************************/

#include "emu.h"
#include "includes/bionicc.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(bionicc_state::get_bg_tile_info)
{
	int attr = m_bgvideoram[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(1,
			(m_bgvideoram[2 * tile_index] & 0xff) + ((attr & 0x07) << 8),
			(attr & 0x18) >> 3,
			TILE_FLIPXY((attr & 0xc0) >> 6));
}

TILE_GET_INFO_MEMBER(bionicc_state::get_fg_tile_info)
{
	int attr = m_fgvideoram[2 * tile_index + 1];
	int flags;

	if ((attr & 0xc0) == 0xc0)
	{
		tileinfo.category = 1;
		tileinfo.group = 0;
		flags = 0;
	}
	else
	{
		tileinfo.category = 0;
		tileinfo.group = (attr & 0x20) >> 5;
		flags = TILE_FLIPXY((attr & 0xc0) >> 6);
	}

	SET_TILE_INFO_MEMBER(2,
			(m_fgvideoram[2 * tile_index] & 0xff) + ((attr & 0x07) << 8),
			(attr & 0x18) >> 3,
			flags);
}

TILE_GET_INFO_MEMBER(bionicc_state::get_tx_tile_info)
{
	int attr = m_txvideoram[tile_index + 0x400];
	SET_TILE_INFO_MEMBER(0,
			(m_txvideoram[tile_index] & 0xff) + ((attr & 0x00c0) << 2),
			attr & 0x3f,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void bionicc_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bionicc_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bionicc_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bionicc_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 64);

	m_tx_tilemap->set_transparent_pen(3);
	m_fg_tilemap->set_transmask(0, 0xffff, 0x8000); /* split type 0 is completely transparent in front half */
	m_fg_tilemap->set_transmask(1, 0xffc1, 0x803e); /* split type 1 has pens 1-5 opaque in front half */
	m_bg_tilemap->set_transparent_pen(15);
}

PALETTE_DECODER_MEMBER( bionicc_state, RRRRGGGGBBBBIIII )
{
	UINT8 bright = (raw & 0x0f);

	UINT8 r = ((raw >> 12) & 0x0f) * 0x11;
	UINT8 g = ((raw >>  8) & 0x0f) * 0x11;
	UINT8 b = ((raw >>  4) & 0x0f) * 0x11;

	if ((bright & 0x08) == 0)
	{
		r = r * (0x07 + bright) / 0x0e;
		g = g * (0x07 + bright) / 0x0e;
		b = b * (0x07 + bright) / 0x0e;
	}

	return rgb_t(r, g, b);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(bionicc_state::bionicc_bgvideoram_w)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(bionicc_state::bionicc_fgvideoram_w)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(bionicc_state::bionicc_txvideoram_w)
{
	COMBINE_DATA(&m_txvideoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE16_MEMBER(bionicc_state::bionicc_scroll_w)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0:
			m_fg_tilemap->set_scrollx(0, data);
			break;
		case 1:
			m_fg_tilemap->set_scrolly(0, data);
			break;
		case 2:
			m_bg_tilemap->set_scrollx(0, data);
			break;
		case 3:
			m_bg_tilemap->set_scrolly(0, data);
			break;
	}
}

WRITE16_MEMBER(bionicc_state::bionicc_gfxctrl_w)
{
	if (ACCESSING_BITS_8_15)
	{
		flip_screen_set(data & 0x0100);

		m_bg_tilemap->enable(data & 0x2000);    /* guess */
		m_fg_tilemap->enable(data & 0x1000);    /* guess */

		machine().bookkeeping().coin_counter_w(0, data & 0x8000);
		machine().bookkeeping().coin_counter_w(1, data & 0x4000);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/



UINT32 bionicc_state::screen_update_bionicc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER1, 0);   /* nothing in FRONT */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER1, 0);
	m_spritegen->draw_sprites(bitmap, cliprect, m_gfxdecode, 3, m_spriteram->buffer(), m_spriteram->bytes(), flip_screen(), 0 );
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER0, 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
