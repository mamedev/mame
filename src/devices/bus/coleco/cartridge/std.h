// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision standard cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_COLECO_STD_H
#define MAME_BUS_COLECO_STD_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> colecovision_standard_cartridge_device

class colecovision_standard_cartridge_device : public device_t,
												public device_colecovision_cartridge_interface
{
public:
	// construction/destruction
	colecovision_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_colecovision_expansion_card_interface overrides
	virtual uint8_t read(offs_t offset, int _8000, int _a000, int _c000, int _e000) override;
};


// device type definition
DECLARE_DEVICE_TYPE(COLECOVISION_STANDARD, colecovision_standard_cartridge_device)


#endif // MAME_BUS_COLECO_STD_H
