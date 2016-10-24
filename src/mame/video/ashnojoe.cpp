// license:BSD-3-Clause
// copyright-holders:David Haywood
/********************************************************************

    Ashita no Joe (Success Joe) [Wave]
    video hardware emulation

*********************************************************************/

#include "emu.h"
#include "includes/ashnojoe.h"


void ashnojoe_state::get_joe_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_tileram[tile_index];

	SET_TILE_INFO_MEMBER(2,
			code & 0xfff,
			((code >> 12) & 0x0f),
			0);
}

void ashnojoe_state::get_joe_tile_info_2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_tileram_2[tile_index * 2];
	int attr = m_tileram_2[tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x40,
			0);
}

void ashnojoe_state::get_joe_tile_info_3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_tileram_3[tile_index];

	SET_TILE_INFO_MEMBER(0,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x10,
			0);
}

void ashnojoe_state::get_joe_tile_info_4(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_tileram_4[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x60,
			0);
}

void ashnojoe_state::get_joe_tile_info_5(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_tileram_5[tile_index * 2];
	int attr = m_tileram_5[tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x20,
			0);
}

void ashnojoe_state::get_joe_tile_info_6(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_tileram_6[tile_index * 2];
	int attr = m_tileram_6[tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}


void ashnojoe_state::get_joe_tile_info_7(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int code = m_tileram_7[tile_index * 2];
	int attr = m_tileram_7[tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}

void ashnojoe_state::ashnojoe_tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_tileram[offset] = data;
	m_joetilemap->mark_tile_dirty(offset);
}


void ashnojoe_state::ashnojoe_tileram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_tileram_2[offset] = data;
	m_joetilemap2->mark_tile_dirty(offset / 2);
}

void ashnojoe_state::ashnojoe_tileram3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_tileram_3[offset] = data;
	m_joetilemap3->mark_tile_dirty(offset);
}

void ashnojoe_state::ashnojoe_tileram4_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_tileram_4[offset] = data;
	m_joetilemap4->mark_tile_dirty(offset);
}

void ashnojoe_state::ashnojoe_tileram5_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_tileram_5[offset] = data;
	m_joetilemap5->mark_tile_dirty(offset / 2);
}

void ashnojoe_state::ashnojoe_tileram6_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_tileram_6[offset] = data;
	m_joetilemap6->mark_tile_dirty(offset / 2);
}

void ashnojoe_state::ashnojoe_tileram7_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_tileram_7[offset] = data;
	m_joetilemap7->mark_tile_dirty(offset / 2);
}

void ashnojoe_state::joe_tilemaps_xscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch( offset )
	{
	case 0:
		m_joetilemap3->set_scrollx(0, data);
		break;
	case 1:
		m_joetilemap5->set_scrollx(0, data);
		break;
	case 2:
		m_joetilemap2->set_scrollx(0, data);
		break;
	case 3:
		m_joetilemap4->set_scrollx(0, data);
		break;
	case 4:
		m_joetilemap6->set_scrollx(0, data);
		m_joetilemap7->set_scrollx(0, data);
		break;
	}
}

void ashnojoe_state::joe_tilemaps_yscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch( offset )
	{
	case 0:
		m_joetilemap3->set_scrolly(0, data);
		break;
	case 1:
		m_joetilemap5->set_scrolly(0, data);
		break;
	case 2:
		m_joetilemap2->set_scrolly(0, data);
		break;
	case 3:
		m_joetilemap4->set_scrolly(0, data);
		break;
	case 4:
		m_joetilemap6->set_scrolly(0, data);
		m_joetilemap7->set_scrolly(0, data);
		break;
	}
}

void ashnojoe_state::video_start()
{
	m_joetilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_joe_tile_info),this),  TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_joetilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_joe_tile_info_2),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_joetilemap3 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_joe_tile_info_3),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_joetilemap4 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_joe_tile_info_4),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_joetilemap5 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_joe_tile_info_5),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_joetilemap6 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_joe_tile_info_6),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_joetilemap7 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_joe_tile_info_7),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_joetilemap->set_transparent_pen(15);
	m_joetilemap2->set_transparent_pen(15);
	m_joetilemap3->set_transparent_pen(15);
	m_joetilemap4->set_transparent_pen(15);
	m_joetilemap5->set_transparent_pen(15);
}

uint32_t ashnojoe_state::screen_update_ashnojoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//m_tilemap_reg[0] & 0x10 // ?? on coin insertion

	flip_screen_set(m_tilemap_reg[0] & 1);

	if(m_tilemap_reg[0] & 0x02)
		m_joetilemap7->draw(screen, bitmap, cliprect, 0, 0);
	else
		m_joetilemap6->draw(screen, bitmap, cliprect, 0, 0);

	m_joetilemap4->draw(screen, bitmap, cliprect, 0, 0);
	m_joetilemap2->draw(screen, bitmap, cliprect, 0, 0);
	m_joetilemap5->draw(screen, bitmap, cliprect, 0, 0);
	m_joetilemap3->draw(screen, bitmap, cliprect, 0, 0);
	m_joetilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
