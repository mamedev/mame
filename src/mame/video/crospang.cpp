// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/*

  Cross Pang
  video hardware emulation

 -- this seems to be the same as the tumblepop bootleg based hardware
    in tumbleb.cpp


*/

#include "emu.h"
#include "includes/crospang.h"


void crospang_state::tilebank_select_w(u16 data)
{
	logerror("tilebank_select_w %04x\n", data);

	m_tilebankselect = (data >> 8) & 3;
}


void crospang_state::tilebank_data_w(u16 data)
{
	logerror("tilebank_data_w %04x\n", data);

	m_tilebank[m_tilebankselect] = data >> 8;

	m_fg_layer->mark_all_dirty();
	m_bg_layer->mark_all_dirty();
}


// Bestri performs some unusual operations on the scroll values before writing them
void crospang_state::bestri_bg_scrolly_w(u16 data)
{
	// addi.w #$1f8, D0
	// eori.w #$154, D0
	int scroll =  (data & 0x3ff) ^ 0x0155;
	m_bg_layer->set_scrolly(0, -scroll + 7);
}

void crospang_state::bestri_fg_scrolly_w(u16 data)
{
	// addi.w #$1f8, D0
	// eori.w #$aa, D0
	int scroll = (data & 0x3ff) ^ 0x00ab;
	m_fg_layer->set_scrolly(0, -scroll + 7);
}

void crospang_state::bestri_fg_scrollx_w(u16 data)
{
	// addi.w #$400, D1
	// eori.w #$1e0, D1
	int scroll =  (data & 0x3ff) ^ 0x1e1;
	m_fg_layer->set_scrollx(0, scroll - 1);
}

void crospang_state::bestri_bg_scrollx_w(u16 data)
{
	// addi.w #$3fc, D1
	// eori.w #$3c0, D1
	int scroll =  (data & 0x3ff) ^ 0x3c1;
	m_bg_layer->set_scrollx(0, scroll + 3);
}


void crospang_state::fg_scrolly_w(u16 data)
{
	m_fg_layer->set_scrolly(0, data + 8);
}

void crospang_state::bg_scrolly_w(u16 data)
{
	m_bg_layer->set_scrolly(0, data + 8);
}

void crospang_state::fg_scrollx_w(u16 data)
{
	m_fg_layer->set_scrollx(0, data);
}

void crospang_state::bg_scrollx_w(u16 data)
{
	m_bg_layer->set_scrollx(0, data + 4);
}


void crospang_state::fg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

void crospang_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(crospang_state::get_bg_tile_info)
{
	int data  = m_bg_videoram[tile_index];
	int tile  = data & 0x03ff;
	int tilebank = (data & 0x0c00) >> 10;
	tile = tile + (m_tilebank[tilebank] << 10);
	int color = (data >> 12) & 0x0f;

	SET_TILE_INFO_MEMBER(1, tile, color + 0x20, 0);
}

TILE_GET_INFO_MEMBER(crospang_state::get_fg_tile_info)
{
	int data  = m_fg_videoram[tile_index];
	int tile  = data & 0x03ff;
	int tilebank = (data & 0x0c00) >> 10;
	tile = tile + (m_tilebank[tilebank] << 10);
	int color = (data >> 12) & 0x0f;

	SET_TILE_INFO_MEMBER(1, tile, color + 0x10, 0);
}


void crospang_state::video_start()
{
	m_bg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(crospang_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(crospang_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_fg_layer->set_transparent_pen(0);
}

u32 crospang_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_layer->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_layer->draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}
