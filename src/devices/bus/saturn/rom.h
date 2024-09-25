// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SATURN_ROM_H
#define MAME_BUS_SATURN_ROM_H

#include "sat_slot.h"


// ======================> saturn_rom_device

class saturn_rom_device : public device_t, public device_sat_cart_interface
{
public:
	// construction/destruction
	saturn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint32_t read_rom(offs_t offset) override;

protected:
	saturn_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cart_type);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(SATURN_ROM, saturn_rom_device)

#endif // MAME_BUS_SATURN_ROM_H
