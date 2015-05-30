// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "emu.h"
#include "includes/deadang.h"


/******************************************************************************/

WRITE16_MEMBER(deadang_state::foreground_w)
{
	COMBINE_DATA(&m_video_data[offset]);
	m_pf1_layer->mark_tile_dirty(offset );
}

WRITE16_MEMBER(deadang_state::text_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_text_layer->mark_tile_dirty(offset );
}

WRITE16_MEMBER(deadang_state::bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_tilebank = data&1;
		if (m_tilebank!=m_oldtilebank)
		{
			m_oldtilebank = m_tilebank;
			m_pf1_layer->mark_all_dirty();
		}
	}
}

/******************************************************************************/

TILEMAP_MAPPER_MEMBER(deadang_state::bg_scan)
{
	return (col&0xf) | ((row&0xf)<<4) | ((col&0x70)<<4) | ((row&0xf0)<<7);
}

TILE_GET_INFO_MEMBER(deadang_state::get_pf3_tile_info)
{
	const UINT16 *bgMap = (const UINT16 *)memregion("gfx6")->base();
	int code= bgMap[tile_index];
	SET_TILE_INFO_MEMBER(4,code&0x7ff,code>>12,0);
}

TILE_GET_INFO_MEMBER(deadang_state::get_pf2_tile_info)
{
	const UINT16 *bgMap = (const UINT16 *)memregion("gfx7")->base();
	int code= bgMap[tile_index];
	SET_TILE_INFO_MEMBER(3,code&0x7ff,code>>12,0);
}

TILE_GET_INFO_MEMBER(deadang_state::get_pf1_tile_info)
{
	int tile=m_video_data[tile_index];
	int color=tile >> 12;
	tile=tile&0xfff;

	SET_TILE_INFO_MEMBER(2,tile+m_tilebank*0x1000,color,0);
}

TILE_GET_INFO_MEMBER(deadang_state::get_text_tile_info)
{
	int tile=(m_videoram[tile_index] & 0xff) | ((m_videoram[tile_index] >> 6) & 0x300);
	int color=(m_videoram[tile_index] >> 8)&0xf;

	SET_TILE_INFO_MEMBER(0,tile,color,0);
}

void deadang_state::video_start()
{
	m_pf3_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deadang_state::get_pf3_tile_info),this),tilemap_mapper_delegate(FUNC(deadang_state::bg_scan),this),16,16,128,256);
	m_pf2_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deadang_state::get_pf2_tile_info),this),tilemap_mapper_delegate(FUNC(deadang_state::bg_scan),this),16,16,128,256);
	m_pf1_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deadang_state::get_pf1_tile_info),this),TILEMAP_SCAN_COLS,16,16, 32, 32);
	m_text_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(deadang_state::get_text_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_pf2_layer->set_transparent_pen(15);
	m_pf1_layer->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);

	save_item(NAME(m_tilebank));
	save_item(NAME(m_oldtilebank));
}

void deadang_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs,fx,fy,x,y,color,sprite,pri;

	for (offs = 0; offs<0x800/2; offs+=4)
	{
		/* Don't draw empty sprite table entries */
		if ((m_spriteram[offs+3] & 0xff00)!=0xf00) continue;

		switch (m_spriteram[offs+2]&0xc000) {
		default:
		case 0xc000: pri=0; break; /* Unknown */
		case 0x8000: pri=0; break; /* Over all playfields */
		case 0x4000: pri=0xf0; break; /* Under top playfield */
		case 0x0000: pri=0xf0|0xcc; break; /* Under middle playfield */
		}

		fx= m_spriteram[offs+0]&0x2000;
		fy= m_spriteram[offs+0]&0x4000;
		y = m_spriteram[offs+0] & 0xff;
		x = m_spriteram[offs+2] & 0xff;
		if (fy) fy=0; else fy=1;
		if (m_spriteram[offs+2]&0x100) x=0-(0xff-x);

		color = (m_spriteram[offs+1]>>12)&0xf;
		sprite = m_spriteram[offs+1]&0xfff;

		if (flip_screen()) {
			x=240-x;
			y=240-y;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
		}

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				sprite,
				color,fx,fy,x,y,
				screen.priority(),pri,15);
	}
}

UINT32 deadang_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Setup the tilemaps */
	m_pf3_layer->set_scrolly(0, ((m_scroll_ram[0x01]&0xf0)<<4)+((m_scroll_ram[0x02]&0x7f)<<1)+((m_scroll_ram[0x02]&0x80)>>7) );
	m_pf3_layer->set_scrollx(0, ((m_scroll_ram[0x09]&0xf0)<<4)+((m_scroll_ram[0x0a]&0x7f)<<1)+((m_scroll_ram[0x0a]&0x80)>>7) );
	m_pf1_layer->set_scrolly(0, ((m_scroll_ram[0x11]&0x10)<<4)+((m_scroll_ram[0x12]&0x7f)<<1)+((m_scroll_ram[0x12]&0x80)>>7) );
	m_pf1_layer->set_scrollx(0, ((m_scroll_ram[0x19]&0x10)<<4)+((m_scroll_ram[0x1a]&0x7f)<<1)+((m_scroll_ram[0x1a]&0x80)>>7) );
	m_pf2_layer->set_scrolly(0, ((m_scroll_ram[0x21]&0xf0)<<4)+((m_scroll_ram[0x22]&0x7f)<<1)+((m_scroll_ram[0x22]&0x80)>>7) );
	m_pf2_layer->set_scrollx(0, ((m_scroll_ram[0x29]&0xf0)<<4)+((m_scroll_ram[0x2a]&0x7f)<<1)+((m_scroll_ram[0x2a]&0x80)>>7) );

	/* Control byte:
	    0x01: Background playfield disable
	    0x02: Middle playfield disable
	    0x04: Top playfield disable
	    0x08: ?  Toggles at start of game
	    0x10: Sprite disable
	    0x20: Unused?
	    0x40: Flipscreen
	    0x80: Always set?
	*/
	m_pf3_layer->enable(!(m_scroll_ram[0x34]&1));
	m_pf1_layer->enable(!(m_scroll_ram[0x34]&2));
	m_pf2_layer->enable(!(m_scroll_ram[0x34]&4));
	flip_screen_set(m_scroll_ram[0x34]&0x40 );

	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);
	m_pf3_layer->draw(screen, bitmap, cliprect, 0,1);
	m_pf1_layer->draw(screen, bitmap, cliprect, 0,2);
	m_pf2_layer->draw(screen, bitmap, cliprect, 0,4);
	if (!(m_scroll_ram[0x34]&0x10)) draw_sprites(screen, bitmap,cliprect);
	m_text_layer->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
