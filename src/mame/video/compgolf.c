// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli, Bryan McPhail
/****************************************************************************************

 Competition Golf Final Round
 video hardware emulation

****************************************************************************************/

#include "emu.h"
#include "includes/compgolf.h"


PALETTE_INIT_MEMBER(compgolf_state, compgolf)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0,bit1,bit2,r,g,b;
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}

WRITE8_MEMBER(compgolf_state::compgolf_video_w)
{
	m_videoram[offset] = data;
	m_text_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(compgolf_state::compgolf_back_w)
{
	m_bg_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(compgolf_state::get_text_info)
{
	tile_index <<= 1;
	SET_TILE_INFO_MEMBER(2, m_videoram[tile_index + 1] | (m_videoram[tile_index] << 8), m_videoram[tile_index] >> 2, 0);
}

TILEMAP_MAPPER_MEMBER(compgolf_state::back_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

TILE_GET_INFO_MEMBER(compgolf_state::get_back_info)
{
	int attr = m_bg_ram[tile_index * 2];
	int code = m_bg_ram[tile_index * 2 + 1] + ((attr & 1) << 8);
	int color = (attr & 0x3e) >> 1;

	SET_TILE_INFO_MEMBER(1, code, color, 0);
}

void compgolf_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(compgolf_state::get_back_info),this), tilemap_mapper_delegate(FUNC(compgolf_state::back_scan),this), 16, 16, 32, 32);
	m_text_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(compgolf_state::get_text_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_text_tilemap->set_transparent_pen(0);
}

/*
preliminary sprite list:
       0        1        2        3
xx------ xxxxxxxx -------- -------- sprite code
---x---- -------- -------- -------- Double Height
----x--- -------- -------- -------- Color,all of it?
-------- -------- xxxxxxxx -------- Y pos
-------- -------- -------- xxxxxxxx X pos
-----x-- -------- -------- -------- Flip X
-------- -------- -------- -------- Flip Y(used?)
*/
void compgolf_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs, fx, fy, x, y, color, sprite;

	for (offs = 0; offs < 0x60; offs += 4)
	{
		sprite = m_spriteram[offs + 1] + (((m_spriteram[offs] & 0xc0) >> 6) * 0x100);
		x = 240 - m_spriteram[offs + 3];
		y = m_spriteram[offs + 2];
		color = (m_spriteram[offs] & 8)>>3;
		fx = m_spriteram[offs] & 4;
		fy = 0; /* ? */

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				sprite,
				color,fx,fy,x,y,0);

		/* Double Height */
		if(m_spriteram[offs] & 0x10)
		{
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				sprite + 1,
				color, fx, fy, x, y + 16, 0);
		}
	}
}

UINT32 compgolf_state::screen_update_compgolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int scrollx = m_scrollx_hi + m_scrollx_lo;
	int scrolly = m_scrolly_hi + m_scrolly_lo;

	m_bg_tilemap->set_scrollx(0, scrollx);
	m_bg_tilemap->set_scrolly(0, scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
