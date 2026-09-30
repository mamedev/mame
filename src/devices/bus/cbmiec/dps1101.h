// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore DPS 1101 Daisy Wheel Printer emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_DPS1101_H
#define MAME_BUS_CBMIEC_DPS1101_H

#pragma once

#include "cbmiec.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dps1101_device

class dps1101_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	dps1101_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(DPS1101, dps1101_device)


#endif // MAME_BUS_CBMIEC_DPS1101_H
