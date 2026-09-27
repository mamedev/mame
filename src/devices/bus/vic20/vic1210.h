// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1210 3K RAM Expansion Cartridge emulation
    Commodore VIC-1211A Super Expander with 3K RAM Cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_VIC1210_H
#define MAME_BUS_VIC20_VIC1210_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1210_device

class vic1210_device :  public device_t,
						public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic1210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC1210, vic1210_device)

#endif // MAME_BUS_VIC20_VIC1210_H
