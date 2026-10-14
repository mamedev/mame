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
	: deco_buffered_palette_device(mconfig, tag, owner, clock, 0, ENDIANNESS_LITTLE)
{
}

deco_buffered_palette_device::deco_buffered_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 entries, endianness_t endianness)
	: device_t(mconfig, DECO_BUFFERED_PALETTE, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_entries(entries)
	, m_paletteram(*this, "paletteram", entries << 2, endianness)
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
			set_pen_color(i, xbgr_888_decoder(m_paletteram[i]));
			m_dirty[i] = false;
		}
	}
	m_dirty_start = m_entries;
	m_dirty_end = -1;
}

//-------------------------------------------------
//  write* - write a word to the base paletteram
//-------------------------------------------------

void deco_buffered_palette_device::write32(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	set_dirty_area(offset);
}

u32 deco_buffered_palette_device::read32(offs_t offset)
{
	return m_paletteram[offset];
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
