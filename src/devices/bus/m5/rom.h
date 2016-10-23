// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __M5_ROM_H
#define __M5_ROM_H

#include "slot.h"


// ======================> m5_rom_device

class m5_rom_device : public device_t,
						public device_m5_cart_interface
{
public:
	// construction/destruction
	m5_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	m5_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> m5_ram_device

class m5_ram_device : public m5_rom_device
{
public:
	// construction/destruction
	m5_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};


// device type definition
extern const device_type M5_ROM_STD;
extern const device_type M5_ROM_RAM;


#endif
