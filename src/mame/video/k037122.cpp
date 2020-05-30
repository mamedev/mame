// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*
Konami 037122

	This chip has CLUT, CRTC, Variable size single tilemap

	Color lookup table is 16 bit BRG format, total 8192 entries (256 color per each tile * 32 banks))

	Color format (4 byte (1x32bit word) per each color)
	Bits                              Description
	fedcba9876543210 fedcba9876543210
	---------------- xxxxx----------- Blue
	---------------- -----xxxxx------ Red
	---------------- ----------xxxxxx Green

	Tilemap is can be rotated, zoomed, similar as K053936 "simple mode"
	Tilemap size is 256x64 (2048x512 pixels) or 128x64 (1024x512 pixels), Each tile is 8bpp 8x8.

	Tile format (4 byte (1x32bit word) per each tile)
	Bits                              Description
	fedcba9876543210 fedcba9876543210
	--------x------- ---------------- Flip Y
	---------x------ ---------------- Flip X
	----------xxxxx- ---------------- CLUT Bank index (256 color granularity)
	---------------- --xxxxxxxxxxxxxx Tile code (from character RAM, 8x8 pixel granularity (128 byte))

	Other bits unknown

	Register map
	00-0f Display timing
	20-2f Scroll/ROZ register
	30-3f Control, etc

	Offset Bits                              Description
	       fedcba9876543210 fedcba9876543210
	00     xxxxxxxxxxxxxxxx ---------------- Horizontal total pixels - 1
	       ---------------- xxxxxxxxxxxxxxxx Horizontal sync width - 1
	04     xxxxxxxxxxxxxxxx ---------------- Horizontal front porch - 5
	       ---------------- xxxxxxxxxxxxxxxx Horizontal back porch + 5
	08     xxxxxxxxxxxxxxxx ---------------- Vertical total pixels - 1
	       ---------------- xxxxxxxxxxxxxxxx Vertical sync width - 1
	0c     xxxxxxxxxxxxxxxx ---------------- Vertical front porch + 1
	       ---------------- xxxxxxxxxxxxxxxx Vertical back porch - 2
	20     sxxxxxxxxxxxxxxx ---------------- X counter starting value (12.4 fixed point)
	       ---------------- sxxxxxxxxxxxxxxx Y counter starting value (12.4 fixed point)
	24     ---------------- sxxxxxxxxxxxxxxx amount to add to the Y counter after each line (4.12 fixed point)
	28     sxxxxxxxxxxxxxxx ---------------- amount to add to the X counter after each horizontal pixel (4.12 fixed point)
	30     ---------------x ---------------- VRAM mapping mode
	       ---------------0 ---------------- CLUT at 0x00000-0x08000, Display tilemap area at 0x08000-0x18000 (256x64)
	       ---------------1 ---------------- CLUT at 0x18000-0x20000, Display tilemap area at 0x00000-0x08000 (128x64)
	       ---------------- -------------xxx Character RAM bank

	Other bits/registers unknown, some registers are used

TODO:
	- verify and implement scroll, ROZ, display timing registers
	- verify other unknown but used registers
*/

#include "emu.h"
#include "k037122.h"
#include "konami_helper.h"
#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"


#define K037122_NUM_TILES       16384

DEFINE_DEVICE_TYPE(K037122, k037122_device, "k037122", "K037122 2D Tilemap")

