/**********************************************************************

    Serial Box 64K Serial Port Buffer emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __SERIAL_BOX__
#define __SERIAL_BOX__


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/cbmiec.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define SERIAL_BOX_TAG			"serialbox"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> serial_box_device

class serial_box_device :  public device_t,
						   public device_cbm_iec_interface
{

public:
    // construction/destruction
    serial_box_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete() { m_shortname = "serbox"; }

	// device_cbm_iec_interface overrides
	void cbm_iec_atn(int state);
	void cbm_iec_data(int state);
	void cbm_iec_reset(int state);

private:
	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type SERIAL_BOX;



#endif
