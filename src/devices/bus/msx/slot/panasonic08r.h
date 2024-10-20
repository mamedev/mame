// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_PANASONIC08R_H
#define MAME_BUS_MSX_SLOT_PANASONIC08R_H

#pragma once

#include "ram_mm.h"
#include "slot.h"
#include "machine/nvram.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_PANASONIC08R, msx_slot_panasonic08r_device)


class msx_slot_panasonic08r_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_panasonic08r_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	template <typename T> msx_slot_panasonic08r_device &set_mm_tag(T &&tag) { m_mm.set_tag(std::forward<T>(tag)); return *this; }
	void set_rom_start(const char *region, u32 offset) { m_rom_region.set_tag(region); m_region_offset = offset; }
	msx_slot_panasonic08r_device &set_sram_size(u16 size) { m_sram_size = size; return *this; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	template <int Bank> void set_view();
	template <int Bank> void bank_w(u8 data);

	void select_banks();

	u16 m_sram_size;
	required_device<nvram_device> m_nvram;
	required_memory_region m_rom_region;
	memory_bank_array_creator<8> m_bank;
	memory_view m_view[8];
	required_device<msx_slot_ram_mm_device> m_mm;
	u32 m_region_offset;
	u16 m_selected_bank[8];
	u8 m_control;
	u8 m_bank9;
	std::unique_ptr<u8[]> m_sram;
};


#endif // MAME_BUS_MSX_SLOT_PANASONIC08R_H