k037122_device::k037122_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, K037122, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_gfx_interface(mconfig, *this, nullptr),
	m_tile_ram(nullptr),
	m_char_ram(nullptr),
	m_reg(nullptr),
	m_gfx_index(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k037122_device::device_start()
{
	if (!palette().device().started())
		throw device_missing_dependencies();

	static const gfx_layout k037122_char_layout =
	{
	8, 8,
	K037122_NUM_TILES,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128 },
	8*128
	};

	m_char_ram = make_unique_clear<uint32_t[]>(0x200000 / 4);
	m_tile_ram = make_unique_clear<uint32_t[]>(0x20000 / 4);
	m_reg = make_unique_clear<uint32_t[]>(0x400 / 4);

	m_layer[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k037122_device::tile_info_layer0)), TILEMAP_SCAN_ROWS, 8, 8, 256, 64);
	m_layer[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k037122_device::tile_info_layer1)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);

	m_layer[0]->set_transparent_pen(0);
	m_layer[1]->set_transparent_pen(0);

	set_gfx(m_gfx_index,std::make_unique<gfx_element>(&palette(), k037122_char_layout, (uint8_t*)m_char_ram.get(), 0, palette().entries() / 16, 0));

	save_pointer(NAME(m_reg), 0x400 / 4);
	save_pointer(NAME(m_char_ram), 0x200000 / 4);
	save_pointer(NAME(m_tile_ram), 0x20000 / 4);

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k037122_device::device_reset()
{
	memset(m_char_ram.get(), 0, 0x200000);
	memset(m_tile_ram.get(), 0, 0x20000);
	memset(m_reg.get(), 0, 0x400);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

TILE_GET_INFO_MEMBER(k037122_device::tile_info_layer0)
{
	uint32_t val = m_tile_ram[tile_index + (0x8000/4)];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x3fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tileinfo.set(m_gfx_index, tile, color, flags);
}

TILE_GET_INFO_MEMBER(k037122_device::tile_info_layer1)
{
	uint32_t val = m_tile_ram[tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = val & 0x3fff;
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tileinfo.set(m_gfx_index, tile, color, flags);
}


void k037122_device::tile_draw( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	const rectangle &visarea = screen.visible_area();

	if (m_reg[0xc] & 0x10000)
	{
		m_layer[1]->set_scrolldx(visarea.min_x, visarea.min_x);
		m_layer[1]->set_scrolldy(visarea.min_y, visarea.min_y);
		m_layer[1]->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_layer[0]->set_scrolldx(visarea.min_x, visarea.min_x);
		m_layer[0]->set_scrolldy(visarea.min_y, visarea.min_y);
		m_layer[0]->draw(screen, bitmap, cliprect, 0, 0);
	}
}

void k037122_device::update_palette_color( uint32_t palette_base, int color )
{
	uint32_t data = m_tile_ram[(palette_base / 4) + color];

	palette().set_pen_color(color, pal5bit(data >> 6), pal6bit(data >> 0), pal5bit(data >> 11));
}

uint32_t k037122_device::sram_r(offs_t offset)
{
	return m_tile_ram[offset];
}

void k037122_device::sram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_tile_ram.get() + offset);

	if (m_reg[0xc] & 0x10000)
	{
		if (offset < 0x8000 / 4)
		{
			m_layer[1]->mark_tile_dirty(offset);
		}
		else if (offset >= 0x8000 / 4 && offset < 0x18000 / 4)
		{
			m_layer[0]->mark_tile_dirty(offset - (0x8000 / 4));
		}
		else if (offset >= 0x18000 / 4)
		{
			update_palette_color(0x18000, offset - (0x18000 / 4));
		}
	}
	else
	{
		if (offset < 0x8000 / 4)
		{
			update_palette_color(0, offset);
		}
		else if (offset >= 0x8000 / 4 && offset < 0x18000 / 4)
		{
			m_layer[0]->mark_tile_dirty(offset - (0x8000 / 4));
		}
		else if (offset >= 0x18000 / 4)
		{
			m_layer[1]->mark_tile_dirty(offset - (0x18000 / 4));
		}
	}
}


uint32_t k037122_device::char_r(offs_t offset)
{
	int bank = m_reg[0x30 / 4] & 0x7;

	return m_char_ram[offset + (bank * (0x40000 / 4))];
}

void k037122_device::char_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int bank = m_reg[0x30 / 4] & 0x7;
	uint32_t addr = offset + (bank * (0x40000/4));

	COMBINE_DATA(m_char_ram.get() + addr);
	gfx(m_gfx_index)->mark_dirty(addr / 32);
}

uint32_t k037122_device::reg_r(offs_t offset)
{
	switch (offset)
	{
		case 0x14/4:
		{
			return 0x000003fa;
		}
	}
	return m_reg[offset];
}

void k037122_device::reg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_reg.get() + offset);
}
