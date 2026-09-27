// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-20 Standard 8K/16K ROM Cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_STD_H
#define MAME_BUS_VIC20_STD_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_standard_cartridge_device

class vic20_standard_cartridge_device :  public device_t,
											public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic20_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void install_rom(vic20_expansion_window &window, memory_region *region);
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_STD, vic20_standard_cartridge_device)

#endif // MAME_BUS_VIC20_STD_H
