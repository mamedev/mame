// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    deco_bufpal.cpp

    Data East buffered palette hardware.

    Used by:
    - dataeast/rohga.cpp
    - dataeast/dragngun.cpp
    - dataeast/fghthist.cpp (fghthist*)

***************************************************************************/

#include "emu.h"
#include "deco_bufpal.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DECO_BUFFERED_PALETTE, deco_buffered_palette_device, "deco_bufpal", "DECO Buffered Palette Hardware")

deco_buffered_palette_device::deco_buffered_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: deco_buffered_palette_device(mconfig, tag, owner, clock, 0)
{
}

deco_buffered_palette_device::deco_buffered_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 entries)
	: device_t(mconfig, DECO_BUFFERED_PALETTE, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_entries(entries)
	, m_dirty_start(entries)
	, m_dirty_end(-1)
{
}


//**************************************************************************
//  GENERIC WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  set_dirty_area - set dirty area boundary
//-------------------------------------------------

void deco_buffered_palette_device::set_dirty_area(s32 offset)
{
	m_dirty[offset] = true;
	if (m_dirty_start > offset)
		m_dirty_start = offset;
	if (m_dirty_end < offset)
		m_dirty_end = offset;
}

//-------------------------------------------------
//  dma_w - DMA request to buffer
//-------------------------------------------------

void deco_buffered_palette_device::dma_w(u8 data)
{
	if (m_dirty_start > m_dirty_end)
		return;

	for (u32 i = m_dirty_start; i <= m_dirty_end; i++)
	{
		if (m_dirty[i])
		{
			set_pen_color(i, xbgr_888_decoder(read_entry(i)));
			m_dirty[i] = false;
		}
	}
	m_dirty_start = m_entries;
	m_dirty_end = -1;
}

//-------------------------------------------------
//  write* - write a byte to the base paletteram
//-------------------------------------------------

void deco_buffered_palette_device::write8(offs_t offset, u8 data)
{
	m_paletteram.write8(offset, data);
	set_dirty_area(offset >> 2);
}

void deco_buffered_palette_device::write16(offs_t offset, u16 data, u16 mem_mask)
{
	m_paletteram.write16(offset, data, mem_mask);
	set_dirty_area(offset >> 1);
}

void deco_buffered_palette_device::write32(offs_t offset, u32 data, u32 mem_mask)
{
	m_paletteram.write32(offset, data, mem_mask);
	set_dirty_area(offset);
}

u8 deco_buffered_palette_device::read8(offs_t offset)
{
	return m_paletteram.read8(offset);
}

u16 deco_buffered_palette_device::read16(offs_t offset)
{
	return m_paletteram.read16(offset);
}

u32 deco_buffered_palette_device::read32(offs_t offset)
{
	return m_paletteram.read32(offset);
}


//-------------------------------------------------
//  write*_ext - write a byte to the extended
//  paletteram
//-------------------------------------------------

void deco_buffered_palette_device::write8_ext(offs_t offset, u8 data)
{
	m_paletteram_ext.write8(offset, data);
	set_dirty_area(offset >> 2);
}


void deco_buffered_palette_device::write16_ext(offs_t offset, u16 data, u16 mem_mask)
{
	m_paletteram_ext.write16(offset, data, mem_mask);
	set_dirty_area(offset >> 1);
}


//**************************************************************************
//  DEVICE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void deco_buffered_palette_device::device_start()
{
	assert(m_entries > 0);

	// find the memory, if present
	const memory_share *share = memshare(tag());
	if (share != nullptr)
	{
		// find the extended (split) memory, if present
		std::string tag_ext = std::string(tag()).append("_ext");
		const memory_share *share_ext = memshare(tag_ext.c_str());

		// determine bytes per entry and configure
		const int bytes_per_entry = 4;
		if (share_ext == nullptr)
			m_paletteram.set(*share, bytes_per_entry);
		else
		{
			m_paletteram.set(*share, bytes_per_entry / 2);
			m_paletteram_ext.set(*share_ext, bytes_per_entry / 2);
		}
	}
	const u32 entries = m_entries;
	m_dirty = make_unique_clear<bool[]>(entries);

	save_pointer(NAME(m_dirty), entries);
	save_item(NAME(m_dirty_start));
	save_item(NAME(m_dirty_end));
}


rgb_t deco_buffered_palette_device::xbgr_888_decoder(u32 raw)
{
	// exact resistor unknown
	return rgb_t(raw & 0xff, (raw >> 8) & 0xff, (raw >> 16) & 0xff);
}
