// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/***************************************************************************

  vastar.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/vastar.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(vastar_state::get_fg_tile_info)
{
	int code = m_fgvideoram[tile_index + 0x800] | (m_fgvideoram[tile_index + 0x400] << 8);
	int color = m_fgvideoram[tile_index];
	int fxy = (code & 0xc00) >> 10; // maybe, based on the other layers
	SET_TILE_INFO_MEMBER(0,
			code,
			color & 0x3f,
			TILE_FLIPXY(fxy));
}

TILE_GET_INFO_MEMBER(vastar_state::get_bg1_tile_info)
{
	int code = m_bg1videoram[tile_index + 0x800] | (m_bg1videoram[tile_index] << 8);
	int color = m_bg1videoram[tile_index + 0xc00];
	int fxy = (code & 0xc00) >> 10;
	SET_TILE_INFO_MEMBER(4,
			code,
			color & 0x3f,
			TILE_FLIPXY(fxy));
}

TILE_GET_INFO_MEMBER(vastar_state::get_bg2_tile_info)
{
	int code = m_bg2videoram[tile_index + 0x800] | (m_bg2videoram[tile_index] << 8);
	int color = m_bg2videoram[tile_index + 0xc00];
	int fxy = (code & 0xc00) >> 10;
	SET_TILE_INFO_MEMBER(3,
			code,
			color & 0x3f,
			TILE_FLIPXY(fxy));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void vastar_state::video_start()
{
	m_fg_tilemap  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(vastar_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS,8,8,32,32);
	m_bg1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(vastar_state::get_bg1_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);
	m_bg2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(vastar_state::get_bg2_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg1_tilemap->set_transparent_pen(0);
	m_bg2_tilemap->set_transparent_pen(0);

	m_bg1_tilemap->set_scroll_cols(32);
	m_bg2_tilemap->set_scroll_cols(32);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(vastar_state::fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(vastar_state::bg1videoram_w)
{
	m_bg1videoram[offset] = data;
	m_bg1_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(vastar_state::bg2videoram_w)
{
	m_bg2videoram[offset] = data;
	m_bg2_tilemap->mark_tile_dirty(offset & 0x3ff);
}




/***************************************************************************

  Display refresh

***************************************************************************/

// I wouldn't rule out the possibility of there being 2 sprite chips due to
// "offs & 0x20" being used to select the tile bank.
//
// for Planet Probe more cases appear correct if we draw the list in reverse
// order, but it's also possible we should be drawing 2 swapped lists in
// forward order instead
void vastar_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
//  for (int offs = 0; offs < 0x40; offs += 2)
	for (int offs = 0x40-2; offs >=0; offs -= 2)
	{
		int code = ((m_spriteram3[offs] & 0xfc) >> 2) + ((m_spriteram2[offs] & 0x01) << 6)
				+ ((offs & 0x20) << 2);

		int sx = m_spriteram3[offs + 1];
		int sy = m_spriteram1[offs];
		int color = m_spriteram1[offs + 1] & 0x3f;
		int flipx = m_spriteram3[offs] & 0x02;
		int flipy = m_spriteram3[offs] & 0x01;

		if (flip_screen())
		{
			flipx = !flipx;
			flipy = !flipy;
		}

		if (m_spriteram2[offs] & 0x08)   /* double width */
		{
			if (!flip_screen())
				sy = 224 - sy;

			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code/2,
					color,
					flipx,flipy,
					sx,sy,0);
			/* redraw with wraparound */
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code/2,
					color,
					flipx,flipy,
					sx,sy+256,0);
		}
		else
		{
			if (!flip_screen())
				sy = 240 - sy;

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

UINT32 vastar_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0;i < 32;i++)
	{
		m_bg1_tilemap->set_scrolly(i,m_bg1_scroll[i]);
		m_bg2_tilemap->set_scrolly(i,m_bg2_scroll[i]);
	}

	switch (*m_sprite_priority)
	{
	case 0:
		m_bg1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		draw_sprites(bitmap,cliprect);
		m_bg2_tilemap->draw(screen, bitmap, cliprect, 0,0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
		break;

	case 1: // ?? planet probe
		m_bg1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		m_bg2_tilemap->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
		break;

	case 2:
		m_bg1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		draw_sprites(bitmap,cliprect);
		m_bg1_tilemap->draw(screen, bitmap, cliprect, 0,0);
		m_bg2_tilemap->draw(screen, bitmap, cliprect, 0,0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
		break;

	case 3:
		m_bg1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		m_bg2_tilemap->draw(screen, bitmap, cliprect, 0,0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
		draw_sprites(bitmap,cliprect);
		break;

	default:
		logerror("Unimplemented priority %X\n", *m_sprite_priority);
		break;
	}
	return 0;
}
