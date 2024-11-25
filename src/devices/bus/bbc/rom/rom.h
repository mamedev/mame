// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro Sideways ROM emulation

***************************************************************************/

#ifndef MAME_BUS_BBC_ROM_ROM_H
#define MAME_BUS_BBC_ROM_ROM_H

#pragma once

#include "slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_rom_device

class bbc_rom_device : public device_t,
							public device_bbc_rom_interface
{
public:
	// construction/destruction
	bbc_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_ROM, bbc_rom_device)


#endif // MAME_BUS_BBC_ROM_ROM_H
