// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-2020 Printer emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MPS2020_H
#define MAME_BUS_CBMIEC_MPS2020_H

#pragma once

#include "cbmiec.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mps2020_device

class mps2020_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	mps2020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MPS2020, mps2020_device)


#endif // MAME_BUS_CBMIEC_MPS2020_H
