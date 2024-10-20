// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_ROM_H
#define MAME_BUS_MSX_SLOT_ROM_H

#pragma once

#include "slot.h"

class msx_slot_rom_device : public device_t,
							public msx_internal_slot_interface
{
public:
	msx_slot_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_rom_start(const char *region, uint32_t offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

protected:
	msx_slot_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	u8 *rom_base() { return m_rom_region->base() + m_region_offset; }

	required_memory_region m_rom_region;
	u32 m_region_offset;
};

DECLARE_DEVICE_TYPE(MSX_SLOT_ROM, msx_slot_rom_device)

#endif // MAME_BUS_MSX_SLOT_ROM_H
