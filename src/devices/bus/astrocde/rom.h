// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __ASTROCADE_ROM_H
#define __ASTROCADE_ROM_H

#include "slot.h"


// ======================> astrocade_rom_device

class astrocade_rom_device : public device_t,
						public device_astrocade_cart_interface
{
public:
	// construction/destruction
	astrocade_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	astrocade_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> astrocade_rom_256k_device

class astrocade_rom_256k_device : public astrocade_rom_device
{
public:
	// construction/destruction
	astrocade_rom_256k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

private:
	uint8_t m_base_bank;
};

// ======================> astrocade_rom_512k_device

class astrocade_rom_512k_device : public astrocade_rom_device
{
public:
	// construction/destruction
	astrocade_rom_512k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

private:
	uint8_t m_base_bank;
};





// device type definition
extern const device_type ASTROCADE_ROM_STD;
extern const device_type ASTROCADE_ROM_256K;
extern const device_type ASTROCADE_ROM_512K;


#endif
