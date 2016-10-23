// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __VC4000_ROM_H
#define __VC4000_ROM_H

#include "slot.h"


// ======================> vc4000_rom_device

class vc4000_rom_device : public device_t,
						public device_vc4000_cart_interface
{
public:
	// construction/destruction
	vc4000_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	vc4000_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> vc4000_rom4k_device

class vc4000_rom4k_device : public vc4000_rom_device
{
public:
	// construction/destruction
	vc4000_rom4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> vc4000_ram1k_device

class vc4000_ram1k_device : public vc4000_rom_device
{
public:
	// construction/destruction
	vc4000_ram1k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};

// ======================> vc4000_chess2_device

class vc4000_chess2_device : public vc4000_rom_device
{
public:
	// construction/destruction
	vc4000_chess2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t extra_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};





// device type definition
extern const device_type VC4000_ROM_STD;
extern const device_type VC4000_ROM_ROM4K;
extern const device_type VC4000_ROM_RAM1K;
extern const device_type VC4000_ROM_CHESS2;


#endif
