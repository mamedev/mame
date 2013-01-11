/**********************************************************************

    COMX-35 F&M EPROM Card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __COMX_EPR__
#define __COMX_EPR__


#include "emu.h"
#include "machine/comxexp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_epr_device

class comx_epr_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_epr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "comx_epr"; }

	// device_comx_expansion_card_interface overrides
	virtual UINT8 comx_mrd_r(address_space &space, offs_t offset, int *extrom);
	virtual void comx_io_w(address_space &space, offs_t offset, UINT8 data);

private:
	UINT8 m_select;
	UINT8 *m_rom;               // program ROM
	UINT8 *m_eprom;             // EPROMs
};


// device type definition
extern const device_type COMX_EPR;


#endif
