// license:BSD-3-Clause
// copyright-holders:Uki
/******************************************************************************

Quiz DNA no Hanran (c) 1992 Face
Quiz Gakuen Paradise (c) 1991 NMK
Quiz Gekiretsu Scramble (Gakuen Paradise 2) (c) 1993 Face

Video hardware
    driver by Uki

******************************************************************************/

#include "emu.h"
#include "includes/quizdna.h"


TILE_GET_INFO_MEMBER(quizdna_state::get_bg_tile_info)
{
	int code = m_bg_ram[tile_index*2] + m_bg_ram[tile_index*2+1]*0x100 ;
	int col = m_bg_ram[tile_index*2+0x1000] & 0x7f;

	if (code>0x7fff)
		code &= 0x83ff;

	SET_TILE_INFO_MEMBER(1, code, col, 0);
}

TILE_GET_INFO_MEMBER(quizdna_state::get_fg_tile_info)
{
	int code,col,x,y;
	UINT8 *FG = memregion("user1")->base();

	x = tile_index & 0x1f;
	y = FG[(tile_index >> 5) & 0x1f] & 0x3f;
	code = y & 1;

	y >>= 1;

	col = m_fg_ram[x*2 + y*0x40 + 1];
	code += (m_fg_ram[x*2 + y*0x40] + (col & 0x1f) * 0x100) * 2;
	col >>= 5;
	col = (col & 3) | ((col & 4) << 1);

	SET_TILE_INFO_MEMBER(0, code, col, 0);
}


void quizdna_state::video_start()
{
	m_flipscreen = -1;
	m_video_enable = 0;
	m_bg_xscroll[0] = 0;
	m_bg_xscroll[1] = 0;

	m_bg_ram = std::make_unique<UINT8[]>(0x2000);
	m_fg_ram = std::make_unique<UINT8[]>(0x1000);

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(quizdna_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32 );
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(quizdna_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,16,8,32,32 );

	m_fg_tilemap->set_transparent_pen(0 );

	save_pointer(NAME(m_bg_ram.get()), 0x2000);
	save_pointer(NAME(m_fg_ram.get()), 0x1000);
	save_item(NAME(m_bg_xscroll));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_video_enable));
}

WRITE8_MEMBER(quizdna_state::bg_ram_w)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_bg_ram[offset] = data;
	RAM[0x12000+offset] = data;

	m_bg_tilemap->mark_tile_dirty((offset & 0xfff) / 2 );
}

WRITE8_MEMBER(quizdna_state::fg_ram_w)
{
	int i;
	int offs = offset & 0xfff;
	UINT8 *RAM = memregion("maincpu")->base();

	RAM[0x10000+offs] = data;
	RAM[0x11000+offs] = data; /* mirror */
	m_fg_ram[offs] = data;

	for (i=0; i<32; i++)
		m_fg_tilemap->mark_tile_dirty(((offs/2) & 0x1f) + i*0x20 );
}

WRITE8_MEMBER(quizdna_state::bg_yscroll_w)
{
	m_bg_tilemap->set_scrolldy(255-data, 255-data+1 );
}

WRITE8_MEMBER(quizdna_state::bg_xscroll_w)
{
	int x;
	m_bg_xscroll[offset] = data;
	x = ~(m_bg_xscroll[0] + m_bg_xscroll[1]*0x100) & 0x1ff;

	m_bg_tilemap->set_scrolldx(x+64, x-64+10 );
}

WRITE8_MEMBER(quizdna_state::screen_ctrl_w)
{
	int tmp = (data & 0x10) >> 4;
	m_video_enable = data & 0x20;

	machine().bookkeeping().coin_counter_w(0, data & 1);

	if (m_flipscreen == tmp)
		return;

	m_flipscreen = tmp;

	flip_screen_set(tmp);
	m_fg_tilemap->set_scrolldx(64, -64 +16);
}

WRITE8_MEMBER(quizdna_state::paletteram_xBGR_RRRR_GGGG_BBBB_w)
{
	int r,g,b,d0,d1;
	int offs = offset & ~1;

	m_generic_paletteram_8[offset] = data;

	d0 = m_generic_paletteram_8[offs];
	d1 = m_generic_paletteram_8[offs+1];

	r = ((d1 << 1) & 0x1e) | ((d1 >> 4) & 1);
	g = ((d0 >> 3) & 0x1e) | ((d1 >> 5) & 1);
	b = ((d0 << 1) & 0x1e) | ((d1 >> 6) & 1);

	m_palette->set_pen_color(offs/2,pal5bit(r),pal5bit(g),pal5bit(b));
}

void quizdna_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs<m_spriteram.bytes(); offs+=8)
	{
		int x = m_spriteram[offs + 3]*0x100 + m_spriteram[offs + 2] + 64 - 8;
		int y = (m_spriteram[offs + 1] & 1)*0x100 + m_spriteram[offs + 0];
		int code = (m_spriteram[offs + 5] * 0x100 + m_spriteram[offs + 4]) & 0x3fff;
		int col =  m_spriteram[offs + 6];
		int fx = col & 0x80;
		int fy = col & 0x40;
		int ysize = (m_spriteram[offs + 1] & 0xc0) >> 6;
		int dy = 0x10;
		col &= 0x1f;

		if (m_flipscreen)
		{
			x -= 7;
			y += 1;
		}

		x &= 0x1ff;
		if (x>0x1f0)
			x -= 0x200;

		if (fy)
		{
			dy = -0x10;
			y += 0x10 * ysize;
		}

		if (code >= 0x2100)
			code &= 0x20ff;

		for (int i=0; i<ysize+1; i++)
		{
			y &= 0x1ff;

			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code ^ i,
					col,
					fx,fy,
					x,y,0);

			y += dy;
		}
	}
}

UINT32 quizdna_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video_enable)
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
		bitmap.fill(m_palette->black_pen(), cliprect);
	return 0;
}
