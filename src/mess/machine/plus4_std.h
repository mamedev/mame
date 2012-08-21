/**********************************************************************

    Commodore Plus/4 standard cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __PLUS4_STANDARD_CARTRIDGE__
#define __PLUS4_STANDARD_CARTRIDGE__


#include "emu.h"
#include "imagedev/cartslot.h"
#include "machine/plus4exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> plus4_standard_cartridge_device

class plus4_standard_cartridge_device : public device_t,
										public device_plus4_expansion_card_interface
{
public:
	// construction/destruction
	plus4_standard_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "plus4_standard"; }
	virtual void device_start();

	// device_plus4_expansion_card_interface overrides
	virtual UINT8 plus4_cd_r(address_space &space, offs_t offset, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h);
};


// device type definition
extern const device_type PLUS4_STD;


#endif
