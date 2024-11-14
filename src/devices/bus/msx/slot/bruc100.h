// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_BRUC100_H
#define MAME_BUS_MSX_SLOT_BRUC100_H

#pragma once

#include "slot.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_BRUC100, msx_slot_bruc100_device)


class msx_slot_bruc100_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_bruc100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_rom_start(const char *region, u32 offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

	void select_bank(u8 bank);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_memory_region m_rom_region;
	memory_bank_array_creator<2> m_rombank;
	u32 m_region_offset;
};


#endif // MAME_BUS_MSX_SLOT_BRUC100_H
