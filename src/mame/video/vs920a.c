// license:BSD-3-Clause
// copyright-holders:David Haywood

// currently used by gstriker.c, apparently inufuku uses the same chip

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


const device_type VS920A = &device_creator<vs920a_text_tilemap_device>;

vs920a_text_tilemap_device::vs920a_text_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VS920A, "VS920A Text Tilemap", tag, owner, clock, "vs920a", __FILE__),
	m_vram(NULL),
	m_pal_base(0),
	m_gfx_region(0),
	m_gfxdecode(*this)

{
}


void vs920a_text_tilemap_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_vram = (UINT16*)auto_alloc_array_clear(this->machine(), UINT16, 0x1000/2);
	save_pointer(NAME(m_vram), 0x1000/2);
	save_item(NAME(m_pal_base));


	m_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(vs920a_text_tilemap_device::get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_tmap->set_transparent_pen(0);
}

void vs920a_text_tilemap_device::device_reset()
{
}


void vs920a_text_tilemap_device::set_gfx_region(device_t &device, int gfxregion)
{
	vs920a_text_tilemap_device &dev = downcast<vs920a_text_tilemap_device &>(device);
	dev.m_gfx_region = gfxregion;
}

void vs920a_text_tilemap_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<vs920a_text_tilemap_device &>(device).m_gfxdecode.set_tag(tag);
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


tilemap_t* vs920a_text_tilemap_device::get_tilemap()
{
	return m_tmap;
}

void vs920a_text_tilemap_device::set_pal_base(int pal_base)
{
	m_pal_base = pal_base;
}

void vs920a_text_tilemap_device::draw(screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority)
{
	m_tmap->draw(screen, bitmap, cliprect, 0, priority);
}
