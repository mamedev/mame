// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __VECTREX_ROM_H
#define __VECTREX_ROM_H

#include "slot.h"


// ======================> vectrex_rom_device

class vectrex_rom_device : public device_t,
						public device_vectrex_cart_interface
{
public:
	// construction/destruction
	vectrex_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	vectrex_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> vectrex_rom64k_device

class vectrex_rom64k_device : public vectrex_rom_device
{
public:
	// construction/destruction
	vectrex_rom64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	int m_bank;
};

// ======================> vectrex_sram_device

class vectrex_sram_device : public vectrex_rom_device
{
public:
	// construction/destruction
	vectrex_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};



// device type definition
extern const device_type VECTREX_ROM_STD;
extern const device_type VECTREX_ROM_64K;
extern const device_type VECTREX_ROM_SRAM;


#endif
