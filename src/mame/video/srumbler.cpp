// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

  srumbler.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/srumbler.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(srumbler_state::get_fg_tile_info)
{
	UINT8 attr = m_foregroundram[2*tile_index];
	SET_TILE_INFO_MEMBER(0,
			m_foregroundram[2*tile_index + 1] + ((attr & 0x03) << 8),
			(attr & 0x3c) >> 2,
			(attr & 0x40) ? TILE_FORCE_LAYER0 : 0);
}

TILE_GET_INFO_MEMBER(srumbler_state::get_bg_tile_info)
{
	UINT8 attr = m_backgroundram[2*tile_index];
	SET_TILE_INFO_MEMBER(1,
			m_backgroundram[2*tile_index + 1] + ((attr & 0x07) << 8),
			(attr & 0xe0) >> 5,
			((attr & 0x08) ? TILE_FLIPY : 0));
	tileinfo.group = (attr & 0x10) >> 4;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void srumbler_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(srumbler_state::get_fg_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(srumbler_state::get_bg_tile_info),this),TILEMAP_SCAN_COLS,    16,16,64,64);

	m_fg_tilemap->set_transparent_pen(3);

	m_bg_tilemap->set_transmask(0,0xffff,0x0000); /* split type 0 is totally transparent in front half */
	m_bg_tilemap->set_transmask(1,0x07ff,0xf800); /* split type 1 has pens 0-10 transparent in front half */

	save_item(NAME(m_scroll));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(srumbler_state::foreground_w)
{
	m_foregroundram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset/2);
}

WRITE8_MEMBER(srumbler_state::background_w)
{
	m_backgroundram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset/2);
}


WRITE8_MEMBER(srumbler_state::_4009_w)
{
	/* bit 0 flips screen */
	flip_screen_set(data & 1);

	/* bits 4-5 used during attract mode, unknown */

	/* bits 6-7 coin counters */
	machine().bookkeeping().coin_counter_w(0,data & 0x40);
	machine().bookkeeping().coin_counter_w(1,data & 0x80);
}


WRITE8_MEMBER(srumbler_state::scroll_w)
{
	m_scroll[offset] = data;

	m_bg_tilemap->set_scrollx(0,m_scroll[0] | (m_scroll[1] << 8));
	m_bg_tilemap->set_scrolly(0,m_scroll[2] | (m_scroll[3] << 8));
}



/***************************************************************************

  Display refresh

***************************************************************************/

void srumbler_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *buffered_spriteram = m_spriteram->buffer();
	int offs;

	/* Draw the sprites. */
	for (offs = m_spriteram->bytes()-4; offs>=0;offs -= 4)
	{
		/* SPRITES
		=====
		Attribute
		0x80 Code MSB
		0x40 Code MSB
		0x20 Code MSB
		0x10 Colour
		0x08 Colour
		0x04 Colour
		0x02 y Flip
		0x01 X MSB
		*/


		int code,colour,sx,sy,flipy;
		int attr = buffered_spriteram[offs+1];
		code = buffered_spriteram[offs];
		code += ( (attr&0xe0) << 3 );
		colour = (attr & 0x1c)>>2;
		sy = buffered_spriteram[offs + 2];
		sx = buffered_spriteram[offs + 3] + 0x100 * ( attr & 0x01);
		flipy = attr & 0x02;

		if (flip_screen())
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code,
				colour,
				flip_screen(),flipy,
				sx, sy,15);
	}
}


UINT32 srumbler_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	draw_sprites(bitmap,cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
