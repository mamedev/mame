// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II Standard cartridge emulation

**********************************************************************/

#pragma once

#ifndef __CBM2_STANDARD_CARTRIDGE__
#define __CBM2_STANDARD_CARTRIDGE__

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm2_standard_cartridge_device

class cbm2_standard_cartridge_device : public device_t,
										public device_cbm2_expansion_card_interface
{
public:
	// construction/destruction
	cbm2_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();

	// device_cbm2_expansion_card_interface overrides
	virtual UINT8 cbm2_bd_r(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3);
};


// device type definition
extern const device_type CBM2_STD;


#endif
