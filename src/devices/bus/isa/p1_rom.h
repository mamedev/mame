// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 ROM cartridge device

**********************************************************************/

#ifndef MAME_BUS_ISA_P1_ROM_H
#define MAME_BUS_ISA_P1_ROM_H

#pragma once

#include "isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class p1_rom_device : public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	p1_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(P1_ROM, p1_rom_device)


#endif // MAME_BUS_ISA_P1_ROM_H
