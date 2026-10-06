// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_LEAPSTER_ROM_H
#define MAME_BUS_LEAPSTER_ROM_H

#pragma once

#include "slot.h"
#include "machine/nvram.h"

// ======================> leapster_rom_plain_device

class leapster_rom_plain_device : public device_t,
						public device_leapster_interface
{
public:
	// construction/destruction
	leapster_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint16_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint16_t data) override;

	virtual uint8_t read_nvram(uint16_t offset) override { return 0xff; }
	virtual void write_nvram(uint16_t offset, uint8_t data) override { }

	virtual uint16_t read_rom(offs_t offset);
	virtual void write_rom(offs_t offset, uint16_t data);

protected:
	leapster_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> leapster_rom_nvram_device

class leapster_rom_nvram_device : public leapster_rom_plain_device
{
public:
	// construction/destruction
	leapster_rom_nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	leapster_rom_nvram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override;

	// reading and writing
	virtual uint16_t read_rom(offs_t offset) override;
	virtual void write_rom(offs_t offset, uint16_t data) override;

	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_cartridge_eeprom;

	virtual uint8_t read_nvram(uint16_t offset) override;
	virtual void write_nvram(uint16_t offset, uint8_t data) override;

private:
	static constexpr int NVRAM_SIZE = 0x800;

};

// device type definition
DECLARE_DEVICE_TYPE(LEAPSTER_ROM_PLAIN,       leapster_rom_plain_device)
DECLARE_DEVICE_TYPE(LEAPSTER_ROM_NVRAM,    leapster_rom_nvram_device)

#endif // MAME_BUS_LEAPSTER_ROM_H
