// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 standard cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_PLUS4_STD_H
#define MAME_BUS_PLUS4_STD_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> plus4_standard_cartridge_device

class plus4_standard_cartridge_device : public device_t,
										public device_plus4_expansion_card_interface
{
public:
	// construction/destruction
	plus4_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void install_rom(plus4_expansion_window &window, memory_region *region);
};


// device type definition
DECLARE_DEVICE_TYPE(PLUS4_STD, plus4_standard_cartridge_device)

#endif // MAME_BUS_PLUS4_STD_H
