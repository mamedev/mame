// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/*************************************************************************************

Submarine (c) 1985 Sigma

Video functions

*************************************************************************************/

#include "emu.h"
#include "includes/sub.h"

void sub_state::sub_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x100; i++)
	{
		int const r = color_prom[i | 0x000];
		int const g = color_prom[i | 0x100];
		int const b = color_prom[i | 0x200];

		palette.set_indirect_color(i, rgb_t(pal4bit(r), pal4bit(g), pal4bit(b)));
	}

	uint8_t const *const lookup = memregion("proms2")->base();
	for (int i = 0; i < 0x400; i++)
	{
		uint8_t const ctabentry = lookup[i | 0x400] | (lookup[i | 0x000] << 4);
		palette.set_pen_indirect(i, ctabentry);
	}
}

TILE_GET_INFO_MEMBER(sub_state::get_tile_info)
{
	int code = m_vram[tile_index] | ((m_attr[tile_index]&0xe0)<<3);
	int color = (m_attr[tile_index]&0x1f)+0x40;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

WRITE8_MEMBER(sub_state::vram_w)
{
	m_vram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(sub_state::attr_w)
{
	m_attr[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(sub_state::scrolly_w)
{
	m_scrolly[offset] = data;
	m_tilemap->set_scrolly(offset,m_scrolly[offset]);
}

void sub_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sub_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_tilemap->set_scroll_cols(32);
}

/*
sprite bank 1
0 xxxx xxxx X offset
1 tttt tttt tile offset
sprite bank 2
0 yyyy yyyy Y offset
1 f--- ---- flips the X offset
1 -f-- ---- flip y, inverted
1 --cc cccc color
*/
void sub_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);

	uint8_t *spriteram = m_spriteram;
	uint8_t *spriteram_2 = m_spriteram2;
	uint8_t x,y,spr_offs,i,col,dx,fx,fy;

	for(i=0;i<0x40;i+=2)
	{
		spr_offs = spriteram[i+1];
		x = spriteram[i+0];
		y = 0xe0 - spriteram_2[i+1];
		col = (spriteram_2[i+0])&0x3f;
		dx = (spriteram_2[i+0] & 0x80) ? 0 : 1;
		fy = (spriteram_2[i+0] & 0x40) ? 0 : 1;
		fx = 0;
		if (flip_screen())
		{
			x = 0xe0 - x;
			fx = 1;
			//fx ^= 1;
			//fy ^= 1;
		}

		if(dx)
			x = 0xe0 - x;


		gfx->transpen(bitmap,cliprect,spr_offs,col,fx,fy,x,y,0);
	}
}


uint32_t sub_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap,cliprect);

	/* re-draw score display above the sprites (window effect) */
	rectangle opaque_rect;
	opaque_rect.min_y = cliprect.min_y;
	opaque_rect.max_y = cliprect.max_y;
	opaque_rect.min_x = flip_screen() ? cliprect.min_x : (cliprect.max_x - 32);
	opaque_rect.max_x = flip_screen() ? (cliprect.min_x + 32) : cliprect.max_x;

	m_tilemap->draw(screen, bitmap, opaque_rect, 0, 0);

	return 0;
}

