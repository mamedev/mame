// license:BSD-3-Clause
// copyright-holders:cam900
#ifndef MAME_BUS_SNES_ST018_H
#define MAME_BUS_SNES_ST018_H

#pragma once

#include "snes_slot.h"
#include "rom.h"
#include "cpu/arm7/arm7.h"
#include "machine/gen_latch.h"

// ======================> sns_rom_st018_device

class sns_rom_st018_device : public sns_rom_device
{
public:
	// construction/destruction
	sns_rom_st018_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void speedup_addon_bios_access() override;

	// additional reading and writing
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

private:
	uint32_t copro_prg_r(offs_t offset);
	uint32_t copro_data_r(offs_t offset);

	uint8_t status_r();
	void signal_w(uint8_t data);

	void copro_map(address_map &map) ATTR_COLD;

	required_device<arm7_cpu_device> m_copro;
	required_device<generic_latch_8_device> m_cpu2copro;
	required_device<generic_latch_8_device> m_copro2cpu;

	std::vector<uint32_t> m_copro_prg;
	std::vector<uint32_t> m_copro_data;

	bool m_signal;
	bool m_copro_reset;
};

// device type definition
DECLARE_DEVICE_TYPE(SNS_LOROM_ST018, sns_rom_st018_device)

#endif // MAME_BUS_SNES_ST018_H
