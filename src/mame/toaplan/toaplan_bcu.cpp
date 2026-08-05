// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************

Toaplan 'BCU' tilemap generator

Implemented with PGA chip and/or NEC Gate array at some (older?) hardware.

Tilemap layer:
4 512x512 pixels tilemap (64x64 tiles)
each tile size is 8x8 pixels

Used at:
- toaplan/toaplan1.cpp
- toaplan/rallybik.cpp
- toaplan/fireshrk.cpp

Tile RAM format: (1 32 bit word per tile)

Bit                                     Description
1111 1111 1111 1111 0000 0000 0000 0000
fedc ba98 7654 3210 fedc ba98 7654 3210
xxxx ---- ---- ---- ---- ---- ---- ---- Priority*
---- ---- --xx xxxx ---- ---- ---- ---- Color index**
---- ---- ---- ---- x--- ---- ---- ---- Invisible flag
---- ---- ---- ---- -xxx xxxx xxxx xxxx Tile index

* 0 = invisible, 1...15 = backmost to frontmost
** 16 color unit
*** Unmarked bits are unused/unknown

TODO:
- VRAM data bus is actually 26 bit (or 32 bit?)
* mentioned in outzone schematics

***************************************************************************/


#include "emu.h"
#include "toaplan_bcu.h"
#include "screen.h"

#define LOG_RAM    (1 << 1)
#define LOG_SCROLL (1 << 2)
#define LOG_REGS   (1 << 3)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGRAM(...)     LOGMASKED(LOG_RAM,    __VA_ARGS__)
#define LOGSCROLL(...)  LOGMASKED(LOG_SCROLL, __VA_ARGS__)
#define LOGREGS(...)    LOGMASKED(LOG_REGS,   __VA_ARGS__)

// host interface
void toaplan_bcu_device::host_map(address_map &map)
{
	map(0x01, 0x01).w(FUNC(toaplan_bcu_device::flipscreen_w));
	map(0x02, 0x03).rw(FUNC(toaplan_bcu_device::tileram_offs_r), FUNC(toaplan_bcu_device::tileram_offs_w));
	map(0x04, 0x07).rw(FUNC(toaplan_bcu_device::tileram_r), FUNC(toaplan_bcu_device::tileram_w));
	map(0x10, 0x1f).rw(FUNC(toaplan_bcu_device::scroll_regs_r), FUNC(toaplan_bcu_device::scroll_regs_w));
}

DEFINE_DEVICE_TYPE(TOAPLAN_BCU, toaplan_bcu_device, "toaplan_bcu", "Toaplan BCU")

toaplan_bcu_device::toaplan_bcu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TOAPLAN_BCU, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_vram(*this, "vram_%u", 0U, 0x4000U, ENDIANNESS_BIG)
	, m_tilemap{nullptr}
	, m_ram_offs(0)
	, m_scrollx{0}
	, m_scrolly{0}
	, m_offsetx(0)
	, m_offsety(0)
	, m_flipscreen(-1)
	, m_dx(0)
	, m_dy(0)
	, m_dx_flipped(0)
	, m_dy_flipped(0)
{
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

template<unsigned Layer>
TILE_GET_INFO_MEMBER(toaplan_bcu_device::get_tile_info)
{
	const u16 tile_number = m_vram[Layer][tile_index] & 0x7fff;
	const u16 attrib = (m_vram[Layer][tile_index] >> 16) & 0xf03f;
	const u16 color = attrib & 0x3f;
	tileinfo.set(0, tile_number, color, 0);
	// "invisible" tiles are behind everything else
	if (BIT(m_vram[Layer][tile_index], 15))
		tileinfo.category = 16;
	else
		tileinfo.category = (attrib & 0xf000) >> 12;
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void toaplan_bcu_device::device_start()
{
	m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(toaplan_bcu_device::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(toaplan_bcu_device::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(toaplan_bcu_device::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[3] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(toaplan_bcu_device::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);
	m_tilemap[3]->set_transparent_pen(0);

	m_tilemap[0]->set_scrolldx(m_dx - 6, m_dx_flipped + 6);
	m_tilemap[1]->set_scrolldx(m_dx - 4, m_dx_flipped + 4);
	m_tilemap[2]->set_scrolldx(m_dx - 2, m_dx_flipped + 2);
	m_tilemap[3]->set_scrolldx(m_dx - 0, m_dx_flipped + 0);

	m_tilemap[0]->set_scrolldy(m_dy, m_dy_flipped);
	m_tilemap[1]->set_scrolldy(m_dy, m_dy_flipped);
	m_tilemap[2]->set_scrolldy(m_dy, m_dy_flipped);
	m_tilemap[3]->set_scrolldy(m_dy, m_dy_flipped);

	save_item(NAME(m_ram_offs));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_offsetx));
	save_item(NAME(m_offsety));
	save_item(NAME(m_flipscreen));
}

void toaplan_bcu_device::device_reset()
{
}

void toaplan_bcu_device::device_post_load()
{
	set_scrolls();
}

void toaplan_bcu_device::set_scrolls()
{
	for (int i = 0; i < 4; i++)
	{
		m_tilemap[i]->set_scrollx(0, (m_scrollx[i] >> 7) - m_offsetx);
		m_tilemap[i]->set_scrolly(0, (m_scrolly[i] >> 7) - m_offsety);
	}
}

// read/write handlers

u16 toaplan_bcu_device::tileram_offs_r()
{
	return m_ram_offs;
}

void toaplan_bcu_device::tileram_offs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (data >= 0x4000)
		LOGRAM("%s: Hmmm, unknown video layer being selected (%04x & %04x)\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_ram_offs);
}


u16 toaplan_bcu_device::tileram_r(offs_t offset)
{
	const int layer = m_ram_offs >> 12;
	const int offs = m_ram_offs & 0xfff;
	u16 video_data = 0;

	// locate layer
	switch (layer)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
			video_data = (m_vram[layer][offs] >> (BIT(~offset, 0) << 4)) & 0xffff;
			break;
		default:
			if (!machine().side_effects_disabled())
				LOGRAM("%s: Hmmm, reading %04x from unknown playfield layer address %06x  Offset:%01x !!!\n", machine().describe_context(), video_data, m_ram_offs, offset);
			break;
	}

	return video_data;
}

void toaplan_bcu_device::tileram_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int layer = m_ram_offs >> 12;
	const int offs = m_ram_offs & 0xfff;

	// locate layer
	switch (layer)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		{
			const u8 shift = BIT(~offset, 0) << 4;
			m_vram[layer][offs] = (m_vram[layer][offs] & ~(u32(mem_mask) << shift)) | (u32(data & mem_mask) << shift);
			m_tilemap[layer]->mark_tile_dirty(offs);
			break;
		}
		default:
			LOGRAM("%s: Hmmm, writing %04x & %04x to unknown playfield layer address %04x  Offset:%01x\n", machine().describe_context(), data, mem_mask, m_ram_offs, offset);
			break;
	}
}

