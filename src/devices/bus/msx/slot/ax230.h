// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_AX230_H
#define MAME_BUS_MSX_SLOT_AX230_H

#pragma once

#include "slot.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_AX230, msx_slot_ax230_device)


class msx_slot_ax230_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_ax230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_rom_start(const char *region, u32 offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u8 BANKS = 0x80;
	static constexpr u8 BANK_MASK = BANKS - 1;

	void mapper_write(offs_t offset, uint8_t data);

	required_memory_region m_rom_region;
	memory_bank_array_creator<4> m_rombank;
	u32 m_region_offset;
};


#endif // MAME_BUS_MSX_SLOT_AX230_H
