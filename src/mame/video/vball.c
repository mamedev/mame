// license:BSD-3-Clause
// copyright-holders:Paul Hampson
/***************************************************************************

  Video Hardware for Championship V'ball by Paul Hampson
  Generally copied from China Gate by Paul Hampson
  "Mainly copied from video of Double Dragon (bootleg) & Double Dragon II"

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/vball.h"




/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(vball_state::background_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) <<6);
}

TILE_GET_INFO_MEMBER(vball_state::get_bg_tile_info)
{
	UINT8 code = m_videoram[tile_index];
	UINT8 attr = m_attribram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			code + ((attr & 0x1f) << 8) + (m_gfxset<<8),
			(attr >> 5) & 0x7,
			0);
}


void vball_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(vball_state::get_bg_tile_info),this),tilemap_mapper_delegate(FUNC(vball_state::background_scan),this), 8, 8,64,64);

	m_bg_tilemap->set_scroll_rows(32);
	m_gfxset=0;
	m_bgprombank=0xff;
	m_spprombank=0xff;

	save_item(NAME(m_scrollx_hi));
	save_item(NAME(m_scrolly_hi));
	save_item(NAME(m_scrollx_lo));
	save_item(NAME(m_gfxset));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_bgprombank));
	save_item(NAME(m_spprombank));
}

WRITE8_MEMBER(vball_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(vball_state::attrib_w)
{
	m_attribram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void vball_state::bgprombank_w( int bank )
{
	int i;
	UINT8* color_prom;

	if (bank==m_bgprombank) return;

	color_prom = memregion("proms")->base() + bank*0x80;
	for (i=0;i<128;i++, color_prom++) {
		m_palette->set_pen_color(i,pal4bit(color_prom[0] >> 0),pal4bit(color_prom[0] >> 4),
						pal4bit(color_prom[0x800] >> 0));
	}
	m_bgprombank=bank;
}

void vball_state::spprombank_w( int bank )
{
	int i;
	UINT8* color_prom;

	if (bank==m_spprombank) return;

	color_prom = memregion("proms")->base()+0x400 + bank*0x80;
	for (i=128;i<256;i++,color_prom++)  {
		m_palette->set_pen_color(i,pal4bit(color_prom[0] >> 0),pal4bit(color_prom[0] >> 4),
						pal4bit(color_prom[0x800] >> 0));
	}
	m_spprombank=bank;
}


#define DRAW_SPRITE( order, sx, sy ) gfx->transpen(bitmap,\
					cliprect, \
					(which+order),color,flipx,flipy,sx,sy,0);

void vball_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	UINT8 *src = m_spriteram;
	int i;

/*  240-Y    S|X|CLR|WCH WHICH    240-X
    xxxxxxxx x|x|xxx|xxx xxxxxxxx xxxxxxxx
*/
	for (i = 0;i < m_spriteram.bytes();i += 4)
	{
		int attr = src[i+1];
		int which = src[i+2]+((attr & 0x07)<<8);
		int sx = ((src[i+3] + 8) & 0xff) - 7;
		int sy = 240 - src[i];
		int size = (attr & 0x80) >> 7;
		int color = (attr & 0x38) >> 3;
		int flipx = ~attr & 0x40;
		int flipy = 0;
		int dy = -16;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
			dy = -dy;
		}

		switch (size)
		{
			case 0: /* normal */
			DRAW_SPRITE(0,sx,sy);
			break;

			case 1: /* double y */
			DRAW_SPRITE(0,sx,sy + dy);
			DRAW_SPRITE(1,sx,sy);
			break;
		}
	}
}

#undef DRAW_SPRITE

UINT32 vball_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	m_bg_tilemap->set_scrolly(0,m_scrolly_hi + *m_scrolly_lo);

	/*To get linescrolling to work properly, we must ignore the 1st two scroll values, no idea why! -SJE */
	for (i = 2; i < 256; i++) {
		m_bg_tilemap->set_scrollx(i,m_scrollx[i-2]);
		//logerror("scrollx[%d] = %d\n",i,m_scrollx[i]);
	}
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	return 0;
}
