// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_VCS_HARMONY_MELODY_H
#define MAME_BUS_VCS_HARMONY_MELODY_H

#pragma once

#include "rom.h"
#include "cpu/arm7/lpc210x.h"


// ======================> a26_rom_harmony_device

class a26_rom_harmony_device : public a26_rom_f6_device
{
public:
	a26_rom_harmony_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

	virtual void install_memory_handlers(address_space *space) override;

private:
	void check_bankswitch(offs_t offset);
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t read8_r(offs_t offset);
	void harmony_arm7_map(address_map &map);

	required_device<lpc210x_device> m_cpu;
};


// device type definition
DECLARE_DEVICE_TYPE(A26_ROM_HARMONY, a26_rom_harmony_device)

#endif // MAME_BUS_VCS_HARMONY_MELODY_H
