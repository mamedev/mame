// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "emu.h"
#include "includes/deadang.h"
#include "screen.h"


/******************************************************************************/

void deadang_state::foreground_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_data[offset]);
	m_pf1_layer->mark_tile_dirty(offset );
}

void deadang_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_text_layer->mark_tile_dirty(offset );
}

void popnrun_state::popnrun_text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_text_layer->mark_tile_dirty(offset / 2);
}

void deadang_state::bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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
	const uint16_t *bgMap = (const uint16_t *)memregion("gfx6")->base();
	int code= bgMap[tile_index];
	tileinfo.set(4,code&0x7ff,code>>12,0);
}

TILE_GET_INFO_MEMBER(deadang_state::get_pf2_tile_info)
{
	const uint16_t *bgMap = (const uint16_t *)memregion("gfx7")->base();
	int code= bgMap[tile_index];
	tileinfo.set(3,code&0x7ff,code>>12,0);
}

TILE_GET_INFO_MEMBER(deadang_state::get_pf1_tile_info)
{
	int tile=m_video_data[tile_index];
	int color=tile >> 12;
	tile=tile&0xfff;

	tileinfo.set(2,tile+m_tilebank*0x1000,color,0);
}

TILE_GET_INFO_MEMBER(deadang_state::get_text_tile_info)
{
	int tile=(m_videoram[tile_index] & 0xff) | ((m_videoram[tile_index] >> 6) & 0x300);
	int color=(m_videoram[tile_index] >> 8)&0xf;

	tileinfo.set(0,tile,color,0);
}

void deadang_state::video_start()
{
	m_pf3_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deadang_state::get_pf3_tile_info)), tilemap_mapper_delegate(*this, FUNC(deadang_state::bg_scan)), 16, 16, 128, 256);
	m_pf2_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deadang_state::get_pf2_tile_info)), tilemap_mapper_delegate(*this, FUNC(deadang_state::bg_scan)), 16, 16, 128, 256);
	m_pf1_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deadang_state::get_pf1_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_text_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deadang_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_pf2_layer->set_transparent_pen(15);
	m_pf1_layer->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);

	save_item(NAME(m_tilebank));
	save_item(NAME(m_oldtilebank));
}


TILE_GET_INFO_MEMBER(popnrun_state::get_popnrun_text_tile_info)
{
	int tile = (m_videoram[tile_index*2+0] & 0xff) << 1; // | ((m_videoram[tile_index] >> 6) & 0x300);
	int attr = (m_videoram[tile_index*2+1]);
	// TODO: not entirely correct (title screen/ranking colors)
	// might be down to bitplanes too
	int color = (attr & 3) ^ 1;

	if(attr & 0x40)
		tile |= 1;

	tileinfo.set(0,tile,color,0);
}

void popnrun_state::video_start()
{
	m_pf3_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deadang_state::get_pf3_tile_info)), tilemap_mapper_delegate(*this, FUNC(deadang_state::bg_scan)), 16, 16, 128, 256);
	m_pf2_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deadang_state::get_pf2_tile_info)), tilemap_mapper_delegate(*this, FUNC(deadang_state::bg_scan)), 16, 16, 128, 256);
	m_pf1_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(deadang_state::get_pf1_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_text_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(popnrun_state::get_popnrun_text_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_pf2_layer->set_transparent_pen(0);
	m_pf1_layer->set_transparent_pen(0);
	m_text_layer->set_transparent_pen(0);

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

uint32_t deadang_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

void popnrun_state::popnrun_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs,fx,fy,x,y,color,sprite,pri;

	// TODO: might have more bits in either 0x3800-0x3bff or 0x3e00-0x3fff
	for (offs = 0; offs<0x200/2; offs+=2)
	{
		/* Don't draw empty sprite table entries */
		//if ((m_spriteram[offs+3] & 0xff00)!=0xf00) continue;

		pri = 0;
#if 0
		switch (m_spriteram[offs+2]&0xc000) {
		default:
		case 0xc000: pri=0; break; /* Unknown */
		case 0x8000: pri=0; break; /* Over all playfields */
		case 0x4000: pri=0xf0; break; /* Under top playfield */
		case 0x0000: pri=0xf0|0xcc; break; /* Under middle playfield */
		}
#endif

		fx = m_spriteram[offs+0]&0x4000;
		fy = m_spriteram[offs+0]&0x8000;
		y = m_spriteram[offs+1] & 0xff;
		x = (m_spriteram[offs+1] >> 8) & 0xff;
#if 0
		if (fy) fy=0; else fy=1;
		if (m_spriteram[offs+2]&0x100) x=0-(0xff-x);
#endif

		color = (m_spriteram[offs+0]>>12)&0x7;
		sprite = m_spriteram[offs+0]&0xfff;

#if 0
		if (flip_screen()) {
			x=240-x;
			y=240-y;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
		}
#endif

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				sprite,
				color,fx,fy,x,y,
				screen.priority(),pri,0);
	}
}

