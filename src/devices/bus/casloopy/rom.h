// license:BSD-3-Clause
// copyright-holders:Vas Crabb
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
	casloopy_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// load/unload
	virtual std::error_condition load() override;
	virtual void unload() override;

	// read/write
	virtual u16 rom_r(offs_t offset) override;
	virtual u8 ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, u8 data) override;

protected:
	casloopy_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

private:
	const u16 *m_rom_base;
	u8 *m_nvram_base;
	u32 m_rom_size;
	u32 m_nvram_size;
};


// ======================> casloopy_adpcm_device

class casloopy_adpcm_device : public casloopy_rom_device
{
public:
	// construction/destruction
	casloopy_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(CASLOOPY_ROM_STD,   casloopy_rom_device)
DECLARE_DEVICE_TYPE(CASLOOPY_ROM_ADPCM, casloopy_adpcm_device)

#endif // MAME_BUS_CASLOOPY_ROM_H
