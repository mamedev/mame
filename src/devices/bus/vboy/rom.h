// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __VBOY_ROM_H
#define __VBOY_ROM_H

#include "slot.h"


// ======================> vboy_rom_device

class vboy_rom_device : public device_t,
						public device_vboy_cart_interface
{
public:
	// construction/destruction
	vboy_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	vboy_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}

	// reading and writing
	virtual uint32_t read_cart(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff) override;
};

// ======================> vboy_eeprom_device

class vboy_eeprom_device : public vboy_rom_device
{
public:
	// construction/destruction
	vboy_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint32_t read_eeprom(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff) override;
	virtual void write_eeprom(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff) override;
};



// device type definition
extern const device_type VBOY_ROM_STD;
extern const device_type VBOY_ROM_EEPROM;



#endif
