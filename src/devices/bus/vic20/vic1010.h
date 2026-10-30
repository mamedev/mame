// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1010 Expansion Module emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_VIC1010_H
#define MAME_BUS_VIC20_VIC1010_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1010_device

class vic1010_device :  public device_t,
						public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic1010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<vic20_expansion_slot_device, 6> m_expansion_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC1010, vic1010_device)

#endif // MAME_BUS_VIC20_VIC1010_H