u16 toaplan_bcu_device::scroll_regs_r(offs_t offset)
{
	const int layer = (offset >> 1);
	u16 scroll = 0;

	switch (offset)
	{
		case 0x0:
		case 0x2:
		case 0x4:
		case 0x6: scroll = m_scrollx[layer]; break;
		case 0x1:
		case 0x3:
		case 0x5:
		case 0x7: scroll = m_scrolly[layer]; break;
		default:
			if (!machine().side_effects_disabled())
				LOGSCROLL("%s: Hmmm, reading unknown video scroll register (%02x) !!!\n", machine().describe_context(), offset);
			break;
	}
	return scroll;
}


void toaplan_bcu_device::scroll_regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int layer = (offset >> 1);
	switch (offset)
	{
		case 0x0:      /* 1D3h */
		case 0x2:      /* 1D5h */
		case 0x4:      /* 1D7h */
		case 0x6:      /* 1D9h */
			COMBINE_DATA(&m_scrollx[layer]);
			m_tilemap[layer]->set_scrollx(0, (m_scrollx[layer] >> 7) - m_offsetx);
			break;
		case 0x1:      /* 1EBh */
		case 0x3:      /* 1EBh */
		case 0x5:      /* 1EBh */
		case 0x7:      /* 1EBh */
			COMBINE_DATA(&m_scrolly[layer]);
			m_tilemap[layer]->set_scrolly(0, (m_scrolly[layer] >> 7) - m_offsety);
			break;
		default:
			LOGSCROLL("%s: Hmmm, writing %04x to unknown video scroll register (%02x) !!!\n", machine().describe_context(), data ,offset);
			break;
	}
}

void toaplan_bcu_device::tile_offsets_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_offsetx);
		LOGSCROLL("%s: Tiles_offsetx now = %08x\n", machine().describe_context(), m_offsetx);
	}
	else
	{
		COMBINE_DATA(&m_offsety);
		LOGSCROLL("%s: Tiles_offsety now = %08x\n", machine().describe_context(), m_offsety);
	}
	set_scrolls();
}

void toaplan_bcu_device::flipscreen_w(u8 data)
{
	if (data != m_flipscreen)
	{
		LOGREGS("%s: Setting BCU controller flipscreen port to %02x\n", machine().describe_context(), data);
		m_flipscreen = BIT(data, 0);     /* 0x0001 = flip, 0x0000 = no flip */
		machine().tilemap().set_flip_all((data ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));

		set_scrolls();
	}
}

// draw routine

template <class BitmapClass>
void toaplan_bcu_device::draw_background_base(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask)
{
	// first draw everything, including "invisible" tiles and priority 0
	m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES | flags, pri, pri_mask);
}

void toaplan_bcu_device::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask)
{
	draw_background_base(screen, bitmap, cliprect, flags, pri, pri_mask);
}

void toaplan_bcu_device::draw_background(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask)
{
	draw_background_base(screen, bitmap, cliprect, flags, pri, pri_mask);
}

template <class BitmapClass>
void toaplan_bcu_device::draw_tilemap_base(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask)
{
	m_tilemap[3]->draw(screen, bitmap, cliprect, flags, pri, pri_mask);
	m_tilemap[2]->draw(screen, bitmap, cliprect, flags, pri, pri_mask);
	m_tilemap[1]->draw(screen, bitmap, cliprect, flags, pri, pri_mask);
	m_tilemap[0]->draw(screen, bitmap, cliprect, flags, pri, pri_mask);
}

void toaplan_bcu_device::draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask)
{
	draw_tilemap_base(screen, bitmap, cliprect, flags, pri, pri_mask);
}

void toaplan_bcu_device::draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask)
{
	draw_tilemap_base(screen, bitmap, cliprect, flags, pri, pri_mask);
}
