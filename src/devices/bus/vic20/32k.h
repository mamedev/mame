// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    32K RAM Expansion Cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_32K_H
#define MAME_BUS_VIC20_32K_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_32k_device

class vic20_32k_device :  public device_t, public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic20_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_32K, vic20_32k_device)

#endif // MAME_BUS_VIC20_32K_H
