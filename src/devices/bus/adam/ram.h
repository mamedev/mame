// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Internal 64KB RAM Expansion emulation

**********************************************************************/

#pragma once

#ifndef __ADAM_RAM__
#define __ADAM_RAM__

#include "emu.h"
#include "exp.h"



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
	virtual void device_start();

	// device_adam_expansion_slot_card_interface overrides
	virtual UINT8 adam_bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2);
	virtual void adam_bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2);

private:
	optional_shared_ptr<UINT8> m_ram;
};


// device type definition
extern const device_type ADAM_RAM;



#endif
