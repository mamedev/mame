// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __ARCADIA_ROM_H
#define __ARCADIA_ROM_H

#include "slot.h"


// ======================> arcadia_rom_device

class arcadia_rom_device : public device_t,
						public device_arcadia_cart_interface
{
public:
	// construction/destruction
	arcadia_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	arcadia_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t extra_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> arcadia_golf_device

class arcadia_golf_device : public arcadia_rom_device
{
public:
	// construction/destruction
	arcadia_golf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};




// device type definition
extern const device_type ARCADIA_ROM_STD;
extern const device_type ARCADIA_ROM_GOLF;

#endif
