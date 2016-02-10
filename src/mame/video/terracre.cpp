// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/terracre.h"


TILE_GET_INFO_MEMBER(terracre_state::get_bg_tile_info)
{
	/* xxxx.----.----.----
	 * ----.xx--.----.----
	 * ----.--xx.xxxx.xxxx */
	unsigned data = m_bg_videoram[tile_index];
	unsigned color = data>>11;
	SET_TILE_INFO_MEMBER(1,data&0x3ff,color,0 );
}

TILE_GET_INFO_MEMBER(terracre_state::get_fg_tile_info)
{
	unsigned data = m_fg_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0,data&0xff,0,0 );
}

void terracre_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const UINT8 *spritepalettebank = memregion("user1")->base();
	gfx_element *pGfx = m_gfxdecode->gfx(2);
	const UINT16 *pSource = m_spriteram->buffer();
	int flip = flip_screen();
	int transparent_pen;

	if( pGfx->elements() > 0x200 )
	{ /* HORE HORE Kid */
		transparent_pen = 0xf;
	}
	else
	{
		transparent_pen = 0x0;
	}
	for( int i=0; i<0x200; i+=8 )
	{
		int tile = pSource[1]&0xff;
		int attrs = pSource[2];
		int flipx = attrs&0x04;
		int flipy = attrs&0x08;
		int color = (attrs&0xf0)>>4;
		int sx = (pSource[3] & 0xff) - 0x80 + 256 * (attrs & 1);
		int sy = 240 - (pSource[0] & 0xff);

		if( transparent_pen )
		{
			int bank;

			if( attrs&0x02 ) tile |= 0x200;
			if( attrs&0x10 ) tile |= 0x100;

			bank = (tile&0xfc)>>1;
			if( tile&0x200 ) bank |= 0x80;
			if( tile&0x100 ) bank |= 0x01;

			color &= 0xe;
			color += 16*(spritepalettebank[bank]&0xf);
		}
		else
		{
			if( attrs&0x02 ) tile|= 0x100;
			color += 16 * (spritepalettebank[(tile>>1)&0xff] & 0x0f);
		}

		if (flip)
		{
				sx=240-sx;
				sy=240-sy;
				flipx = !flipx;
				flipy = !flipy;
		}

		pGfx->transpen(
			bitmap,cliprect,tile, color,flipx,flipy,sx,sy,transparent_pen );

		pSource += 4;
	}
}

PALETTE_INIT_MEMBER(terracre_state, terracre)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters use colors 0-0x0f */
	for (i = 0; i < 0x10; i++)
		palette.set_pen_indirect(i, i);

	/* background tiles use colors 0xc0-0xff in four banks */
	/* the bottom two bits of the color code select the palette bank for */
	/* pens 0-7; the top two bits for pens 8-0x0f. */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry;

		if (i & 0x08)
			ctabentry = 0xc0 | (i & 0x0f) | ((i & 0xc0) >> 2);
		else
			ctabentry = 0xc0 | (i & 0x0f) | ((i & 0x30) >> 0);

		palette.set_pen_indirect(0x10 + i, ctabentry);
	}

	/* sprites use colors 128-191 in four banks */
	/* The lookup table tells which colors to pick from the selected bank */
	/* the bank is selected by another PROM and depends on the top 8 bits of */
	/* the sprite code. The PROM selects the bank *separately* for pens 0-7 and */
	/* 8-15 (like for tiles). */
	for (i = 0; i < 0x1000; i++)
	{
		UINT8 ctabentry;
		int i_swapped = ((i & 0x0f) << 8) | ((i & 0xff0) >> 4);

		if (i & 0x80)
			ctabentry = 0x80 | ((i & 0x0c) << 2) | (color_prom[i >> 4] & 0x0f);
		else
			ctabentry = 0x80 | ((i & 0x03) << 4) | (color_prom[i >> 4] & 0x0f);

		palette.set_pen_indirect(0x110 + i_swapped, ctabentry);
	}
}

WRITE16_MEMBER(terracre_state::amazon_background_w)
{
	COMBINE_DATA( &m_bg_videoram[offset] );
	m_background->mark_tile_dirty(offset );
}

WRITE16_MEMBER(terracre_state::amazon_foreground_w)
{
	COMBINE_DATA( &m_fg_videoram[offset] );
	m_foreground->mark_tile_dirty(offset );
}

WRITE16_MEMBER(terracre_state::amazon_flipscreen_w)
{
	if( ACCESSING_BITS_0_7 )
	{
		machine().bookkeeping().coin_counter_w(0, data&0x01 );
		machine().bookkeeping().coin_counter_w(1, (data&0x02)>>1 );
		flip_screen_set(data&0x04);
	}
}

WRITE16_MEMBER(terracre_state::amazon_scrolly_w)
{
	COMBINE_DATA(&m_yscroll);
	m_background->set_scrolly(0,m_yscroll);
}

WRITE16_MEMBER(terracre_state::amazon_scrollx_w)
{
	COMBINE_DATA(&m_xscroll);
	m_background->set_scrollx(0,m_xscroll);
}

void terracre_state::video_start()
{
	m_background = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(terracre_state::get_bg_tile_info),this),TILEMAP_SCAN_COLS,16,16,64,32);
	m_foreground = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(terracre_state::get_fg_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,32);
	m_foreground->set_transparent_pen(0xf);

	/* register for saving */
	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
}

UINT32 terracre_state::screen_update_amazon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if( m_xscroll&0x2000 )
		bitmap.fill(m_palette->black_pen(), cliprect );
	else
		m_background->draw(screen, bitmap, cliprect, 0, 0 );

	draw_sprites(bitmap,cliprect );
	m_foreground->draw(screen, bitmap, cliprect, 0, 0 );
	return 0;
}
