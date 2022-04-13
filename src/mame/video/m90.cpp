// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*****************************************************************************

    Irem M90 system.  There is 1 video chip - NANAO GA-25, it produces
    2 tilemaps and sprites.  16 control bytes:

    0:  Playfield 1 X scroll
    2:  Playfield 1 Y scroll
    4:  Playfield 2 X scroll
    6:  Playfield 2 Y scroll
    8:  Bit 0x01 - unknown (set by hasamu)
    10: Playfield 1 control
        Bits0x03 - Playfield 1 VRAM base
        Bit 0x04 - Playfield 1 width (0 is 64 tiles, 0x4 is 128 tiles)
        Bit 0x10 - Playfield 1 disable
        Bit 0x20 - Playfield 1 rowscroll enable
        Bit 0x40 - Playfield 1 y-offset table enable
    12: Playfield 2 control
        Bits0x03 - Playfield 2 VRAM base
        Bit 0x04 - Playfield 2 width (0 is 64 tiles, 0x4 is 128 tiles)
        Bit 0x10 - Playfield 2 disable
        Bit 0x20 - Playfield 2 rowscroll enable
        Bit 0x40 - Playfield 2 y-offset table enable
    14: Bits0x03 - Sprite/Tile Priority (related to sprite color)

    Emulation by Bryan McPhail, mish@tendril.co.uk, thanks to Chris Hardy!

*****************************************************************************/

#include "emu.h"
#include "includes/m90.h"
#include "screen.h"

#include <algorithm>

TILE_GET_INFO_MEMBER(m90_state::get_tile_info)
{
	uint16_t *vram = (uint16_t*)tilemap.user_data();
	int tile,color;

	tile=vram[tile_index<<1];
	color=vram[(tile_index<<1)|1];
	tileinfo.set(0,
			tile,
			color&0xf,
			TILE_FLIPYX((color & 0xc0) >> 6));
			tileinfo.category = (color & 0x30) ? 1 : 0;
}

void m90_state::common_tilemap_init()
{
	m_pf_layer[0][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m90_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8,  64,64);
	m_pf_layer[0][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m90_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 128,64);
	m_pf_layer[1][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m90_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8,  64,64);
	m_pf_layer[1][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m90_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 128,64);

	// fix for bootlegs
	m_pf_layer[0][0]->set_user_data(&m_video_data[0x0000]);
	m_pf_layer[0][1]->set_user_data(&m_video_data[0x0000]);
	m_pf_layer[1][0]->set_user_data(&m_video_data[0x4000]);
	m_pf_layer[1][1]->set_user_data(&m_video_data[0x4000]);
}

void m90_state::video_start()
{
	common_tilemap_init();

	m_pf_layer[0][0]->set_transparent_pen(0);
	m_pf_layer[0][1]->set_transparent_pen(0);

	m_last_pf[0] = 0;
	m_last_pf[1] = 2;
}

VIDEO_START_MEMBER(m90_state,bomblord)
{
	common_tilemap_init();

	m_pf_layer[1][0]->set_transparent_pen(0);
	m_pf_layer[1][1]->set_transparent_pen(0);
	m_pf_layer[0][0]->set_transparent_pen(0);
	m_pf_layer[0][1]->set_transparent_pen(0);
}

VIDEO_START_MEMBER(m90_state,dynablsb)
{
	common_tilemap_init();

	m_pf_layer[1][0]->set_transparent_pen(0);
	m_pf_layer[1][1]->set_transparent_pen(0);
}

void m90_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	uint16_t *spriteram = m_video_data + 0xee00/2;
	int offs;

	for (offs = 0x1f2/2; offs >= 0; offs -= 3)
	{
		int x,y,sprite,colour,fx,fy,y_multi,i;

		sprite = spriteram[offs+1];
		colour = (spriteram[offs+0] >> 9) & 0x0f;

		y = spriteram[offs+0] & 0x1ff;
		x = spriteram[offs+2] & 0x1ff;

		x = x - 16;
		y = 512 - y;

		fx = (spriteram[offs+2] >> 8) & 0x02;
		fy = (spriteram[offs+0] >> 8) & 0x80;

		y_multi = 1 << ((spriteram[offs+0] & 0x6000) >> 13);
		y -= 16 * y_multi;

		for (i = 0;i < y_multi;i++)

			if (m_video_control_data[7] & 0x01)
				m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					screen.priority(),
					(colour & 0x08) ? 0x00 : 0x02,0);
			else if (m_video_control_data[7] & 0x02)
				m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					screen.priority(),
					((colour & 0x0c)==0x0c) ? 0x00 : 0x02,0);
			else
				m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
					sprite + (fy ? y_multi-1 - i : i),
					colour,
					fx,fy,
					x,y+i*16,
					screen.priority(),
					0x02,0);
	}
}

