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
	arcadia_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	arcadia_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
	virtual DECLARE_READ8_MEMBER(extra_rom) override;
};

// ======================> arcadia_golf_device

class arcadia_golf_device : public arcadia_rom_device
{
public:
	// construction/destruction
	arcadia_golf_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};




// device type definition
extern const device_type ARCADIA_ROM_STD;
extern const device_type ARCADIA_ROM_GOLF;

#endif
