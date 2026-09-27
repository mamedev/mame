// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore MAX BASIC cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC10_BASIC_H
#define MAME_BUS_VIC10_BASIC_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic10_basic_cartridge_device

class vic10_basic_cartridge_device : public device_t, public device_vic10_expansion_card_interface
{
public:
	// construction/destruction
	vic10_basic_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC10_BASIC, vic10_basic_cartridge_device)

#endif // MAME_BUS_VIC10_BASIC_H
