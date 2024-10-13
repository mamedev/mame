// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_ASTROCADE_ROM_H
#define MAME_BUS_ASTROCADE_ROM_H

#pragma once

#include "slot.h"
#include "imagedev/cassette.h"


// ======================> astrocade_rom_device

class astrocade_rom_device : public device_t,
						public device_astrocade_cart_interface
{
public:
	// construction/destruction
	astrocade_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;

protected:
	astrocade_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> astrocade_rom_256k_device

class astrocade_rom_256k_device : public astrocade_rom_device
{
public:
	// construction/destruction
	astrocade_rom_256k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;

private:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_base_bank;
};

// ======================> astrocade_rom_512k_device

class astrocade_rom_512k_device : public astrocade_rom_device
{
public:
	// construction/destruction
	astrocade_rom_512k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;

private:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_base_bank;
};

// ======================> astrocade_rom_cass_device

class astrocade_rom_cass_device : public astrocade_rom_device
{
public:
	// construction/destruction
	astrocade_rom_cass_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;

private:
	virtual void device_start() override { }
	virtual void device_reset() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cassette_image_device> m_cassette;
};


// device type definition
DECLARE_DEVICE_TYPE(ASTROCADE_ROM_STD,  astrocade_rom_device)
DECLARE_DEVICE_TYPE(ASTROCADE_ROM_256K, astrocade_rom_256k_device)
DECLARE_DEVICE_TYPE(ASTROCADE_ROM_512K, astrocade_rom_512k_device)
DECLARE_DEVICE_TYPE(ASTROCADE_ROM_CASS, astrocade_rom_cass_device)

#endif // MAME_BUS_ASTROCADE_ROM_H
