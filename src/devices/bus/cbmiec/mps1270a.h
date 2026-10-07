// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MPS-1270A Inkjet Printer emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MPS1270A_H
#define MAME_BUS_CBMIEC_MPS1270A_H

#pragma once

#include "cbmiec.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mps1270a_device

class mps1270a_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	mps1270a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MPS1270A, mps1270a_device)


#endif // MAME_BUS_CBMIEC_MPS1270A_H
