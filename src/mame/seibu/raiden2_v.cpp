// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
/********************************************************************************************************

    Seibu Protected 1993-94 era hardware, V30 based (sequel to the SYS68C hardware)

********************************************************************************************************/

#include "emu.h"
#include "raiden2.h"

void raiden2_state::m_videoram_private_w(offs_t offset, uint16_t data)
{
	//  map(0x0d000, 0x0d7ff).ram().w(FUNC(raiden2_state::background_w)).share("back_data");
	//  map(0x0d800, 0x0dfff).ram().w(FUNC(raiden2_state::foreground_w).share("fore_data");
	//  map(0x0e000, 0x0e7ff).ram().w(FUNC(raiden2_state::midground_w).share("mid_data");
	//  map(0x0e800, 0x0f7ff).ram().w(FUNC(raiden2_state::text_w).share("text_data");

	if (offset < 0x800 / 2)
	{
		background_w(offset, data);
	}
	else if (offset < 0x1000 /2)
	{
		offset -= 0x800 / 2;
		foreground_w(offset, data);
	}
	else if (offset < 0x1800/2)
	{
		offset -= 0x1000 / 2;
		midground_w(offset, data);
	}
	else if (offset < 0x2800/2)
	{
		offset -= 0x1800 / 2;
		text_w(offset, data);
	}
}


void raiden2_state::background_w(offs_t offset, u16 data)
{
	if (m_back_data[offset] != data)
	{
		m_back_data[offset] = data;
		m_background_layer->mark_tile_dirty(offset);
	}
}

void raiden2_state::midground_w(offs_t offset, u16 data)
{
	if (m_mid_data[offset] != data)
	{
		m_mid_data[offset] = data;
		m_midground_layer->mark_tile_dirty(offset);
	}
}

void raiden2_state::foreground_w(offs_t offset, u16 data)
{
	if (m_fore_data[offset] != data)
	{
		m_fore_data[offset] = data;
		m_foreground_layer->mark_tile_dirty(offset);
	}
}

void raiden2_state::text_w(offs_t offset, u16 data)
{
	if (m_text_data[offset] != data)
	{
		m_text_data[offset] = data;
		m_text_layer->mark_tile_dirty(offset);
	}
}

void raiden2_state::tilemap_enable_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tilemap_enable);
}

void raiden2_state::tile_scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	tilemap_t *tm = nullptr;
	switch (offset/2)
	{
	case 0: tm = m_background_layer; break;
	case 1: tm = m_midground_layer; break;
	case 2: tm = m_foreground_layer; break;
	default: assert(0); break;
	}

	COMBINE_DATA(&m_scrollvals[offset]);
	data = m_scrollvals[offset];

	if (offset & 1)
		tm->set_scrolly(0, data);
	else
		tm->set_scrollx(0, data);
}

void raiden2_state::tile_bank_01_w(u8 data)
{
	int new_bank;
	new_bank = 0 | ((data & 1) << 1);
	if (new_bank != m_bg_bank)
	{
		m_bg_bank = new_bank;
		m_background_layer->mark_all_dirty();
	}

	new_bank = 1 | (data & 2);
	if (new_bank != m_mid_bank)
	{
		m_mid_bank = new_bank;
		m_midground_layer->mark_all_dirty();
	}
}

u16 raiden2_state::cop_tile_bank_2_r()
{
	return m_cop_bank;
}

void raiden2_state::cop_tile_bank_2_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_cop_bank);

	if (ACCESSING_BITS_8_15)
	{
		const int new_bank = 4 | (data >> 14);
		if (new_bank != m_fg_bank)
		{
			m_fg_bank = new_bank;
			m_foreground_layer->mark_all_dirty();
		}
	}
}

void raiden2_state::raidendx_cop_bank_2_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_cop_bank);

	const int new_bank = 4 | ((m_cop_bank >> 4) & 3);
	if (new_bank != m_fg_bank)
	{
		m_fg_bank = new_bank;
		m_foreground_layer->mark_all_dirty();
	}

	// m_mainbank[1] coming from 6c9 ?
	const int bb = m_cop_bank >> 12;
	m_mainbank->set_entry(bb);
}


TILE_GET_INFO_MEMBER(raiden2_state::get_back_tile_info)
{
	int tile = m_back_data[tile_index];
	int color = (tile >> 12) | (0 << 4);

	tile = (tile & 0xfff) | (m_bg_bank << 12);

	tileinfo.set(1,tile+0x0000,color,0);
}

