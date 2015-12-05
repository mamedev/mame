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
	arcadia_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	arcadia_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_READ8_MEMBER(extra_rom);
};

// ======================> arcadia_golf_device

class arcadia_golf_device : public arcadia_rom_device
{
public:
	// construction/destruction
	arcadia_golf_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};




// device type definition
extern const device_type ARCADIA_ROM_STD;
extern const device_type ARCADIA_ROM_GOLF;

#endif
