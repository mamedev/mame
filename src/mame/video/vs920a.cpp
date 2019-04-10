// license:BSD-3-Clause
// copyright-holders:David Haywood

// currently used by gstriker.cpp, apparently inufuku uses the same chip

/*** VS920A (score tilemap) **********************************************/

/*

    VS920A - (Very) Simple tilemap chip
    -----------------------------------

- 1 Plane
- Tiles 8x8, 4bpp
- Map 64x64
- No scrolling or other effects (at least in gstriker)


    Videoram format:
    ----------------

pppp tttt tttt tttt

t=tile, p=palette

*/


#include "emu.h"
#include "vs920a.h"


DEFINE_DEVICE_TYPE(VS920A, vs920a_text_tilemap_device, "vs920a", "VS920A Text Tilemap")

vs920a_text_tilemap_device::vs920a_text_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VS920A, tag, owner, clock)
	, m_tmap(nullptr)
	, m_vram()
	, m_pal_base(0)
	, m_gfx_region(0)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}


void vs920a_text_tilemap_device::device_start()
{
	if (!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_vram = make_unique_clear<uint16_t[]>(0x1000/2);
	save_pointer(NAME(m_vram), 0x1000/2);
	save_item(NAME(m_pal_base));


	m_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(vs920a_text_tilemap_device::get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_tmap->set_transparent_pen(0);
}

void vs920a_text_tilemap_device::device_reset()
{
}

TILE_GET_INFO_MEMBER(vs920a_text_tilemap_device::get_tile_info)
{
	int data;
	int tileno, pal;

	data = m_vram[tile_index];

	tileno = data & 0xFFF;
	pal =   (data >> 12) & 0xF;

	SET_TILE_INFO_MEMBER(m_gfx_region, tileno, m_pal_base + pal, 0);
}

WRITE16_MEMBER(vs920a_text_tilemap_device::vram_w)
{
	COMBINE_DATA(&m_vram[offset]);
	m_tmap->mark_tile_dirty(offset);
}


READ16_MEMBER(vs920a_text_tilemap_device::vram_r)
{
	return m_vram[offset];
}


void vs920a_text_tilemap_device::draw(screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority)
{
	m_tmap->draw(screen, bitmap, cliprect, 0, priority);
}
