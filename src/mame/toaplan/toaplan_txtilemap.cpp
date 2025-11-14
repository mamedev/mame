// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood
/***************************************************************************

Common Toaplan text tilemap generator usally paired with GP9001

Tilemap: Single 64x32 tilemap (tile size: 8x8) with Per-line scroll

RAM format (original docs from toaplan/truxton2.cpp)

Text RAM format      $0000-1FFF (actually its probably $0000-0FFF)
---- --xx xxxx xxxx = Tile number
xxxx xx-- ---- ---- = Color (0 - 3Fh) + 40h

Line select / flip   $0000-01EF (some games go to $01FF (excess?))
---x xxxx xxxx xxxx = Line select for each line
x--- ---- ---- ---- = X flip for each line ???

Line scroll          $0000-01EF (some games go to $01FF (excess?))
---- ---x xxxx xxxx = X scroll for each line

Used in:
- toaplan/fixeight.cpp
- toaplan/raizing_batrider.cpp
- toaplan/raizing.cpp
- toaplan/truxton2.cpp

***************************************************************************/


#include "emu.h"
#include "toaplan_txtilemap.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(TOAPLAN_TXTILEMAP, toaplan_txtilemap_device, "toaplan_txtilemap", "Toaplan Text tilemap generator")

toaplan_txtilemap_device::toaplan_txtilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TOAPLAN_TXTILEMAP, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_videoram(*this, "videoram", 0x2000U, ENDIANNESS_BIG)
	, m_linescroll(*this, "linescroll", 0x200U, ENDIANNESS_BIG)
	, m_lineselect(*this, "lineselect", 0x1000U, ENDIANNESS_BIG) // some hardware has 4KB line select RAM
	, m_tilemap(nullptr)
	, m_dx(0)
	, m_dy(0)
	, m_dx_flipped(0)
	, m_dy_flipped(0)
{
}

TILE_GET_INFO_MEMBER(toaplan_txtilemap_device::get_tile_info)
{
	const u16 attrib = m_videoram[tile_index];
	const u32 tile_number = attrib & 0x3ff;
	const u32 color = (attrib >> 10) & 0x3f;
	tileinfo.set(0, tile_number, color, 0);
}


void toaplan_txtilemap_device::device_start()
{
	m_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(toaplan_txtilemap_device::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap->set_scroll_rows(8*32); /* line scrolling */
	m_tilemap->set_scroll_cols(1);
	m_tilemap->set_scrolldx(m_dx, m_dx_flipped);
	m_tilemap->set_scrolldy(m_dy, m_dy_flipped);
	m_tilemap->set_transparent_pen(0);
}

void toaplan_txtilemap_device::device_reset()
{
}

// read/write handlers

u16 toaplan_txtilemap_device::videoram_r(offs_t offset)
{
	return m_videoram[offset];
}

u16 toaplan_txtilemap_device::linescroll_r(offs_t offset)
{
	return m_linescroll[offset];
}

u16 toaplan_txtilemap_device::lineselect_r(offs_t offset)
{
	return m_lineselect[offset];
}

void toaplan_txtilemap_device::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	if (offset < 64*32)
		m_tilemap->mark_tile_dirty(offset);
}

void toaplan_txtilemap_device::linescroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*** Line-Scroll RAM for Text Layer ***/
	COMBINE_DATA(&m_linescroll[offset]);
	m_tilemap->set_scrollx(offset, m_linescroll[offset]);
}

void toaplan_txtilemap_device::lineselect_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_lineselect[offset]);
}

// draw routine

template <class BitmapClass>
void toaplan_txtilemap_device::draw_tilemap_base(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect)
{
	rectangle clip = cliprect;

	/* it seems likely that flipx can be set per line! */
	/* however, none of the games does it, and emulating it in the */
	/* MAME tilemap system without being ultra slow would be tricky */
	m_tilemap->set_flip(m_lineselect[0] & 0x8000 ? 0 : TILEMAP_FLIPX);

	/* line select is used for 'for use in' and '8ing' screen on bbakraid, 'Raizing' logo on batrider */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		clip.min_y = clip.max_y = y;
		m_tilemap->set_scrolly(0, m_lineselect[y] - y);
		m_tilemap->draw(screen, bitmap, clip, 0);
	}
}

template <class BitmapClass>
void toaplan_txtilemap_device::draw_tilemap_bootleg_base(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect)
{
	// bootleg hardware has no scroll RAM
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
}

void toaplan_txtilemap_device::draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_tilemap_base(screen, bitmap, cliprect);
}

void toaplan_txtilemap_device::draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	draw_tilemap_base(screen, bitmap, cliprect);
}

void toaplan_txtilemap_device::draw_tilemap_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_tilemap_bootleg_base(screen, bitmap, cliprect);
}

void toaplan_txtilemap_device::draw_tilemap_bootleg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	draw_tilemap_bootleg_base(screen, bitmap, cliprect);
}