void m90_state::bomblord_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int offs = 0, last_sprite = 0;
	int x,y,sprite,colour,fx,fy;


	while ((offs < m_spriteram.bytes()/2) && (m_spriteram[offs+0] != 0x8000))
	{
		last_sprite = offs;
		offs += 4;
	}

	for (offs = last_sprite; offs >= 0; offs -= 4)
	{
		sprite = m_spriteram[offs+1];
		colour = (m_spriteram[offs+2] >> 9) & 0x0f;

		y = (m_spriteram[offs+0] & 0x1ff) + 152;
		x = (m_spriteram[offs+3] & 0x1ff) + 16;

		x = x - 16;
		y = 512 - y;

		if (y < 0) y += 512;

		fx = (m_spriteram[offs+3] >> 8) & 0x02;
		fy = (m_spriteram[offs+2] >> 8) & 0x80;

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				sprite,
				colour,
				fx,fy,
				x,y,
				screen.priority(),
				(colour & 0x08) ? 0x00 : 0x02,0);
	}
}

void m90_state::dynablsb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int offs = 0, last_sprite = 0;
	int x,y,sprite,colour,fx,fy;

	while ((offs < m_spriteram.bytes()/2) && (m_spriteram[offs+0] != 0xffff))
	{
		last_sprite = offs;
		offs += 4;
	}

	for (offs = last_sprite; offs >= 0; offs -= 4)
	{
		sprite = m_spriteram[offs+1];
		colour = (m_spriteram[offs+2] >> 9) & 0x0f;

		y = (m_spriteram[offs+0] & 0x1ff) + 288;
		x = (m_spriteram[offs+3] & 0x1ff) - 64;

		x = x - 16;
		y = 512 - y;

		if (y < 0) y += 512;

		fx = (m_spriteram[offs+3] >> 8) & 0x02;
		fy = (m_spriteram[offs+2] >> 8) & 0x80;

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				sprite,
				colour,
				fx,fy,
				x,y,
				screen.priority(),
				(colour & 0x08) ? 0x00 : 0x02,0);
	}
}

void m90_state::m90_video_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_data[offset]);

	int page[2] = { m_video_control_data[5] & 0x3, m_video_control_data[6] & 0x3 };
	for (int i = 0; i < 2; i++)
	{
		if ((offset >> 13) == page[i])
			m_pf_layer[i][0]->mark_tile_dirty((offset & 0x1fff) >> 1);

		page[i] >>= 1;
		if ((offset >> 14) == page[i])
			m_pf_layer[i][1]->mark_tile_dirty((offset & 0x3fff) >> 1);
	}
}

void m90_state::bootleg_video_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_data[offset]);

	int layer = offset >> 14;
	if (!(offset & 0x2000))
		m_pf_layer[layer][0]->mark_tile_dirty((offset & 0x1fff) >> 1);

	m_pf_layer[layer][1]->mark_tile_dirty((offset & 0x3fff) >> 1);
}

