// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __O2_ROM_H
#define __O2_ROM_H

#include "slot.h"


// ======================> o2_rom_device

class o2_rom_device : public device_t,
						public device_o2_cart_interface
{
public:
	// construction/destruction
	o2_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	o2_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_rom04(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_rom0c(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

	virtual void write_bank(int bank) override;

protected:
	int m_bank_base;
};

// ======================> o2_rom12_device

class o2_rom12_device : public o2_rom_device
{
public:
	// construction/destruction
	o2_rom12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom04(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_rom0c(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};

// ======================> o2_rom16_device

class o2_rom16_device : public o2_rom_device
{
public:
	// construction/destruction
	o2_rom16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom04(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_rom0c(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};



// device type definition
extern const device_type O2_ROM_STD;
extern const device_type O2_ROM_12K;
extern const device_type O2_ROM_16K;


#endif
