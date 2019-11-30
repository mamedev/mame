// license:BSD-3-Clause
// copyright-holders:David Graves, R. Belmont, David Haywood
#include "emu.h"
#include "includes/gcpinbal.h"
#include "screen.h"


/*******************************************************************/


TILE_GET_INFO_MEMBER(gcpinbal_state::get_bg0_tile_info)
{
	const u16 tile = m_tilemapram[0 + tile_index * 2];
	const u16 attr = m_tilemapram[1 + tile_index * 2];

	SET_TILE_INFO_MEMBER(0,
			(tile & 0xfff) + m_bg0_gfxset,
			(attr & 0x1f),
			TILE_FLIPYX((attr & 0x300) >> 8));
}

TILE_GET_INFO_MEMBER(gcpinbal_state::get_bg1_tile_info)
{
	const u16 tile = m_tilemapram[0x800 + tile_index * 2];
	const u16 attr = m_tilemapram[0x801 + tile_index * 2];

	SET_TILE_INFO_MEMBER(0,
			(tile & 0xfff) + 0x2000 + m_bg1_gfxset,
			(attr & 0x1f) + 0x30,
			TILE_FLIPYX((attr & 0x300) >> 8));
}

TILE_GET_INFO_MEMBER(gcpinbal_state::get_fg_tile_info)
{
	const u16 tile = m_tilemapram[0x1000 + tile_index];
	SET_TILE_INFO_MEMBER(1, (tile & 0xfff), (tile >> 12), 0);
}

void gcpinbal_state::video_start()
{
	int xoffs = 0;
	int yoffs = 0;

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gcpinbal_state::get_bg0_tile_info)),TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gcpinbal_state::get_bg1_tile_info)),TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gcpinbal_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 64, 64);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);

	/* flipscreen n/a */
	m_tilemap[0]->set_scrolldx(-xoffs, 0);
	m_tilemap[1]->set_scrolldx(-xoffs, 0);
	m_tilemap[2]->set_scrolldx(-xoffs, 0);
	m_tilemap[0]->set_scrolldy(-yoffs, 0);
	m_tilemap[1]->set_scrolldy(-yoffs, 0);
	m_tilemap[2]->set_scrolldy(-yoffs, 0);
}

void gcpinbal_state::gcpinbal_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = (m_d80060_ram[0x8 / 2] & 0x8800) ? 0xf0 : 0xfc;
}


/******************************************************************
                   TILEMAP READ AND WRITE HANDLERS
*******************************************************************/

void gcpinbal_state::tilemaps_word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tilemapram[offset]);

	if (offset < 0x800) /* BG0 */
		m_tilemap[0]->mark_tile_dirty(offset / 2);
	else if ((offset < 0x1000)) /* BG1 */
		m_tilemap[1]->mark_tile_dirty((offset % 0x800) / 2);
	else if ((offset < 0x1800)) /* FG */
		m_tilemap[2]->mark_tile_dirty((offset % 0x800));
}


/**************************************************************
                        SCREEN REFRESH
**************************************************************/

uint32_t gcpinbal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t layer[3];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once(KEYCODE_V))
	{
		m_dislayer[0] ^= 1;
		popmessage("bg0: %01x", m_dislayer[0]);
	}

	if (machine().input().code_pressed_once(KEYCODE_B))
	{
		m_dislayer[1] ^= 1;
		popmessage("bg1: %01x", m_dislayer[1]);
	}

	if (machine().input().code_pressed_once(KEYCODE_N))
	{
		m_dislayer[2] ^= 1;
		popmessage("fg: %01x", m_dislayer[2]);
	}
#endif

	m_scrollx[0] = m_d80010_ram[0x4 / 2];
	m_scrolly[0] = m_d80010_ram[0x6 / 2];
	m_scrollx[1] = m_d80010_ram[0x8 / 2];
	m_scrolly[1] = m_d80010_ram[0xa / 2];
	m_scrollx[2] = m_d80010_ram[0xc / 2];
	m_scrolly[2] = m_d80010_ram[0xe / 2];

	for (int i = 0; i < 3; i++)
	{
		m_tilemap[i]->set_scrollx(0, m_scrollx[i]);
		m_tilemap[i]->set_scrolly(0, m_scrolly[i]);
	}

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;


#ifdef MAME_DEBUG
	if (m_dislayer[layer[0]] == 0)
#endif
	m_tilemap[layer[0]]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[1]] == 0)
#endif
	m_tilemap[layer[1]]->draw(screen, bitmap, cliprect, 0, 2);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[2]] == 0)
#endif
	m_tilemap[layer[2]]->draw(screen, bitmap, cliprect, 0, 4);

	m_sprgen->gcpinbal_draw_sprites(screen, bitmap, cliprect, 16);

#if 0
	{
//      char buf[80];
		sprintf(buf,"bg0_gfx: %04x bg1_gfx: %04x ", m_bg0_gfxset, m_bg1_gfxset);
		popmessage(buf);
	}
#endif

	return 0;
}
