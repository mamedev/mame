// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_FS4600_H
#define MAME_BUS_MSX_SLOT_FS4600_H

#pragma once

#include "slot.h"
#include "machine/nvram.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_FS4600, msx_slot_fs4600_device)


class msx_slot_fs4600_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_fs4600_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_rom_start(const char *region, u32 offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr size_t SRAM_SIZE = 0x1000;

	void control_w(u8 data);
	u8 sram_r();
	void sram_w(offs_t offset, u8 data);
	template <int Bank> void bank_w(u8 data);
	u8 bank_r(offs_t offset);

	required_device<nvram_device> m_nvram;
	required_memory_region m_rom_region;
	memory_bank_array_creator<3> m_rombank;
	memory_view m_view[3];
	u32 m_region_offset;
	u32 m_sram_address;
	std::unique_ptr<u8[]> m_sram;
};


#endif // MAME_BUS_MSX_SLOT_FS4600_H
