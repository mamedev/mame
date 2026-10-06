// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 4022 Printer emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_C4022_H
#define MAME_BUS_IEEE488_C4022_H

#pragma once

#include "ieee488.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c4022_device

class c4022_device : public device_t, public device_ieee488_interface
{
public:
	// construction/destruction
	c4022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(GPIB_C4022, c4022_device)


#endif // MAME_BUS_IEEE488_C4022_H
