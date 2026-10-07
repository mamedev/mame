// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1525 Graphic Printer emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_VIC1525_H
#define MAME_BUS_CBMIEC_VIC1525_H

#pragma once

#include "cbmiec.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1525_device

class vic1525_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	vic1525_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC1525, vic1525_device)


#endif // MAME_BUS_CBMIEC_VIC1525_H
