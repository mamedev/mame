// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rossmöller TurboTrans

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_TURBOTRANS_H
#define MAME_BUS_CBMIEC_TURBOTRANS_H

#pragma once

#include "c1541.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_turbotrans_device

class c1541_turbotrans_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541_turbotrans_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_TURBOTRANS, c1541_turbotrans_device)


#endif // MAME_BUS_CBMIEC_TURBOTRANS_H
