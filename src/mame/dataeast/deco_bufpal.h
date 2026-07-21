// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    deco_bufpal.h

    Data East buffered palette hardware.

******************************************************************************/

#ifndef MAME_DATAEAST_DECO_BUFPAL_H
#define MAME_DATAEAST_DECO_BUFPAL_H

#pragma once

#include "memarray.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(DECO_BUFFERED_PALETTE, deco_buffered_palette_device)

class deco_buffered_palette_device : public device_t, public device_palette_interface
{
public:
	// construction/destruction
	deco_buffered_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 entries);
	deco_buffered_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
//  void set_membits(int membits);
//  void set_endianness(endianness_t endianness);
	void set_entries(u32 entries) { m_entries = entries; }

	// palette RAM accessors
	memory_array &basemem() { return m_paletteram; }
	memory_array &extmem() { return m_paletteram_ext; }

	// raw entry reading
	u32 read_entry(pen_t pen) const
	{
		u32 data = m_paletteram.read(pen);
		if (m_paletteram_ext.base() != nullptr)
			data |= m_paletteram_ext.read(pen) << (8 * m_paletteram.bytes_per_entry());
		return data;
	}

	// generic read/write handlers
	u8 read8(offs_t offset);
	void write8(offs_t offset, u8 data);
	void write8_ext(offs_t offset, u8 data);
	u16 read16(offs_t offset);
	void write16(offs_t offset, u16 data, u16 mem_mask = ~0);
	void write16_ext(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 read32(offs_t offset);
	void write32(offs_t offset, u32 data, u32 mem_mask = ~0);

	void dma_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_palette_interface overrides
	virtual u32 palette_entries() const noexcept override { return m_entries; }

private:
	void set_dirty_area(s32 offset);
	rgb_t xbgr_888_decoder(u32 raw);

	// configuration state
	u32                     m_entries;              // number of entries in the palette
//  u32                     m_indirect_entries;     // number of indirect colors in the palette
//  bool                    m_enable_shadows;       // are shadows enabled?
//  bool                    m_enable_highlights;    // are highlights enabled?
//  int                     m_membits;              // width of palette RAM, if different from native
//  bool                    m_membits_supplied;     // true if membits forced in static config
//  endianness_t            m_endianness;           // endianness of palette RAM, if different from native
//  bool                    m_endianness_supplied;  // true if endianness forced in static config

	// palette RAM
	memory_array            m_paletteram;           // base memory
	memory_array            m_paletteram_ext;       // extended memory

	// dirty flag
	std::unique_ptr<bool[]> m_dirty;
	s32                     m_dirty_start;
	s32                     m_dirty_end;
};

#endif  // MAME_DATAEAST_DECO_BUFPAL_H
