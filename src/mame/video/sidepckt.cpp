// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/******************************************************************************

    Data East Side Pocket hardware

    Functions to emulate the video hardware

******************************************************************************/

#include "emu.h"
#include "includes/sidepckt.h"


PALETTE_INIT_MEMBER(sidepckt_state, sidepckt)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0;i < palette.entries();i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i + palette.entries()] >> 0) & 0x01;
		bit1 = (color_prom[i + palette.entries()] >> 1) & 0x01;
		bit2 = (color_prom[i + palette.entries()] >> 2) & 0x01;
		bit3 = (color_prom[i + palette.entries()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(sidepckt_state::get_tile_info)
{
	UINT8 attr = m_colorram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			m_videoram[tile_index] + ((attr & 0x07) << 8),
			((attr & 0x10) >> 3) | ((attr & 0x20) >> 5),
			TILE_FLIPX);
	tileinfo.group = (attr & 0x80) >> 7;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void sidepckt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sidepckt_state::get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);

	m_bg_tilemap->set_transmask(0,0xff,0x00); /* split type 0 is totally transparent in front half */
	m_bg_tilemap->set_transmask(1,0x01,0xfe); /* split type 1 has pen 0 transparent in front half */

	machine().tilemap().set_flip_all(TILEMAP_FLIPX);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(sidepckt_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sidepckt_state::colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sidepckt_state::flipscreen_w)
{
	int flipscreen = data;
	machine().tilemap().set_flip_all(flipscreen ? TILEMAP_FLIPY : TILEMAP_FLIPX);
}


/***************************************************************************

  Display refresh

***************************************************************************/

void sidepckt_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = 0;offs < m_spriteram.bytes(); offs += 4)
	{
		int sx,sy,code,color,flipx,flipy;

		code = m_spriteram[offs+3] + ((m_spriteram[offs+1] & 0x03) << 8);
		color = (m_spriteram[offs+1] & 0xf0) >> 4;

		sx = m_spriteram[offs+2]-2;
		sy = m_spriteram[offs];

		flipx = m_spriteram[offs+1] & 0x08;
		flipy = m_spriteram[offs+1] & 0x04;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,0);
		/* wraparound */
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx-256,sy,0);
	}
}


UINT32 sidepckt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	draw_sprites(bitmap,cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	return 0;
}
