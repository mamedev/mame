/**********************************************************************

    Coleco Adam Internal 64KB RAM Expansion emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __ADAM_RAM__
#define __ADAM_RAM__

#include "emu.h"
#include "machine/adamexp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_ram_expansion_device

class adam_ram_expansion_device :  public device_t,
								   public device_adam_expansion_slot_card_interface
{
public:
	// construction/destruction
	adam_ram_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "adam_ram"; }
	virtual void device_start();

	// device_adam_expansion_slot_card_interface overrides
	virtual UINT8 adam_bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2);
	virtual void adam_bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2);
};


// device type definition
extern const device_type ADAM_RAM;



#endif
