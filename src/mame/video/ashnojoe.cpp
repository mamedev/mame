// license:BSD-3-Clause
// copyright-holders:David Haywood
/********************************************************************

    Ashita no Joe (Success Joe) [Wave]
    video hardware emulation

*********************************************************************/

#include "emu.h"
#include "includes/ashnojoe.h"


TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_highest)
{
	int code = m_tileram[0][tile_index];

	SET_TILE_INFO_MEMBER(2,
			code & 0xfff,
			((code >> 12) & 0x0f),
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_midlow)
{
	int code = m_tileram[1][tile_index * 2];
	int attr = m_tileram[1][tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x40,
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_high)
{
	int code = m_tileram[2][tile_index];

	SET_TILE_INFO_MEMBER(0,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x10,
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_low)
{
	int code = m_tileram[3][tile_index];

	SET_TILE_INFO_MEMBER(1,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x60,
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_midhigh)
{
	int code = m_tileram[4][tile_index * 2];
	int attr = m_tileram[4][tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x20,
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_lowest)
{
	const int buffer = (m_tilemap_reg[0] & 0x02) >> 1;
	int code = m_tileram[5 + buffer][tile_index * 2];
	int attr = m_tileram[5 + buffer][tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}


void ashnojoe_state::tilemaps_xscroll_w(offs_t offset, u16 data)
{
	switch (offset)
	{
	case 0:
		m_tilemap[2]->set_scrollx(0, data);
		break;
	case 1:
		m_tilemap[4]->set_scrollx(0, data);
		break;
	case 2:
		m_tilemap[1]->set_scrollx(0, data);
		break;
	case 3:
		m_tilemap[3]->set_scrollx(0, data);
		break;
	case 4:
		m_tilemap[5]->set_scrollx(0, data);
		break;
	}
}

void ashnojoe_state::tilemaps_yscroll_w(offs_t offset, u16 data)
{
	switch (offset)
	{
	case 0:
		m_tilemap[2]->set_scrolly(0, data);
		break;
	case 1:
		m_tilemap[4]->set_scrolly(0, data);
		break;
	case 2:
		m_tilemap[1]->set_scrolly(0, data);
		break;
	case 3:
		m_tilemap[3]->set_scrolly(0, data);
		break;
	case 4:
		m_tilemap[5]->set_scrolly(0, data);
		break;
	}
}

void ashnojoe_state::tilemap_regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old = m_tilemap_reg[offset];
	data = COMBINE_DATA(&m_tilemap_reg[offset]);
	if (old != data)
	{
		if (offset == 0)
		{
			if ((old ^ data) & 0x02)
				m_tilemap[5]->mark_all_dirty();
		}
	}
}

void ashnojoe_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_tile_info_highest),this), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_tile_info_midlow),this),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_tile_info_high),this),    TILEMAP_SCAN_ROWS,  8,  8, 64, 64);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_tile_info_low),this),     TILEMAP_SCAN_ROWS,  8,  8, 64, 64);
	m_tilemap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_tile_info_midhigh),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[5] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ashnojoe_state::get_tile_info_lowest),this),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tilemap[0]->set_transparent_pen(15);
	m_tilemap[1]->set_transparent_pen(15);
	m_tilemap[2]->set_transparent_pen(15);
	m_tilemap[3]->set_transparent_pen(15);
	m_tilemap[4]->set_transparent_pen(15);
}

u32 ashnojoe_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//m_tilemap_reg[0] & 0x10 // ?? on coin insertion

	flip_screen_set(m_tilemap_reg[0] & 1);

	m_tilemap[5]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[4]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
