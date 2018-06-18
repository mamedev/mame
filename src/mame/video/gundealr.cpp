// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/gundealr.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(gundealr_state::get_bg_tile_info)
{
	uint8_t attr = m_bg_videoram[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(0,
			m_bg_videoram[2 * tile_index] + ((attr & 0x07) << 8),
			(attr & 0xf0) >> 4,
			0);
}

TILEMAP_MAPPER_MEMBER(gundealr_state::pagescan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x3f) << 4) + ((row & 0x10) << 6);
}

TILE_GET_INFO_MEMBER(gundealr_state::get_fg_tile_info)
{
	uint8_t attr = m_fg_videoram[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(1,
			m_fg_videoram[2 * tile_index] + ((attr & 0x03) << 8),
			(attr & 0xf0) >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void gundealr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(gundealr_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(gundealr_state::get_fg_tile_info),this), tilemap_mapper_delegate(FUNC(gundealr_state::pagescan),this), 16, 16, 64, 32);

	m_fg_tilemap->set_transparent_pen(15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(gundealr_state::bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(gundealr_state::fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(gundealr_state::paletteram_w)
{
	int r,g,b,val;

	m_paletteram[offset] = data;

	val = m_paletteram[offset & ~1];
	r = (val >> 4) & 0x0f;
	g = (val >> 0) & 0x0f;

	val = m_paletteram[offset | 1];
	b = (val >> 4) & 0x0f;
	/* TODO: the bottom 4 bits are used as well, but I'm not sure about the meaning */

	m_palette->set_pen_color(offset / 2, pal4bit(r), pal4bit(g), pal4bit(b));
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t gundealr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
