// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __APF_ROM_H
#define __APF_ROM_H

#include "slot.h"


// ======================> apf_rom_device

class apf_rom_device : public device_t,
						public device_apf_cart_interface
{
public:
	// construction/destruction
	apf_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	apf_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> apf_basic_device

class apf_basic_device : public apf_rom_device
{
public:
	// construction/destruction
	apf_basic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t extra_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> apf_spacedst_device

class apf_spacedst_device : public apf_rom_device
{
public:
	// construction/destruction
	apf_spacedst_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};





// device type definition
extern const device_type APF_ROM_STD;
extern const device_type APF_ROM_BASIC;
extern const device_type APF_ROM_SPACEDST;


#endif
