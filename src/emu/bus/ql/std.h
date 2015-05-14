// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL standard ROM cartridge emulation

**********************************************************************/

#pragma once

#ifndef __QL_STANDARD_ROM_CARTRIDGE__
#define __QL_STANDARD_ROM_CARTRIDGE__

#include "rom.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ql_standard_rom_cartridge_t

class ql_standard_rom_cartridge_t : public device_t,
									public device_ql_rom_cartridge_card_interface
{
public:
	// construction/destruction
	ql_standard_rom_cartridge_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();

	// device_ql_rom_cartridge_card_interface overrides
	virtual UINT8 read(address_space &space, offs_t offset, UINT8 data);
};


// device type definition
extern const device_type QL_STANDARD_ROM_CARTRIDGE;


#endif
