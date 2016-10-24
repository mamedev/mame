// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
/************************************************************************

    Quiz Panicuru Fantasy video hardware

************************************************************************/

#include "emu.h"
#include "includes/quizpani.h"


tilemap_memory_index quizpani_state::bg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0xff) << 4) + ((row & 0x70) << 8);
}

void quizpani_state::bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_bg_videoram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			(code & 0xfff) + (0x1000 * m_bgbank),
			code >> 12,
			0);
}

void quizpani_state::txt_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_txt_videoram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code & 0xfff) + (0x1000 * m_txtbank),
			code >> 12,
			0);
}

void quizpani_state::bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void quizpani_state::txt_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_txt_videoram[offset] = data;
	m_txt_tilemap->mark_tile_dirty(offset);
}

void quizpani_state::tilesbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if(m_txtbank != (data & 0x30)>>4)
		{
			m_txtbank = (data & 0x30)>>4;
			m_txt_tilemap->mark_all_dirty();
		}

		if(m_bgbank != (data & 3))
		{
			m_bgbank = data & 3;
			m_bg_tilemap->mark_all_dirty();
		}
	}
}

void quizpani_state::video_start()
{
	m_bg_tilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(quizpani_state::bg_tile_info),this), tilemap_mapper_delegate(FUNC(quizpani_state::bg_scan),this),16,16,256,32);
	m_txt_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(quizpani_state::txt_tile_info),this),tilemap_mapper_delegate(FUNC(quizpani_state::bg_scan),this),16,16,256,32);
	m_txt_tilemap->set_transparent_pen(15);

	save_item(NAME(m_bgbank));
	save_item(NAME(m_txtbank));
}

uint32_t quizpani_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollreg[0] - 64);
	m_bg_tilemap->set_scrolly(0, m_scrollreg[1] + 16);
	m_txt_tilemap->set_scrollx(0, m_scrollreg[2] - 64);
	m_txt_tilemap->set_scrolly(0, m_scrollreg[3] + 16);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_txt_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
