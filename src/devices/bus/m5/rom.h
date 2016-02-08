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
	m5_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	m5_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
};

// ======================> m5_ram_device

class m5_ram_device : public m5_rom_device
{
public:
	// construction/destruction
	m5_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram) override;
	virtual DECLARE_WRITE8_MEMBER(write_ram) override;
};


// device type definition
extern const device_type M5_ROM_STD;
extern const device_type M5_ROM_RAM;


#endif
