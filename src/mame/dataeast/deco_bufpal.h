// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    deco_bufpal.h

    Data East buffered palette hardware.

******************************************************************************/

#ifndef MAME_DATAEAST_DECO_BUFPAL_H
#define MAME_DATAEAST_DECO_BUFPAL_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(DECO_BUFFERED_PALETTE, deco_buffered_palette_device)

class deco_buffered_palette_device : public device_t, public device_palette_interface
{
public:
	// construction/destruction
	deco_buffered_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 entries, endianness_t endianness);
	deco_buffered_palette_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_entries(u32 entries) { m_entries = entries; }

	// generic read/write handlers
	u32 read32(offs_t offset);
	void write32(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <bool IsLittleEndian> u16 read16(offs_t offset)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 4;
		return (read32(offset >> 1) >> shift) & 0xffff;
	}
	template <bool IsLittleEndian> void write16(offs_t offset, u16 data, u16 mem_mask = ~0)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0) << 4;
		write32(offset >> 1, u32(data) << shift, u32(mem_mask) << shift);
	}

	template <bool IsLittleEndian> u8 read8(offs_t offset)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0, 2) << 3;
		return (read32(offset >> 2) >> shift) & 0xff;
	}
	template <bool IsLittleEndian> void write8(offs_t offset, u8 data)
	{
		const u8 shift = BIT(IsLittleEndian ? offset : ~offset, 0, 2) << 3;
		write32(offset >> 2, u32(data) << shift, u32(0xff) << shift);
	}

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
	u32                       m_entries;              // number of entries in the palette

	// palette RAM
	memory_share_creator<u32> m_paletteram;

	// dirty flag
	std::unique_ptr<bool[]>   m_dirty;
	s32                       m_dirty_start;
	s32                       m_dirty_end;
};

#endif  // MAME_DATAEAST_DECO_BUFPAL_H
