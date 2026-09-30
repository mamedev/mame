// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MCS 801 Color Printer emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MCS801_H
#define MAME_BUS_CBMIEC_MCS801_H

#pragma once

#include "cbmiec.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mcs801_device

class mcs801_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	mcs801_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MCS801, mcs801_device)


#endif // MAME_BUS_CBMIEC_MCS801_H
