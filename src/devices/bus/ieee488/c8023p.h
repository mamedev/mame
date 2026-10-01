// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

	Commodore 8023P/MPP-1361 Printer emulation

**********************************************************************/

#ifndef MAME_BUS_IEEE488_C8023P_H
#define MAME_BUS_IEEE488_C8023P_H

#pragma once

#include "ieee488.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c8023p_device

class c8023p_device : public device_t, public device_ieee488_interface
{
public:
	// construction/destruction
	c8023p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(GPIB_C8023P, c8023p_device)


#endif // MAME_BUS_IEEE488_C8023P_H
