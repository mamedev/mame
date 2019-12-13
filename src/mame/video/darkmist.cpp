// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria, Tomasz Slanina
#include "emu.h"
#include "includes/darkmist.h"



/* vis. flags */

#define DISPLAY_SPR     1
#define DISPLAY_FG      2 /* 2 or 8 */
#define DISPLAY_BG      4
#define DISPLAY_TXT     16


TILE_GET_INFO_MEMBER(darkmist_state::get_bgtile_info)
{
	int code,attr,pal;

	code=memregion("bg_map")->base()[tile_index*2]; /* TTTTTTTT */
	attr=memregion("bg_map")->base()[(tile_index*2)+1]; /* -PPP--TT - FIXED BITS (0xxx00xx) */

	code+=(attr&3)<<8;
	pal=(attr>>4) & 0xf;

	SET_TILE_INFO_MEMBER(1,
		code,
		pal,
		0);
}

TILE_GET_INFO_MEMBER(darkmist_state::get_fgtile_info)
{
	int code,attr,pal;

	code = memregion("fg_map")->base()[tile_index*2]; /* TTTTTTTT */
	attr = memregion("fg_map")->base()[(tile_index*2)+1]; /* -PPP--TT - FIXED BITS (0xxx00xx) */

	code+=(attr&3)<<8;
	pal=(attr>>4) & 0xf;

	SET_TILE_INFO_MEMBER(2,
		code,
		pal,
		0);
}

TILE_GET_INFO_MEMBER(darkmist_state::get_txttile_info)
{
	int code,attr,pal;

	code=m_videoram[tile_index];
	attr=m_videoram[tile_index+0x400];
	pal=(attr>>1);

	code+=(attr&1)<<8;

	SET_TILE_INFO_MEMBER(0,
		code,
		pal & 0xf,
		0);
}

void darkmist_state::darkmist_palette(palette_device &palette) const
{
	//palette.set_indirect_color(0x100, rgb_t::black());

	std::pair<uint8_t const *, uint8_t> const planes[4]{
			{ &m_bg_clut[0], 0x80 },
			{ &m_fg_clut[0], 0x00 },
			{ &m_spr_clut[0], 0x40 },
			{ &m_tx_clut[0], 0xc0 } };

	for (unsigned plane = 0; ARRAY_LENGTH(planes) > plane; ++plane)
	{
		for (unsigned i = 0; 0x100 > i; ++i)
		{
			uint8_t const clut = planes[plane].first[i];
//          if (clut & 0x40) // 0x40 indicates transparent pen
//              ctabentry = 0x100;
//          else
			int const ctabentry = (clut & 0x3f) | planes[plane].second;
			palette.set_pen_indirect((plane << 8) | i, ctabentry);
		}
	}
}


void darkmist_state::video_start()
{
	m_bgtilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darkmist_state::get_bgtile_info)), TILEMAP_SCAN_ROWS, 16, 16, 512, 64);
	m_fgtilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darkmist_state::get_fgtile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 256);
	m_txtilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(darkmist_state::get_txttile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
//  m_fgtilemap->set_transparent_pen(0);
//  m_txtilemap->set_transparent_pen(0);

	save_item(NAME(m_hw));
	m_screen->register_screen_bitmap(m_temp_bitmap);
}

// TODO: move this code into framework or substitute with a valid alternative
void darkmist_state::mix_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* clut)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t *dest = &bitmap.pix16(y);
		uint16_t *src = &m_temp_bitmap.pix16(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t pix = (src[x] & 0xff);
			uint16_t real = clut[pix];

			if (!(real & 0x40))
				dest[x] = src[x];
		}
	}
}

/*
    Sprites

    76543210
0 - TTTT TTTT - tile
1 - xyBP PPP? - palette (P), flips (x,y), B - use spritebank,
                ? - unknown, according to gamecode top bit of one of coords(y/x)
2 - YYYY YYYY - y coord
3 - XXXX XXXX - x coord

*/
void darkmist_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i,fx,fy,tile,palette;
	// fetch from top to bottom
	for(i=m_spriteram.bytes()-32;i>=0;i-=32)
	{
		fy=m_spriteram[i+1]&0x40;
		fx=m_spriteram[i+1]&0x80;

		tile=m_spriteram[i+0];

		if(m_spriteram[i+1]&0x20)
			tile += (*m_spritebank << 8);

		palette=((m_spriteram[i+1])>>1)&0xf;

		if(m_spriteram[i+1]&0x1)
			palette=machine().rand()&15;

		m_gfxdecode->gfx(3)->transpen(
		bitmap,cliprect,
		tile,
		palette,
		fx,fy,
		m_spriteram[i+3],m_spriteram[i+2],0 );
	}
}

uint32_t darkmist_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#define DM_GETSCROLL(n) (((m_scroll[(n)]<<1)&0xff) + ((m_scroll[(n)]&0x80)?1:0) +( ((m_scroll[(n)-1]<<4) | (m_scroll[(n)-1]<<12) )&0xff00))

	m_bgtilemap->set_scrollx(0, DM_GETSCROLL(0x2));
	m_bgtilemap->set_scrolly(0, DM_GETSCROLL(0x6));
	m_fgtilemap->set_scrollx(0, DM_GETSCROLL(0xa));
	m_fgtilemap->set_scrolly(0, DM_GETSCROLL(0xe));

	m_temp_bitmap.fill(0,cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	if(m_hw & DISPLAY_BG)
	{
		m_bgtilemap->draw(screen, m_temp_bitmap, cliprect, 0,0);
		mix_layer(screen, bitmap, cliprect, m_bg_clut);
	}

	if(m_hw & DISPLAY_FG)
	{
		m_fgtilemap->draw(screen, m_temp_bitmap, cliprect, 0,0);
		mix_layer(screen, bitmap, cliprect, m_fg_clut);
	}

	if(m_hw & DISPLAY_SPR)
	{
		draw_sprites(m_temp_bitmap,cliprect);
		mix_layer(screen, bitmap, cliprect, m_spr_clut);
	}

	if(m_hw & DISPLAY_TXT)
	{
		m_txtilemap->draw(screen, m_temp_bitmap, cliprect, 0,0);
		mix_layer(screen, bitmap, cliprect, m_tx_clut);
	}

	return 0;
}

WRITE8_MEMBER(darkmist_state::tx_vram_w)
{
	m_videoram[offset] = data;
	m_txtilemap->mark_tile_dirty(offset & 0x3ff);
}
