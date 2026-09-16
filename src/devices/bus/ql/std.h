// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL standard ROM cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_QL_STD_H
#define MAME_BUS_QL_STD_H

#pragma once

#include "rom.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ql_standard_rom_cartridge_device

class ql_standard_rom_cartridge_device : public device_t, public device_ql_rom_cartridge_card_interface
{
public:
	// construction/destruction
	ql_standard_rom_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_ql_rom_cartridge_card_interface overrides
	virtual uint8_t read(offs_t offset, uint8_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(QL_STANDARD_ROM_CARTRIDGE, ql_standard_rom_cartridge_device)

#endif // MAME_BUS_QL_STD_H
