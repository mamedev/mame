// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#ifndef MAME_SINCLAIR_NEOGS_H
#define MAME_SINCLAIR_NEOGS_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "sound/dac.h"

DECLARE_DEVICE_TYPE(NEOGS, neogs_device)

class neogs_device : public device_t
{
public:
	neogs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0)
		: device_t(mconfig, NEOGS, tag, owner, clock)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_bank_rom(*this, "bank_rom")
		, m_bank_ram(*this, "bank_ram")
		, m_view(*this, "view")
		, m_dac(*this, "dac%u", 0U)
	{ }

	u8 status_r() { return m_status; };
	void command_w(u8 data) { m_status |= 0x01; m_command_in = data; };
	u8 data_r() { m_status &= ~0x80; return m_data_out; };
	void data_w(u8 data) { m_status |= 0x80; m_data_in = data; };
	void ctrl_w(u8 data);

protected:
	// device-level overrides
	void device_start() override;
	void device_reset() override;
	const tiny_rom_entry *device_rom_region() const override;

	void device_add_mconfig(machine_config &config) override;

	INTERRUPT_GEN_MEMBER(irq0_line_assert);

	void map_memory(address_map &map);
	void map_io(address_map &map);
	void update_config();

	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	memory_bank_creator m_bank_rom;
	memory_bank_creator m_bank_ram;
	memory_view m_view;
	required_device_array<dac_word_interface, 8> m_dac;

private:
	template <u8 Bank> u8 ram_bank_r(offs_t offset);
	template <u8 Bank> void ram_bank_w(offs_t offset, u8 data);

	u8 m_data_in;
	u8 m_data_out;
	u8 m_command_in;
	u8 m_status;

	u8 m_mpag;
	u8 m_mpagx;
	u8 m_gscfg0;
	u8 m_vol[8];
};

#endif // MAME_SINCLAIR_NEOGS_H
