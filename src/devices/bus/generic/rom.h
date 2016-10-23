// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __GENERIC_ROM_H
#define __GENERIC_ROM_H

#include "slot.h"


// ======================> generic_rom_device

class generic_rom_device : public device_t,
						public device_generic_cart_interface
{
public:
	// construction/destruction
	generic_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override {}
};


// ======================> generic_rom_plain_device

class generic_rom_plain_device : public generic_rom_device
{
public:
	// construction/destruction
	generic_rom_plain_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	generic_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint16_t read16_rom(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual uint32_t read32_rom(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff) override;
};


// ======================> generic_romram_plain_device

class generic_romram_plain_device : public generic_rom_plain_device
{
public:
	// construction/destruction
	generic_romram_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};


// ======================> generic_rom_linear_device

class generic_rom_linear_device : public generic_rom_device
{
public:
	// construction/destruction
	generic_rom_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint16_t read16_rom(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual uint32_t read32_rom(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff) override;
};



// device type definition
extern const device_type GENERIC_ROM_PLAIN;
extern const device_type GENERIC_ROM_LINEAR;
extern const device_type GENERIC_ROMRAM_PLAIN;


#endif
