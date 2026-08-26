// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Chip Level Design RapiDOS Professional

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_RAPIDOS_H
#define MAME_BUS_CBMIEC_RAPIDOS_H

#pragma once

#include "c1541.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_rapidos_professional_device

class c1541_rapidos_professional_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541_rapidos_professional_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_RAPIDOS_PROFESSIONAL, c1541_rapidos_professional_device)


#endif // MAME_BUS_CBMIEC_RAPIDOS_H
