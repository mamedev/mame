/**********************************************************************

    COMX-35 Serial/Parallel Printer Card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __COMX_PRN__
#define __COMX_PRN__


#include "emu.h"
#include "machine/comxexp.h"
#include "machine/comxpl80.h"
#include "machine/ctronics.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_prn_device

class comx_prn_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_prn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "comx_prn"; }

	// device_comx_expansion_card_interface overrides
	virtual UINT8 comx_mrd_r(address_space &space, offs_t offset, int *extrom);
	virtual UINT8 comx_io_r(address_space &space, offs_t offset);
	virtual void comx_io_w(address_space &space, offs_t offset, UINT8 data);

private:
	required_device<centronics_device> m_centronics;

	UINT8 *m_rom;               // program ROM
};


// device type definition
extern const device_type COMX_PRN;


#endif
