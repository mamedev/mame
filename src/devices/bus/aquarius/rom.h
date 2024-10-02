// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Aquarius Software Cartridges

***************************************************************************/

#ifndef MAME_BUS_AQUARIUS_ROM_H
#define MAME_BUS_AQUARIUS_ROM_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aquarius_rom_device

class aquarius_rom_device :
	public device_t,
	public device_aquarius_cartridge_interface
{
public:
	// construction/destruction
	aquarius_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_aquarius_cartridge_interface overrides
	virtual uint8_t mreq_ce_r(offs_t offset) override;
};


// device type definition
DECLARE_DEVICE_TYPE(AQUARIUS_ROM, aquarius_rom_device)


#endif // MAME_BUS_AQUARIUS_ROM_H
