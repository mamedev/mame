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

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	required_memory_region m_rom_region;
	uint32_t m_region_offset;
	const uint8_t *m_rom;
	uint8_t m_selected_bank2[4];
	const uint8_t *m_bank_base2[4];

	void map_bank();
};

class msx_slot_fsa1fm_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_fsa1fm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_rom_start(const char *region, uint32_t offset) { m_rom_region.set_tag(region); m_region_offset = offset; }

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	static constexpr size_t SRAM_SIZE = 0x2000;

	required_device<nvram_device> m_nvram;
	required_device<i8251_device> m_i8251;
	required_device<i8255_device> m_i8255;
	required_ioport m_switch_port;
	required_memory_region m_rom_region;
	uint32_t m_region_offset;
	const uint8_t *m_rom;
	uint8_t m_selected_bank[3];
	std::vector<uint8_t> m_sram;
	const uint8_t *m_bank_base_0000;
	const uint8_t *m_bank_base_4000;
	const uint8_t *m_bank_base_8000;

	void map_bank();
	void i8255_port_a_w(uint8_t data);
	void i8255_port_b_w(uint8_t data);
	uint8_t i8255_port_c_r();
};


#endif // MAME_BUS_MSX_SLOT_FSA1FM_H
