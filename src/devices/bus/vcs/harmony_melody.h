// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_VCS_HARMONY_MELODY_H
#define MAME_BUS_VCS_HARMONY_MELODY_H

#pragma once

#include "rom.h"
#include "cpu/arm7/lpc210x.h"


// ======================> a26_rom_harmony_device

class a26_rom_harmony_device : public a26_rom_base_device
{
public:
	a26_rom_harmony_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void install_memory_handlers(address_space *space) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;


private:
	void check_bankswitch(offs_t offset);
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t read8_r(offs_t offset);
	void harmony_arm7_map(address_map &map) ATTR_COLD;

	required_device<lpc210x_device> m_cpu;
	uint8_t m_base_bank;
};


// device type definition
DECLARE_DEVICE_TYPE(A26_ROM_HARMONY, a26_rom_harmony_device)

#endif // MAME_BUS_VCS_HARMONY_MELODY_H
