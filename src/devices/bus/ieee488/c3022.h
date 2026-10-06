// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 3022 Printer emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_C3022_H
#define MAME_BUS_IEEE488_C3022_H

#pragma once

#include "ieee488.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c3022_device

class c3022_device : public device_t, public device_ieee488_interface
{
public:
	// construction/destruction
	c3022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(GPIB_C3022, c3022_device)


#endif // MAME_BUS_IEEE488_C3022_H
