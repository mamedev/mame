// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Access Software cassette port dongle emulation

    Used in e.g. 10th Frame

**********************************************************************/

#ifndef MAME_BUS_PET_ACCESS_H
#define MAME_BUS_PET_ACCESS_H

#pragma once

#include "cass.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> access_dongle_device

class access_dongle_device : public device_t, public device_pet_datassette_port_interface
{
public:
	// construction/destruction
	access_dongle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_pet_datassette_port_interface overrides
	virtual int datassette_sense() override { return 0; }
};


// device type definition
DECLARE_DEVICE_TYPE(ACCESS_DONGLE, access_dongle_device)

#endif // MAME_BUS_PET_ACCESS_H
