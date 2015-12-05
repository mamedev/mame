// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SAT_ROM_H
#define __SAT_ROM_H

#include "sat_slot.h"


// ======================> saturn_rom_device

class saturn_rom_device : public device_t,
						public device_sat_cart_interface
{
public:
	// construction/destruction
	saturn_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	saturn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_rom);
};



// device type definition
extern const device_type SATURN_ROM;


#endif