TILE_GET_INFO_MEMBER(raiden2_state::get_mid_tile_info)
{
	int tile = m_mid_data[tile_index];
	int color = (tile >> 12) | (2 << 4);

	tile = (tile & 0xfff) | (m_mid_bank << 12);

	tileinfo.set(1,tile,color,0);
}

TILE_GET_INFO_MEMBER(raiden2_state::get_fore_tile_info)
{
	int tile = m_fore_data[tile_index];
	int color = (tile >> 12) | (1 << 4);

	tile = (tile & 0xfff) | (m_fg_bank << 12);

	tileinfo.set(1,tile,color,0);
}

TILE_GET_INFO_MEMBER(raiden2_state::get_text_tile_info)
{
	int tile = m_text_data[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	tileinfo.set(0,tile + m_tx_bank * 0x1000,color,0);
}

void raiden2_state::video_start()
{
	m_screen->register_screen_bitmap(m_tile_bitmap);
	m_spritegen->alloc_sprite_bitmap();

	m_back_data = make_unique_clear<u16[]>(0x800/2);
	m_fore_data = make_unique_clear<u16[]>(0x800/2);
	m_mid_data = make_unique_clear<u16[]>(0x800/2);
	m_text_data = make_unique_clear<u16[]>(0x1000/2);
	m_palette_data = make_unique_clear<u16[]>(0x1000/2);
	m_palette->basemem().set(m_palette_data.get(), 0x1000/2 * sizeof(u16), 16, ENDIANNESS_LITTLE, 2);

	save_pointer(NAME(m_back_data), 0x800/2);
	save_pointer(NAME(m_fore_data), 0x800/2);
	save_pointer(NAME(m_mid_data), 0x800/2);
	save_pointer(NAME(m_text_data), 0x1000/2);
	save_pointer(NAME(m_palette_data), 0x1000/2);

	m_text_layer       = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden2_state::get_text_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 64,32 );
	m_background_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden2_state::get_back_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 32,32 );
	m_midground_layer  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden2_state::get_mid_tile_info)),  TILEMAP_SCAN_ROWS, 16,16, 32,32 );
	m_foreground_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiden2_state::get_fore_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 32,32 );
}

void raiden2_state::blend_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind16 &source, int layer)
{
	if (layer == -1)
		return;

	pen_t const *const pens = &m_palette->pen(0);
	layer <<= 14;
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u16 *src = &source.pix(y, cliprect.min_x);
		u32 *dst = &bitmap.pix(y, cliprect.min_x);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u16 val = *src++;
			if ((val & 0xc000) == layer && (val & 0x000f) != 0x000f)
			{
				val &= 0x07ff;

				if (m_blend_active[val])
					*dst = alpha_blend_r32(*dst, pens[val], 0x7f);
				else
					*dst = pens[val];
			}
			dst++;
		}
	}
}

void raiden2_state::tilemap_draw_and_blend(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tilemap)
{
	tilemap->draw(screen, m_tile_bitmap, cliprect, 0, 0);
	blend_layer(bitmap, cliprect, m_tile_bitmap, 0);
}

u32 raiden2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	if (!(m_tilemap_enable & 16))
	{
		m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram->buffer(), m_spriteram->bytes());

		blend_layer(bitmap, cliprect, m_spritegen->get_sprite_temp_bitmap(), m_cur_spri[0]);
	}

	if (!(m_tilemap_enable & 1))
		tilemap_draw_and_blend(screen, bitmap, cliprect, m_background_layer);

	if (!(m_tilemap_enable & 16))
		blend_layer(bitmap, cliprect, m_spritegen->get_sprite_temp_bitmap(), m_cur_spri[1]);

	if (!(m_tilemap_enable & 2))
		tilemap_draw_and_blend(screen, bitmap, cliprect, m_midground_layer);

	if (!(m_tilemap_enable & 16))
		blend_layer(bitmap, cliprect, m_spritegen->get_sprite_temp_bitmap(), m_cur_spri[2]);

	if (!(m_tilemap_enable & 4))
		tilemap_draw_and_blend(screen, bitmap, cliprect, m_foreground_layer);

	if (!(m_tilemap_enable & 16))
		blend_layer(bitmap, cliprect, m_spritegen->get_sprite_temp_bitmap(), m_cur_spri[3]);

	if (!(m_tilemap_enable & 8))
		tilemap_draw_and_blend(screen, bitmap, cliprect, m_text_layer);

	if (!(m_tilemap_enable & 16))
		blend_layer(bitmap, cliprect, m_spritegen->get_sprite_temp_bitmap(), m_cur_spri[4]);

	if (machine().input().code_pressed_once(KEYCODE_Z))
		if (m_raiden2cop) m_raiden2cop->dump_table();

	return 0;
}


