// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
/***************************************************************************

  cabal.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/cabal.h"

TILE_GET_INFO_MEMBER(cabal_state::get_back_tile_info)
{
	int tile = m_videoram[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO_MEMBER(1,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(cabal_state::get_text_tile_info)
{
	int tile = m_colorram[tile_index];
	int color = (tile>>10);

	tile &= 0x3ff;

	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}


void cabal_state::video_start()
{
	m_background_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cabal_state::get_back_tile_info),this),TILEMAP_SCAN_ROWS,16,16,16,16);
	m_text_layer       = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cabal_state::get_text_tile_info),this),TILEMAP_SCAN_ROWS,  8,8,32,32);

	m_text_layer->set_transparent_pen(3);
	m_background_layer->set_transparent_pen(15);
}


/**************************************************************************/

WRITE16_MEMBER(cabal_state::flipscreen_w)
{
	if (ACCESSING_BITS_0_7)
	{
		int flip = (data & 0x20) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
		m_background_layer->set_flip(flip);
		m_text_layer->set_flip(flip);

		flip_screen_set(data & 0x20);
	}
}

WRITE16_MEMBER(cabal_state::background_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_background_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(cabal_state::text_videoram_w)
{
	COMBINE_DATA(&m_colorram[offset]);
	m_text_layer->mark_tile_dirty(offset);
}


/********************************************************************

    Cabal Spriteram
    ---------------

    +0   .......x ........  Sprite enable bit
    +0   ........ xxxxxxxx  Sprite Y coordinate
    +1   ..??.... ........  ??? unknown ???
    +1   ....xxxx xxxxxxxx  Sprite tile number
    +2   .xxxx... ........  Sprite color bank
    +2   .....x.. ........  Sprite flip x
    +2   .......x xxxxxxxx  Sprite X coordinate
    +3   (unused)

            -------E YYYYYYYY
            ----BBTT TTTTTTTT
            -CCCCF-X XXXXXXXX
            -------- --------

********************************************************************/

void cabal_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs,data0,data1,data2;
	for( offs = m_spriteram.bytes()/2 - 4; offs >= 0; offs -= 4 )
	{
		data0 = m_spriteram[offs];
		data1 = m_spriteram[offs+1];
		data2 = m_spriteram[offs+2];

		if( data0 & 0x100 )
		{
			int tile_number = data1 & 0xfff;
			int color   = ( data2 & 0x7800 ) >> 11;
			int sy = ( data0 & 0xff );
			int sx = ( data2 & 0x1ff );
			int flipx = ( data2 & 0x0400 );
			int flipy = 0;

			if ( sx>256 )   sx -= 512;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				tile_number,
				color,
				flipx,flipy,
				sx,sy,0xf );
		}
	}
}


UINT32 cabal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_background_layer->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(bitmap,cliprect);
	m_text_layer->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
