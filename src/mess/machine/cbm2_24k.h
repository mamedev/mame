/**********************************************************************

    GLA 24K RAM cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __CBM2_24K_CARTRIDGE__
#define __CBM2_24K_CARTRIDGE__


#include "emu.h"
#include "machine/cbm2exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm2_24k_cartridge_device

class cbm2_24k_cartridge_device : public device_t,
								  public device_cbm2_expansion_card_interface
{
public:
	// construction/destruction
	cbm2_24k_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "cbm2_24k"; }
	virtual void device_start();

	// device_cbm2_expansion_card_interface overrides
	virtual UINT8 cbm2_bd_r(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3);
	virtual void cbm2_bd_w(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3);
};


// device type definition
extern const device_type CBM2_24K;


#endif
