// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Standard 2K/4K cartridge emulation

**********************************************************************/

#pragma once

#ifndef __VIDEOBRAIN_STANDARD_CARTRIDGE__
#define __VIDEOBRAIN_STANDARD_CARTRIDGE__

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> videobrain_standard_cartridge_device

class videobrain_standard_cartridge_device : public device_t,
												public device_videobrain_expansion_card_interface
{
public:
	// construction/destruction
	videobrain_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();

	// device_videobrain_expansion_card_interface overrides
	virtual UINT8 videobrain_bo_r(address_space &space, offs_t offset, int cs1, int cs2);
};


// device type definition
extern const device_type VB_STD;


#endif
