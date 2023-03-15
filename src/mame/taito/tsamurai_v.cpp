// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*
**  Video Driver for Taito Samurai (1985)
*/

#include "emu.h"
#include "tsamurai.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(tsamurai_state::get_bg_tile_info)
{
	uint8_t attributes = m_bg_videoram[2*tile_index+1];
	int tile_number = m_bg_videoram[2*tile_index];
	tile_number += (( attributes & 0xc0 ) >> 6 ) * 256;  /* legacy */
	tileinfo.set(0,
			tile_number,
			attributes & 0x1f,
			0);
}

TILE_GET_INFO_MEMBER(tsamurai_state::get_fg_tile_info)
{
	int tile_number = m_videoram[tile_index];
	if (m_textbank1 & 0x01) tile_number += 256; /* legacy */
	tileinfo.set(1,
			tile_number,
			m_colorram[((tile_index&0x1f)*2)+1] & 0x1f,
			0);
}

TILE_GET_INFO_MEMBER(m660_state::get_bg_tile_info)
{
	uint8_t attributes = m_bg_videoram[2*tile_index+1];
	int tile_number = m_bg_videoram[2*tile_index];
	tile_number += (( attributes & 0xc0 ) >> 6 ) * 256;  /* legacy */
	tile_number += (( attributes & 0x20 ) >> 5 ) * 1024; /* Mission 660 add-on*/
	tileinfo.set(0,
			tile_number,
			attributes & 0x1f,
			0);
}

TILE_GET_INFO_MEMBER(m660_state::get_fg_tile_info)
{
	int tile_number = m_videoram[tile_index];
	if (m_textbank1 & 0x01) tile_number += 256; /* legacy */
	if (m_textbank2 & 0x01) tile_number += 512; /* Mission 660 add-on */
	tileinfo.set(1,
			tile_number,
			m_colorram[((tile_index&0x1f)*2)+1] & 0x1f,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/



void tsamurai_state::video_start()
{
	m_background = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tsamurai_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);
	m_foreground = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tsamurai_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	m_background->set_transparent_pen(0);
	m_foreground->set_transparent_pen(0);

	m_bgcolor = 0;
	m_numsprites = 32;

	save_item(NAME(m_bgcolor));
	save_item(NAME(m_textbank1));
}

void m660_state::video_start()
{
	tsamurai_state::video_start();

	m_textbank2 = 0;
	save_item(NAME(m_textbank2));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void tsamurai_state::scrolly_w(uint8_t data)
{
	m_background->set_scrolly(0, data );
}

void tsamurai_state::scrollx_w(uint8_t data)
{
	m_background->set_scrollx(0, data );
}

void tsamurai_state::bgcolor_w(uint8_t data)
{
	m_bgcolor = data;
}

WRITE_LINE_MEMBER(tsamurai_state::textbank1_w)
{
	m_textbank1 = state;
	m_foreground->mark_all_dirty();
}

WRITE_LINE_MEMBER(m660_state::textbank2_w)
{
	m_textbank2 = state;
	m_foreground->mark_all_dirty();
}

void tsamurai_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset]=data;
	m_background->mark_tile_dirty(offset/2);
}

void tsamurai_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset]=data;
	m_foreground->mark_tile_dirty(offset);
}

void tsamurai_state::fg_colorram_w(offs_t offset, uint8_t data)
{
	if( m_colorram[offset]!=data )
	{
		m_colorram[offset]=data;
		if (offset & 1)
		{
			int col = offset/2;
			int row;
			for (row = 0;row < 32;row++)
				m_foreground->mark_tile_dirty(32*row+col);
		}
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

void tsamurai_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(2);
	const uint8_t *source = m_spriteram+m_numsprites*4-4;
	const uint8_t *finish = m_spriteram; /* ? */

	while( source>=finish )
	{
		int attributes = source[2]; /* bit 0x10 is usually, but not always set */

		int sx = source[3] - 16;
		int sy = 240-source[0];
		int sprite_number = source[1];
		int color = attributes&0x1f;

#if 0
		/* VS Gong Fight */
		if (attributes == 0xe)
			attributes = 4;
		if (attributes > 7 || attributes < 4 || attributes == 5 )
			attributes = 6;
		color = attributes&0x1f;
#endif

#if 0
		/* Nunchakun */
		color = 0x2d - (attributes&0x1f);
#endif

		if( sy<-16 ) sy += 256;

		/* 240-source[0] seems nice,but some dangling sprites appear on the left      */
		/* side in Mission 660.Setting it to 242 fixes it,but will break other games. */
		/* So I'm using this specific check. -kal 11 jul 2002 */
//      if(sprite_type == 1) sy=sy+2;

		if( flip_screen() )
		{
			gfx->transpen(bitmap,cliprect,
				sprite_number&0x7f,
				color,
				1,(sprite_number&0x80)?0:1,
				256-32-sx,256-32-sy,0 );
		}
		else
		{
			gfx->transpen(bitmap,cliprect,
				sprite_number&0x7f,
				color,
				0,sprite_number&0x80,
				sx,sy,0 );
		}

		source -= 4;
	}
}

uint32_t tsamurai_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

/* Do the column scroll used for the "660" logo on the title screen */
	m_foreground->set_scroll_cols(32);
	for (i = 0 ; i < 32 ; i++)
	{
		m_foreground->set_scrolly(i, m_colorram[i*2]);
	}
/* end of column scroll code */

	/*
	    This following isn't particularly efficient.  We'd be better off to
	    dynamically change every 8th palette to the background color, so we
	    could draw the background as an opaque tilemap.

	    Note that the background color register isn't well understood
	    (screenshots would be helpful)
	*/
	bitmap.fill(m_bgcolor, cliprect);
	m_background->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	m_foreground->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

/***************************************************************************

VS Gong Fight runs on older hardware

***************************************************************************/


void vsgongf_state::vsgongf_color_w(uint8_t data)
{
	if( m_vsgongf_color != data )
	{
		m_vsgongf_color = data;
		m_foreground ->mark_all_dirty();
	}
}


TILE_GET_INFO_MEMBER(vsgongf_state::get_vsgongf_tile_info)
{
	int tile_number = m_videoram[tile_index];
	int color = m_vsgongf_color&0x1f;
	if( m_textbank1 ) tile_number += 0x100;
	tileinfo.set(1,
			tile_number,
			color,
			0);
}

void vsgongf_state::video_start()
{
	m_foreground = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(vsgongf_state::get_vsgongf_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	m_numsprites = 16;

	save_item(NAME(m_vsgongf_color));
	save_item(NAME(m_textbank1));
}

uint32_t vsgongf_state::screen_update_vsgongf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	#ifdef MAME_DEBUG
	if( machine().input().code_pressed( KEYCODE_Q ) ){
		while( machine().input().code_pressed( KEYCODE_Q ) ){
			m_key_count++;
			m_vsgongf_color = m_key_count;
			m_foreground ->mark_all_dirty();
		}
	}
	#endif

	m_foreground->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	return 0;
}
