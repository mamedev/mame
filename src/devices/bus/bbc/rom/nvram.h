// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro Sideways RAM (Battery Backup) emulation

***************************************************************************/

#ifndef MAME_BUS_BBC_ROM_NVRAM_H
#define MAME_BUS_BBC_ROM_NVRAM_H

#pragma once

#include "slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_nvram_device

class bbc_nvram_device : public device_t,
							public device_bbc_rom_interface
{
public:
	// construction/destruction
	bbc_nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_NVRAM, bbc_nvram_device)

#endif // MAME_BUS_BBC_ROM_NVRAM_H
