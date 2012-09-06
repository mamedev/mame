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
	UINT8 attr = m_bg_videoram[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(
			0,
			m_bg_videoram[2 * tile_index] + ((attr & 0x07) << 8),
			(attr & 0xf0) >> 4,
			0);
}

TILEMAP_MAPPER_MEMBER(gundealr_state::gundealr_scan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x3f) << 4) + ((row & 0x10) << 6);
}

TILE_GET_INFO_MEMBER(gundealr_state::get_fg_tile_info)
{
	UINT8 attr = m_fg_videoram[2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(
			1,
			m_fg_videoram[2 * tile_index] + ((attr & 0x03) << 8),
			(attr & 0xf0) >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gundealr )
{
	gundealr_state *state = machine.driver_data<gundealr_state>();
	state->m_bg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(gundealr_state::get_bg_tile_info),state), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	state->m_fg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(gundealr_state::get_fg_tile_info),state), tilemap_mapper_delegate(FUNC(gundealr_state::gundealr_scan),state), 16, 16, 64, 32);

	state->m_fg_tilemap->set_transparent_pen(15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(gundealr_state::gundealr_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(gundealr_state::gundealr_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(gundealr_state::gundealr_paletteram_w)
{
	int r,g,b,val;

	m_paletteram[offset] = data;

	val = m_paletteram[offset & ~1];
	r = (val >> 4) & 0x0f;
	g = (val >> 0) & 0x0f;

	val = m_paletteram[offset | 1];
	b = (val >> 4) & 0x0f;
	/* TODO: the bottom 4 bits are used as well, but I'm not sure about the meaning */

	palette_set_color_rgb(machine(), offset / 2, pal4bit(r), pal4bit(g), pal4bit(b));
}

WRITE8_MEMBER(gundealr_state::gundealr_fg_scroll_w)
{
	m_scroll[offset] = data;
	m_fg_tilemap->set_scrollx(0, m_scroll[1] | ((m_scroll[0] & 0x03) << 8));
	m_fg_tilemap->set_scrolly(0, m_scroll[3] | ((m_scroll[2] & 0x03) << 8));
}

WRITE8_MEMBER(gundealr_state::yamyam_fg_scroll_w)
{
	m_scroll[offset] = data;
	m_fg_tilemap->set_scrollx(0, m_scroll[0] | ((m_scroll[1] & 0x03) << 8));
	m_fg_tilemap->set_scrolly(0, m_scroll[2] | ((m_scroll[3] & 0x03) << 8));
}

WRITE8_MEMBER(gundealr_state::gundealr_flipscreen_w)
{
	m_flipscreen = data;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}



/***************************************************************************

  Display refresh

***************************************************************************/

SCREEN_UPDATE_IND16( gundealr )
{
	gundealr_state *state = screen.machine().driver_data<gundealr_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
