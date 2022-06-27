// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "spbactn.h"



void spbactn_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset&0x1fff);
}

TILE_GET_INFO_MEMBER(spbactn_state::get_bg_tile_info)
{
	int attr = m_bgvideoram[tile_index];
	int tileno = m_bgvideoram[tile_index+0x2000];
	tileinfo.set(1, tileno, ((attr & 0x00f0)>>4), 0);
}


void spbactn_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset&0x1fff);
}

TILE_GET_INFO_MEMBER(spbactn_state::get_fg_tile_info)
{
	int attr = m_fgvideoram[tile_index];
	int tileno = m_fgvideoram[tile_index+0x2000];

	int color = ((attr & 0x00f0)>>4);

	/* blending */
	if (attr & 0x0008)
		color += 0x0010;

	tileinfo.set(0, tileno, color, 0);
}



VIDEO_START_MEMBER(spbactn_state,spbactn)
{
	/* allocate bitmaps */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spbactn_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 64, 128);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spbactn_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 64, 128);
	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(spbactn_state,spbactnp)
{
	VIDEO_START_CALL_MEMBER(spbactn);
	// no idea..
	m_extra_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spbactn_state::get_extra_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 16, 16);
}

void spbactn_state::spbatnp_90006_w(uint16_t data)
{
	//printf("spbatnp_90006_w %04x\n",data);
}


void spbactn_state::spbatnp_9000c_w(uint16_t data)
{
	//printf("spbatnp_9000c_w %04x\n",data);
}

void spbactn_state::spbatnp_9000e_w(uint16_t data)
{
	//printf("spbatnp_9000e_w %04x\n",data);
}

void spbactn_state::spbatnp_9000a_w(uint16_t data)
{
	//printf("spbatnp_9000a_w %04x\n",data);
}

void spbactn_state::spbatnp_90124_w(uint16_t data)
{
	//printf("spbatnp_90124_w %04x\n",data);
	m_bg_tilemap->set_scrolly(0, data);

}

void spbactn_state::spbatnp_9012c_w(uint16_t data)
{
	//printf("spbatnp_9012c_w %04x\n",data);
	m_bg_tilemap->set_scrollx(0, data);
}


void spbactn_state::extraram_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_extraram[offset]);
	m_extra_tilemap->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(spbactn_state::get_extra_tile_info)
{
	int tileno = m_extraram[(tile_index*2)+1];
	tileno |= m_extraram[(tile_index*2)] << 8;
	tileinfo.set(3, tileno, 0, 0);
}




int spbactn_state::draw_video(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool alt_sprites)
{
	m_tile_bitmap_bg.fill(0, cliprect);
	m_tile_bitmap_fg.fill(0, cliprect);
	m_sprite_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_sprgen->gaiden_draw_sprites(screen, m_gfxdecode->gfx(2), cliprect, &m_spvideoram[0], 0, 0, flip_screen(), m_sprite_bitmap);
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
