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


WRITE16_MEMBER(crospang_state::bestri_tilebank_select_w)
{
	logerror("bestri_tilebank_select_w %04x\n", data & mem_mask);

	m_bestri_tilebankselect = (data >> 8) & 3;
}


WRITE16_MEMBER(crospang_state::bestri_tilebank_data_w)
{
	logerror("bestri_tilebank_data_w %04x\n", data & mem_mask);

	m_bestri_tilebank[m_bestri_tilebankselect] = data >> 8;

	m_fg_layer->mark_all_dirty();
	m_bg_layer->mark_all_dirty();
}


// Bestri performs some unusual operations on the scroll values before writing them
WRITE16_MEMBER(crospang_state::bestri_bg_scrolly_w)
{
	// addi.w #$1f8, D0
	// eori.w #$154, D0
	int scroll =  (data & 0x3ff) ^ 0x0155;
	m_bg_layer->set_scrolly(0, -scroll + 7);
}

WRITE16_MEMBER(crospang_state::bestri_fg_scrolly_w)
{
	// addi.w #$1f8, D0
	// eori.w #$aa, D0
	int scroll = (data & 0x3ff) ^ 0x00ab;
	m_fg_layer->set_scrolly(0, -scroll + 7);
}

WRITE16_MEMBER(crospang_state::bestri_fg_scrollx_w)
{
	// addi.w #$400, D1
	// eori.w #$1e0, D1
	int scroll =  (data & 0x3ff) ^ 0x1e1;
	m_fg_layer->set_scrollx(0, scroll - 1);
}

WRITE16_MEMBER(crospang_state::bestri_bg_scrollx_w)
{
	// addi.w #$3fc, D1
	// eori.w #$3c0, D1
	int scroll =  (data & 0x3ff) ^ 0x3c1;
	m_bg_layer->set_scrollx(0, scroll + 3);
}


WRITE16_MEMBER(crospang_state::crospang_fg_scrolly_w)
{
	m_fg_layer->set_scrolly(0, data + 8);
}

WRITE16_MEMBER(crospang_state::crospang_bg_scrolly_w)
{
	m_bg_layer->set_scrolly(0, data + 8);
}

WRITE16_MEMBER(crospang_state::crospang_fg_scrollx_w)
{
	m_fg_layer->set_scrollx(0, data);
}

WRITE16_MEMBER(crospang_state::crospang_bg_scrollx_w)
{
	m_bg_layer->set_scrollx(0, data + 4);
}


WRITE16_MEMBER(crospang_state::crospang_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(crospang_state::crospang_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(crospang_state::get_bg_tile_info)
{
	int data  = m_bg_videoram[tile_index];
	int tile  = data & 0x03ff;
	int tilebank = (data & 0x0c00) >> 10;
	tile = tile + (m_bestri_tilebank[tilebank] << 10);
	int color = (data >> 12) & 0x0f;

	SET_TILE_INFO_MEMBER(1, tile, color + 0x20, 0);
}

TILE_GET_INFO_MEMBER(crospang_state::get_fg_tile_info)
{
	int data  = m_fg_videoram[tile_index];
	int tile  = data & 0x03ff;
	int tilebank = (data & 0x0c00) >> 10;
	tile = tile + (m_bestri_tilebank[tilebank] << 10);
	int color = (data >> 12) & 0x0f;

	SET_TILE_INFO_MEMBER(1, tile, color + 0x10, 0);
}


void crospang_state::video_start()
{
	m_bg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(crospang_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(crospang_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_fg_layer->set_transparent_pen(0);
}

uint32_t crospang_state::screen_update_crospang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_layer->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_layer->draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}
