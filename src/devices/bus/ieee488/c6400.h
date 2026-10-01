// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 6400 Printer emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_C6400_H
#define MAME_BUS_IEEE488_C6400_H

#pragma once

#include "ieee488.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c6400_device

class c6400_device : public device_t, public device_ieee488_interface
{
public:
	// construction/destruction
	c6400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(GPIB_C6400, c6400_device)


#endif // MAME_BUS_IEEE488_C6400_H
