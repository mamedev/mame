// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Nicola Salmoria

/*
    CUS42 + CUS43 Tilemaps
    used by
    baraduke.cpp (all games)
    namcos86.cpp (all games)
*/

#include "emu.h"
#include "namco_cus4xtmap.h"

DEFINE_DEVICE_TYPE(NAMCO_CUS4XTMAP, namco_cus4xtmap_device, "namco_cus4xtmap", "Namco CUS42 + CUS43 (dual tilemaps)")

namco_cus4xtmap_device::namco_cus4xtmap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NAMCO_CUS4XTMAP, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_tile_cb(*this)
	, m_vram(*this, "videoram", 0x2000, ENDIANNESS_BIG)
	, m_tilemap{nullptr, nullptr}
	, m_xscroll{0, 0}
	, m_yscroll{0, 0}
	, m_xoffs(0)
	, m_yoffs(0)
	, m_flipped_xoffs(0)
	, m_flipped_yoffs(0)
{
}

void namco_cus4xtmap_device::device_start()
{
	m_tile_cb.resolve();

	// two scrolling tilemaps
	m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_cus4xtmap_device::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namco_cus4xtmap_device::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	// define offsets for scrolling
	static const int adj[2] = { 0,2 };
	for (int i = 0; i < 2; i++)
	{
		const int dx = m_xoffs + adj[i];
		m_tilemap[i]->set_scrolldx(dx, m_flipped_xoffs - dx);
		m_tilemap[i]->set_scrolldy(m_yoffs, m_flipped_yoffs);
		m_tilemap[i]->set_transparent_pen(7);
	}

	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
}

/**************************************************************************************/

void namco_cus4xtmap_device::mark_all_dirty(void)
{
	for (auto &tilemap : m_tilemap)
		tilemap->mark_all_dirty();
}

template <unsigned Layer>
TILE_GET_INFO_MEMBER(namco_cus4xtmap_device::get_tile_info)
{
	u16 const attr = m_vram[(Layer << 12) + (2 * tile_index + 1)];
	u8 gfxno = 0;
	u32 code = m_vram[(Layer << 12) + (2 * tile_index)] + ((attr & 0x03) << 8);
	if (!m_tile_cb.isnull())
		m_tile_cb(Layer, gfxno, code);
	tileinfo.set(gfxno, code, attr, 0);
}

void namco_cus4xtmap_device::init_scroll(bool flip)
{
	for (int i = 0; i < 2; i++)
	{
		int scrollx = m_xscroll[i];
		int scrolly = m_yscroll[i];
		if (flip)
		{
			scrollx = -scrollx;
			scrolly = -scrolly;
		}
		m_tilemap[i]->set_scrollx(0, scrollx);
		m_tilemap[i]->set_scrolly(0, scrolly);
	}
}

void namco_cus4xtmap_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 layer, u32 flags, u8 prival, u8 primask)
{
	m_tilemap[layer]->draw(screen, bitmap, cliprect, flags, prival, primask);
}

// 8 bit handlers
void namco_cus4xtmap_device::vram_w(offs_t offset, u8 data, u8 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
	m_tilemap[BIT(offset, 12)]->mark_tile_dirty((offset & 0xfff) >> 1);
}

u8 namco_cus4xtmap_device::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void namco_cus4xtmap_device::scroll_set(int layer, u8 offset, u8 data)
{
	switch (offset)
	{
		case 0: // high scroll x
			m_xscroll[layer] = (m_xscroll[layer] & 0xff) | (data << 8);
			break;
		case 1: // low scroll x
			m_xscroll[layer] = (m_xscroll[layer] & 0xff00) | data;
			break;
		case 2: // scroll y
			m_yscroll[layer] = data;
			break;
	}
}
