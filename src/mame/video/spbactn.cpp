// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "includes/spbactn.h"



void spbactn_state::bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset&0x1fff);
}

void spbactn_state::get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int attr = m_bgvideoram[tile_index];
	int tileno = m_bgvideoram[tile_index+0x2000];
	SET_TILE_INFO_MEMBER(1, tileno, ((attr & 0x00f0)>>4), 0);
}


void spbactn_state::fg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset&0x1fff);
}

void spbactn_state::get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int attr = m_fgvideoram[tile_index];
	int tileno = m_fgvideoram[tile_index+0x2000];

	int color = ((attr & 0x00f0)>>4);

	/* blending */
	if (attr & 0x0008)
		color += 0x0010;

	SET_TILE_INFO_MEMBER(0, tileno, color, 0);
}



void spbactn_state::video_start_spbactn()
{
	/* allocate bitmaps */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(spbactn_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 8, 64, 128);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(spbactn_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 8, 64, 128);
	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

}

void spbactn_state::video_start_spbactnp()
{
	video_start_spbactn();
	// no idea..
	m_extra_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(spbactn_state::get_extra_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 16, 16);
}
void spbactn_state::spbatnp_90002_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("spbatnp_90002_w %04x\n",data);
}

void spbactn_state::spbatnp_90006_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("spbatnp_90006_w %04x\n",data);
}


void spbactn_state::spbatnp_9000c_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("spbatnp_9000c_w %04x\n",data);
}

void spbactn_state::spbatnp_9000e_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("spbatnp_9000e_w %04x\n",data);
}

void spbactn_state::spbatnp_9000a_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("spbatnp_9000a_w %04x\n",data);
}

void spbactn_state::spbatnp_90124_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("spbatnp_90124_w %04x\n",data);
	m_bg_tilemap->set_scrolly(0, data);

}

void spbactn_state::spbatnp_9012c_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("spbatnp_9012c_w %04x\n",data);
	m_bg_tilemap->set_scrollx(0, data);
}


void spbactn_state::extraram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_extraram[offset]);
	m_extra_tilemap->mark_tile_dirty(offset/2);
}

void spbactn_state::get_extra_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tileno = m_extraram[(tile_index*2)+1];
	tileno |= m_extraram[(tile_index*2)] << 8;
	SET_TILE_INFO_MEMBER(3, tileno, 0, 0);
}




int spbactn_state::draw_video(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool alt_sprites)
{
	m_tile_bitmap_bg.fill(0, cliprect);
	m_tile_bitmap_fg.fill(0, cliprect);
	m_sprite_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_sprgen->gaiden_draw_sprites(screen, m_gfxdecode, cliprect, m_spvideoram, 0, 0, flip_screen(), m_sprite_bitmap);
	m_bg_tilemap->draw(screen, m_tile_bitmap_bg, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, m_tile_bitmap_fg, cliprect, 0, 0);

	m_mixer->mix_bitmaps(screen, bitmap, cliprect, *m_palette, &m_tile_bitmap_bg, &m_tile_bitmap_fg, (bitmap_ind16*)nullptr, &m_sprite_bitmap);

	return 0;
}

uint32_t spbactn_state::screen_update_spbactn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return draw_video(screen,bitmap,cliprect,false);
}

uint32_t spbactn_state::screen_update_spbactnp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// hack to make the extra cpu do something..
	m_extraram2[0x104] = machine().rand();
	m_extraram2[0x105] = machine().rand();

	return draw_video(screen,bitmap,cliprect,true);
}
