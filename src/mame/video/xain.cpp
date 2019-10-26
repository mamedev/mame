// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Rob Rosenbrock, Phil Stroffolino
/***************************************************************************

    xain.c

    The priority prom has 7 inputs:

    A0: Text layer (MAP)
    A1: Sprite layer (OBJ)
    A2: BG1
    A3: BG2
    A4-A6:  From CPU priority register

    The 2 bit data output from the prom selects:

    0 - Text layer
    1 - Sprite layer
    2 - BG1
    3 - BG2

    Decoding the prom manually gives the following rules:

    PRI mode 0 - text (top) -> sprite -> bg1 -> bg2 (bottom)
    PRI mode 1 - text (top) -> sprite -> bg2 -> bg1 (bottom)
    PRI mode 2 - bg1 (top) -> sprite -> bg2 -> text (bottom)
    PRI mode 3 - bg2 (top) -> sprite -> bg1 -> text (bottom)
    PRI mode 4 - bg1 (top) -> sprite -> text -> bg2 (bottom)
    PRI mode 5 - bg2 (top) -> sprite -> text -> bg1 (bottom)
    PRI mode 6 - text (top) -> bg1 -> sprite -> bg2 (bottom)
    PRI mode 7 - text (top) -> bg2 -> sprite -> bg1 (bottom)

***************************************************************************/

#include "emu.h"
#include "includes/xain.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(xain_state::back_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

template <unsigned N> TILE_GET_INFO_MEMBER(xain_state::get_bg_tile_info)
{
	int const attr = m_bgram[N][tile_index | 0x400];
	SET_TILE_INFO_MEMBER(2 - N,
			m_bgram[N][tile_index] | ((attr & 7) << 8),
			(attr & 0x70) >> 4,
			(attr & 0x80) ? TILE_FLIPX : 0);
}

TILE_GET_INFO_MEMBER(xain_state::get_char_tile_info)
{
	int attr = m_charram[tile_index | 0x400];
	SET_TILE_INFO_MEMBER(0,
			m_charram[tile_index] | ((attr & 3) << 8),
			(attr & 0xe0) >> 5,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void xain_state::video_start()
{
	m_bg_tilemaps[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xain_state::get_bg_tile_info<0>)), tilemap_mapper_delegate(*this, FUNC(xain_state::back_scan)), 16, 16, 32, 32);
	m_bg_tilemaps[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xain_state::get_bg_tile_info<1>)), tilemap_mapper_delegate(*this, FUNC(xain_state::back_scan)), 16, 16, 32, 32);
	m_char_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(xain_state::get_char_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemaps[0]->set_transparent_pen(0);
	m_bg_tilemaps[1]->set_transparent_pen(0);
	m_char_tilemap->set_transparent_pen(0);

	save_item(NAME(m_pri));
	save_item(NAME(m_scrollx[0]));
	save_item(NAME(m_scrolly[0]));
	save_item(NAME(m_scrollx[1]));
	save_item(NAME(m_scrolly[1]));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(xain_state::charram_w)
{
	m_charram[offset] = data;
	m_char_tilemap->mark_tile_dirty(offset & 0x3ff);
}


WRITE8_MEMBER(xain_state::flipscreen_w)
{
	flip_screen_set(data & 1);
}


/***************************************************************************

  Display refresh

***************************************************************************/

void xain_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes();offs += 4)
	{
		int sx,sy,flipx,flipy;
		int attr = m_spriteram[offs+1];
		int numtile = m_spriteram[offs+2] | ((attr & 7) << 8);
		int color = (attr & 0x38) >> 3;

		sx = 238 - m_spriteram[offs+3];
		if (sx <= -7) sx += 256;
		sy = 240 - m_spriteram[offs];
		if (sy <= -7) sy += 256;
		flipx = attr & 0x40;
		flipy = 0;
		if (flip_screen())
		{
			sx = 238 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (attr & 0x80)    /* double height */
		{
			m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
					numtile,
					color,
					flipx,flipy,
					sx,flipy ? sy+16:sy-16,0);
			m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
					numtile+1,
					color,
					flipx,flipy,
					sx,sy,0);
		}
		else
		{
			m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
					numtile,
					color,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

uint32_t xain_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch (m_pri&0x7)
	{
	case 0:
		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
		m_char_tilemap->draw(screen, bitmap, cliprect, 0,0);
		break;
	case 1:
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
		m_char_tilemap->draw(screen, bitmap, cliprect, 0,0);
		break;
	case 2:
		m_char_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, 0,0);
		break;
	case 3:
		m_char_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, 0,0);
		break;
	case 4:
		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_char_tilemap->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, 0,0);
		break;
	case 5:
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_char_tilemap->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, 0,0);
		break;
	case 6:
		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap,cliprect);
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, 0,0);
		m_char_tilemap->draw(screen, bitmap, cliprect, 0,0);
		break;
	case 7:
		m_bg_tilemaps[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_sprites(bitmap,cliprect);
		m_bg_tilemaps[0]->draw(screen, bitmap, cliprect, 0,0);
		m_char_tilemap->draw(screen, bitmap, cliprect, 0,0);
		break;
	}
	return 0;
}
