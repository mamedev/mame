// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 standard cartridge emulation

**********************************************************************/

#pragma once

#ifndef __PLUS4_STANDARD_CARTRIDGE__
#define __PLUS4_STANDARD_CARTRIDGE__

#include "emu.h"
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
	plus4_standard_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_plus4_expansion_card_interface overrides
	virtual UINT8 plus4_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) override;


};


// device type definition
extern const device_type PLUS4_STD;


#endif
