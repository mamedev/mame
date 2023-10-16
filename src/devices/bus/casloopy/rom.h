// license:BSD-3-Clause
// copyright-holders:Phil Bennett
#ifndef MAME_BUS_CASLOOPY_ROM_H
#define MAME_BUS_CASLOOPY_ROM_H

#pragma once

#include "slot.h"


// ======================> casloopy_rom_device

class casloopy_rom_device : public device_t,
							public device_casloopy_cart_interface
{
public:
	// construction/destruction
	casloopy_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_rom(offs_t offset) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, u8 data) override;

public:
	casloopy_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override { }
	virtual void device_reset() override { }
};


// ======================> casloopy_adpcm_device

class casloopy_adpcm_device : public casloopy_rom_device
{
public:
	// construction/destruction
	casloopy_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

// device type definition
DECLARE_DEVICE_TYPE(CASLOOPY_ROM_STD,   casloopy_rom_device)
DECLARE_DEVICE_TYPE(CASLOOPY_ROM_ADPCM, casloopy_adpcm_device)

#endif // MAME_BUS_CASLOOPY_ROM_H
