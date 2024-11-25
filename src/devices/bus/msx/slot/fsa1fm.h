// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_FSA1FM_H
#define MAME_BUS_MSX_SLOT_FSA1FM_H

#pragma once

#include "slot.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/nvram.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_FSA1FM, msx_slot_fsa1fm_device)
DECLARE_DEVICE_TYPE(MSX_SLOT_FSA1FM2, msx_slot_fsa1fm2_device)


class msx_slot_fsa1fm2_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_fsa1fm2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_rom_start(const char *region, uint32_t offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr size_t RAM_SIZE = 0x2000;

	template <int Bank> void set_view();
	template <int Bank> void bank_w(u8 data);
	u8 bank_r(offs_t offset);
	void control_w(u8 data);

	required_memory_region m_rom_region;
	memory_bank_array_creator<6> m_bank;
	memory_view m_view[6];
	uint32_t m_region_offset;
	uint8_t m_selected_bank[6];
	bool m_ram_active[6];
	uint8_t m_control;
	std::unique_ptr<u8[]> m_ram;
	std::unique_ptr<u8[]> m_empty_bank;
};


class msx_slot_fsa1fm_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_fsa1fm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_rom_start(const char *region, uint32_t offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	static constexpr size_t SRAM_SIZE = 0x2000;

	void map_bank();
	void i8255_port_a_w(u8 data);
	void i8255_port_b_w(u8 data);
	u8 i8255_port_c_r();

	required_device<nvram_device> m_nvram;
	required_device<i8251_device> m_i8251;
	required_device<i8255_device> m_i8255;
	required_ioport m_switch_port;
	required_memory_region m_rom_region;
	memory_bank_creator m_rombank;
	u32 m_region_offset;
	std::unique_ptr<u8[]> m_sram;
};


#endif // MAME_BUS_MSX_SLOT_FSA1FM_H