uint32_t m90_state::screen_update_m90(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const layer_ctrl[2] = { m_video_control_data[5], m_video_control_data[6] };

	bool const video_enable = !(m_video_control_data[7] & 0x04);
	bool const pf_enable[2] = { !(m_video_control_data[5] & 0x10), !(m_video_control_data[6] & 0x10) };
	int const clip_miny = std::min(cliprect.min_y, 511);
	int const clip_maxy = std::min(cliprect.max_y, 511);

// m_pf_layer[0][0]->enable(pf_enable[0]);
// m_pf_layer[1][0]->enable(pf_enable[1]);
// m_pf_layer[0][1]->enable(pf_enable[0]);
// m_pf_layer[1][1]->enable(pf_enable[1]);

	constexpr int rowscroll_offs[2] = { 0xf000 >> 1, 0xf400 >> 1 };
	constexpr int rowscroll_bias[2] = { 2, -2 };
	for (int layer = 0; layer < 2; layer++)
	{
		/* Dirty tilemaps if VRAM base changes */
		uint8_t const pf_base = uint8_t(layer_ctrl[layer] & 0x3);
		if (pf_base != m_last_pf[layer])
		{
			m_pf_layer[layer][0]->set_user_data(&m_video_data[pf_base << 13]);
			m_pf_layer[layer][1]->set_user_data(&m_video_data[(pf_base & ~1) << 13]);
			m_pf_layer[layer][0]->mark_all_dirty();
			m_pf_layer[layer][1]->mark_all_dirty();
			m_last_pf[layer] = pf_base;
		}

		/* Setup scrolling */
		if (layer_ctrl[layer] & 0x20)
		{
			m_pf_layer[layer][0]->set_scroll_rows(512);
			m_pf_layer[layer][1]->set_scroll_rows(512);

			for (int i = 0; i < 512; i++)
			{
				m_pf_layer[layer][0]->set_scrollx(i, m_video_data[rowscroll_offs[layer] + i] + rowscroll_bias[layer]);
				m_pf_layer[layer][1]->set_scrollx(i, m_video_data[rowscroll_offs[layer] + i] + rowscroll_bias[layer] + 256);
			}
		}
		else
		{
			m_pf_layer[layer][0]->set_scroll_rows(1);
			m_pf_layer[layer][1]->set_scroll_rows(1);
			m_pf_layer[layer][0]->set_scrollx(0, m_video_control_data[1 | (layer<<1)] + rowscroll_bias[layer]);
			m_pf_layer[layer][1]->set_scrollx(0, m_video_control_data[1 | (layer<<1)] + rowscroll_bias[layer] + 256);
		}
	}

	if (!video_enable)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	screen.priority().fill(0, cliprect);

	if (pf_enable[1])
	{
		// use the playfield 2 y-offset table for each scanline
		if (layer_ctrl[1] & 0x40)
		{
			rectangle clip;
			clip.min_x = cliprect.min_x;
			clip.max_x = cliprect.max_x;

			for (int line = clip_miny; line <= clip_maxy; line++)
			{
				clip.min_y = clip.max_y = line;

				if (layer_ctrl[1] & 0x4)
				{
					m_pf_layer[1][1]->set_scrolly(0, 0x200 + m_video_data[0xfc00/2 + line]);
					m_pf_layer[1][1]->draw(screen, bitmap, clip, 0,0);
					m_pf_layer[1][1]->draw(screen, bitmap, clip, 1,1);
				}
				else
				{
					m_pf_layer[1][0]->set_scrolly(0, 0x200 + m_video_data[0xfc00/2 + line]);
					m_pf_layer[1][0]->draw(screen, bitmap, clip, 0,0);
					m_pf_layer[1][0]->draw(screen, bitmap, clip, 1,1);
				}
			}
		}
		else
		{
			if (layer_ctrl[1] & 0x4)
			{
				m_pf_layer[1][1]->set_scrolly(0, m_video_control_data[2] );
				m_pf_layer[1][1]->draw(screen, bitmap, cliprect, 0,0);
				m_pf_layer[1][1]->draw(screen, bitmap, cliprect, 1,1);
			}
			else
			{
				m_pf_layer[1][0]->set_scrolly(0, m_video_control_data[2] );
				m_pf_layer[1][0]->draw(screen, bitmap, cliprect, 0,0);
				m_pf_layer[1][0]->draw(screen, bitmap, cliprect, 1,1);
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	if (pf_enable[0])
	{
		// use the playfield 1 y-offset table for each scanline
		if (layer_ctrl[0] & 0x40)
		{
			rectangle clip;
			clip.min_x = cliprect.min_x;
			clip.max_x = cliprect.max_x;

			for (int line = clip_miny; line <= clip_maxy; line++)
			{
				clip.min_y = clip.max_y = line;

				if (layer_ctrl[0] & 0x4)
				{
					m_pf_layer[0][1]->set_scrolly(0, 0x200 + m_video_data[0xf800/2 + line]);
					m_pf_layer[0][1]->draw(screen, bitmap, clip, 0,0);
					m_pf_layer[0][1]->draw(screen, bitmap, clip, 1,1);
				}
				else
				{
					m_pf_layer[0][0]->set_scrolly(0, 0x200 + m_video_data[0xf800/2 + line]);
					m_pf_layer[0][0]->draw(screen, bitmap, clip, 0,0);
					m_pf_layer[0][0]->draw(screen, bitmap, clip, 1,1);
				}
			}
		}
		else
		{
			if (layer_ctrl[0] & 0x4)
			{
				m_pf_layer[0][1]->set_scrolly(0, m_video_control_data[0] );
				m_pf_layer[0][1]->draw(screen, bitmap, cliprect, 0,0);
				m_pf_layer[0][1]->draw(screen, bitmap, cliprect, 1,1);
			}
			else
			{
				m_pf_layer[0][0]->set_scrolly(0, m_video_control_data[0] );
				m_pf_layer[0][0]->draw(screen, bitmap, cliprect, 0,0);
				m_pf_layer[0][0]->draw(screen, bitmap, cliprect, 1,1);
			}
		}
	}

	draw_sprites(screen,bitmap,cliprect);

	return 0;
}

uint32_t m90_state::screen_update_bomblord(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	/* Setup scrolling */
	if (m_video_control_data[6]&0x20) {
		m_pf_layer[0][0]->set_scroll_rows(512);
		m_pf_layer[0][1]->set_scroll_rows(512);
		for (i=std::min(cliprect.min_y,511); i<=std::min(cliprect.max_y,511); i++) {
			m_pf_layer[0][0]->set_scrollx(i, m_video_data[0xf400/2+i]-12);
			m_pf_layer[0][1]->set_scrollx(i, m_video_data[0xf400/2+i]-12+256);
		}
	} else {
		m_pf_layer[0][0]->set_scroll_rows(1);
		m_pf_layer[0][1]->set_scroll_rows(1);
		m_pf_layer[0][0]->set_scrollx(0, m_video_data[0xf004/2]-12);
		m_pf_layer[0][1]->set_scrollx(0, m_video_data[0xf004/2]-12);
	}

	if (m_video_control_data[6] & 0x02) {
		m_pf_layer[1][1]->mark_all_dirty();
		m_pf_layer[1][1]->set_scrollx(0, m_video_data[0xf000/2]-16 );
		m_pf_layer[1][1]->set_scrolly(0, m_video_data[0xf008/2]+388);
		m_pf_layer[1][1]->draw(screen, bitmap, cliprect, 0,0);
		m_pf_layer[1][1]->draw(screen, bitmap, cliprect, 1,1);
	} else {
		m_pf_layer[1][0]->mark_all_dirty();
		m_pf_layer[1][0]->set_scrollx(0, m_video_data[0xf000/2]-16 );
		m_pf_layer[1][0]->set_scrolly(0, m_video_data[0xf008/2]-120);
		m_pf_layer[1][0]->draw(screen, bitmap, cliprect, 0,0);
		m_pf_layer[1][0]->draw(screen, bitmap, cliprect, 1,1);
	}

	if (m_video_control_data[6] & 0x04) {
		m_pf_layer[0][1]->mark_all_dirty();
		m_pf_layer[0][1]->set_scrolly(0, m_video_data[0xf00c/2]+392);
		m_pf_layer[0][1]->draw(screen, bitmap, cliprect, 0,0);
		m_pf_layer[0][1]->draw(screen, bitmap, cliprect, 1,1);
	} else {
		m_pf_layer[0][0]->mark_all_dirty();
		m_pf_layer[0][0]->set_scrolly(0, m_video_data[0xf00c/2]-116);
		m_pf_layer[0][0]->draw(screen, bitmap, cliprect, 0,0);
		m_pf_layer[0][0]->draw(screen, bitmap, cliprect, 1,1);
	}

	bomblord_draw_sprites(screen,bitmap,cliprect);

	return 0;
}

uint32_t m90_state::screen_update_dynablsb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (!(m_video_data[0xf008/2] & 0x4000)) {
		m_pf_layer[0][1]->mark_all_dirty();
		m_pf_layer[0][1]->set_scroll_rows(1);
		m_pf_layer[0][1]->set_scrollx(0, m_video_data[0xf004/2]+64);
		m_pf_layer[0][1]->set_scrolly(0, m_video_data[0xf006/2]+512);
		m_pf_layer[0][1]->draw(screen, bitmap, cliprect, 0,0);
		m_pf_layer[0][1]->draw(screen, bitmap, cliprect, 1,1);
	} else {
		m_pf_layer[0][0]->mark_all_dirty();
		m_pf_layer[0][0]->set_scroll_rows(1);
		m_pf_layer[0][0]->set_scrollx(0, m_video_data[0xf004/2]+64);
		m_pf_layer[0][0]->set_scrolly(0, m_video_data[0xf006/2]+4);
		m_pf_layer[0][0]->draw(screen, bitmap, cliprect, 0,0);
		m_pf_layer[0][0]->draw(screen, bitmap, cliprect, 1,1);
	}

	if (!(m_video_data[0xf008/2] & 0x8000)) {
		m_pf_layer[1][1]->mark_all_dirty();
		m_pf_layer[1][1]->set_scroll_rows(1);
		m_pf_layer[1][1]->set_scrollx(0, m_video_data[0xf000/2]+68);
		m_pf_layer[1][1]->set_scrolly(0, m_video_data[0xf002/2]+512);
		m_pf_layer[1][1]->draw(screen, bitmap, cliprect, 0,0);
		m_pf_layer[1][1]->draw(screen, bitmap, cliprect, 1,1);
	} else {
		m_pf_layer[1][0]->mark_all_dirty();
		m_pf_layer[1][0]->set_scroll_rows(1);
		m_pf_layer[1][0]->set_scrollx(0, m_video_data[0xf000/2]+68);
		m_pf_layer[1][0]->set_scrolly(0, m_video_data[0xf002/2]+4);
		m_pf_layer[1][0]->draw(screen, bitmap, cliprect, 0,0);
		m_pf_layer[1][0]->draw(screen, bitmap, cliprect, 1,1);
	}

	dynablsb_draw_sprites(screen,bitmap,cliprect);

	return 0;
}