uint32_t popnrun_state::popnrun_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO: different scroll RAM hookup
	// 0x18 seems to enable the various layers
	/* Setup the tilemaps */
//  m_pf3_layer->set_scrolly(0, ((m_scroll_ram[0x01]&0xf0)<<4)+((m_scroll_ram[0x02]&0x7f)<<1)+((m_scroll_ram[0x02]&0x80)>>7) );
//  m_pf3_layer->set_scrollx(0, ((m_scroll_ram[0x09]&0xf0)<<4)+((m_scroll_ram[0x0a]&0x7f)<<1)+((m_scroll_ram[0x0a]&0x80)>>7) );
//  m_pf1_layer->set_scrolly(0, ((m_scroll_ram[0x11]&0x10)<<4)+((m_scroll_ram[0x12]&0x7f)<<1)+((m_scroll_ram[0x12]&0x80)>>7) );
//  m_pf1_layer->set_scrollx(0, ((m_scroll_ram[0x19]&0x10)<<4)+((m_scroll_ram[0x1a]&0x7f)<<1)+((m_scroll_ram[0x1a]&0x80)>>7) );
//  m_pf2_layer->set_scrolly(0, ((m_scroll_ram[0x21]&0xf0)<<4)+((m_scroll_ram[0x22]&0x7f)<<1)+((m_scroll_ram[0x22]&0x80)>>7) );
//  m_pf2_layer->set_scrollx(0, ((m_scroll_ram[0x29]&0xf0)<<4)+((m_scroll_ram[0x2a]&0x7f)<<1)+((m_scroll_ram[0x2a]&0x80)>>7) );

	m_pf3_layer->enable(!(m_scroll_ram[0x34]&1));
	m_pf1_layer->enable(!(m_scroll_ram[0x34]&2));
	m_pf2_layer->enable(!(m_scroll_ram[0x34]&4));
//  flip_screen_set(m_scroll_ram[0x34]&0x40 );

	bitmap.fill(1, cliprect);
	screen.priority().fill(0, cliprect);
	// 32 pixels?
//  int scrollx = (m_scroll_ram[0x4/2] & 0x0f);

	// debug tilemap code
	// this is likely to be collision data
	for(int x=0;x<16;x++)
	{
		for(int y=0;y<8;y++)
		{
			int tile = m_video_data[y+x*8+0xc0] & 0xff;
			int res_x, res_y;

			if(tile != 0)
			{
				res_x = (x*16) & 0xff;
				res_y = y*32;
				//if(cliprect.contains(res_x,res_y))
				bitmap.plot_box(res_x,res_y,16,16,tile+0x10);
			}

			tile = m_video_data[y+x*8+0xc0] >> 8;

			if(tile != 0)
			{
				res_x = (x*16) & 0xff;
				res_y = y*32+16;
				//if(cliprect.contains(res_x,res_y))
				bitmap.plot_box(res_x,res_y,16,16,tile+0x10);
			}
		}
	}

	//m_pf3_layer->draw(screen, bitmap, cliprect, 0,1);
	//m_pf1_layer->draw(screen, bitmap, cliprect, 0,2);
	//m_pf2_layer->draw(screen, bitmap, cliprect, 0,4);
	if (m_scroll_ram[0x18/2]&0x1)
		popnrun_draw_sprites(screen, bitmap,cliprect);
	m_text_layer->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
