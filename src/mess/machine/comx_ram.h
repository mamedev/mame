/**********************************************************************

    COMX-35 RAM Card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __COMX_RAM__
#define __COMX_RAM__


#include "emu.h"
#include "machine/comxexp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_ram_device

class comx_ram_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "comx_ram"; }

	// device_comx_expansion_card_interface overrides
	virtual UINT8 comx_mrd_r(address_space &space, offs_t offset, int *extrom);
	virtual void comx_mwr_w(address_space &space, offs_t offset, UINT8 data);
	virtual void comx_io_w(address_space &space, offs_t offset, UINT8 data);

private:
	optional_shared_ptr<UINT8> m_ram;

	int m_bank;
};


// device type definition
extern const device_type COMX_RAM;


#endif
