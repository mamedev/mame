// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_BUS_VSMILE_ROM_H
#define MAME_BUS_VSMILE_ROM_H

#pragma once

#include "vsmile_slot.h"

// ======================> vsmile_rom_device

class vsmile_rom_device : public device_t, public device_vsmile_cart_interface
{
public:
	// construction/destruction
	vsmile_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t bank0_r(offs_t offset) override { return m_rom[m_bank_offset + 0x000000 + offset]; }
	virtual uint16_t bank1_r(offs_t offset) override { return m_rom[m_bank_offset + 0x100000 + offset]; }
	virtual uint16_t bank2_r(offs_t offset) override { return m_rom[m_bank_offset + 0x200000 + offset]; }
	virtual uint16_t bank3_r(offs_t offset) override { return m_rom[m_bank_offset + 0x300000 + offset]; }

	// banking
	virtual void set_cs2(bool cs2) override;

protected:
	vsmile_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

	uint32_t m_bank_offset;
};


// ======================> vsmile_rom_nvram_device

class vsmile_rom_nvram_device : public vsmile_rom_device
{
public:
	// construction/destruction
	vsmile_rom_nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t bank2_r(offs_t offset) override;
	virtual void bank2_w(offs_t offset, uint16_t data) override;

protected:
	vsmile_rom_nvram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(VSMILE_ROM_STD, vsmile_rom_device)
DECLARE_DEVICE_TYPE(VSMILE_ROM_NVRAM, vsmile_rom_nvram_device)


#endif // MAME_BUS_VSMILE_ROM_H
