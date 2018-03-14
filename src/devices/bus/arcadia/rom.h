// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_ARCADIA_ROM_H
#define MAME_BUS_ARCADIA_ROM_H

#pragma once

#include "slot.h"


// ======================> arcadia_rom_device

class arcadia_rom_device : public device_t,
						public device_arcadia_cart_interface
{
public:
	// construction/destruction
	arcadia_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
	virtual DECLARE_READ8_MEMBER(extra_rom) override;

public:
	arcadia_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> arcadia_golf_device

class arcadia_golf_device : public arcadia_rom_device
{
public:
	// construction/destruction
	arcadia_golf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};




// device type definition
DECLARE_DEVICE_TYPE(ARCADIA_ROM_STD,  arcadia_rom_device)
DECLARE_DEVICE_TYPE(ARCADIA_ROM_GOLF, arcadia_golf_device)

#endif // MAME_BUS_ARCADIA_ROM_H
