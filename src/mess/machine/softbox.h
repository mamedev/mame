/**********************************************************************

    SSE SoftBox emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __SOFTBOX__
#define __SOFTBOX__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ieee488.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> softbox_device

class softbox_device :  public device_t,
						public device_ieee488_interface
{

public:
	// construction/destruction
	softbox_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "softbox"; }

	// device_ieee488_interface overrides
	void ieee488_atn(int state);
	void ieee488_ifc(int state);

private:
	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type SOFTBOX;



#endif
