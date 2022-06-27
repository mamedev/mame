// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "gotcha.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(gotcha_state::tilemap_scan)
{
	return (col & 0x1f) | (row << 5) | ((col & 0x20) << 5);
}

inline void gotcha_state::get_tile_info( tile_data &tileinfo, int tile_index ,uint16_t *vram, int color_offs)
{
	uint16_t data = vram[tile_index];
	int code = (data & 0x3ff) | (m_gfxbank[(data & 0x0c00) >> 10] << 10);

	tileinfo.set(0, code, (data >> 12) + color_offs, 0);
}

TILE_GET_INFO_MEMBER(gotcha_state::fg_get_tile_info)
{
	get_tile_info(tileinfo, tile_index, m_fgvideoram, 0);
}

TILE_GET_INFO_MEMBER(gotcha_state::bg_get_tile_info)
{
	get_tile_info(tileinfo, tile_index, m_bgvideoram, 16);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void gotcha_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gotcha_state::fg_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(gotcha_state::tilemap_scan)), 16, 16, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gotcha_state::bg_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(gotcha_state::tilemap_scan)), 16, 16, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);

	m_fg_tilemap->set_scrolldx(-1, 0);
	m_bg_tilemap->set_scrolldx(-5, 0);
}


void gotcha_state::fgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

void gotcha_state::bgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gotcha_state::gfxbank_select_w(uint8_t data)
{
	m_banksel = data & 0x03;
}

void gotcha_state::gfxbank_w(uint8_t data)
{
	if (m_gfxbank[m_banksel] != (data & 0x0f))
	{
		m_gfxbank[m_banksel] = data & 0x0f;
		machine().tilemap().mark_all_dirty();
	}
}

void gotcha_state::scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_fg_tilemap->set_scrollx(0, m_scroll[0]); break;
		case 1: m_fg_tilemap->set_scrolly(0, m_scroll[1]); break;
		case 2: m_bg_tilemap->set_scrollx(0, m_scroll[2]); break;
		case 3: m_bg_tilemap->set_scrolly(0, m_scroll[3]); break;
	}
}


uint32_t gotcha_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}
