// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II Standard cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_CBM2_STD_H
#define MAME_BUS_CBM2_STD_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm2_standard_cartridge_device

class cbm2_standard_cartridge_device : public device_t,
										public device_cbm2_expansion_card_interface
{
public:
	// construction/destruction
	cbm2_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(CBM2_STD, cbm2_standard_cartridge_device)


#endif // MAME_BUS_CBM2_STD_H
