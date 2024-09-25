// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_SONY08_H
#define MAME_BUS_MSX_SLOT_SONY08_H

#pragma once

#include "slot.h"
#include "machine/nvram.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_SONY08, msx_slot_sony08_device)


class msx_slot_sony08_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_sony08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_rom_start(const char *region, u32 offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr size_t SRAM_SIZE = 0x4000;

	template <int Bank> void bank_w(u8 data);

	required_device<nvram_device> m_nvram;
	required_memory_region m_rom_region;
	memory_bank_array_creator<6> m_rombank;
	memory_view m_view[2];
	u32 m_region_offset;
	std::unique_ptr<u8[]> m_sram;
};


#endif // MAME_BUS_MSX_SLOT_SONY08_H
