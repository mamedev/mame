/**********************************************************************

    Diag264 Serial Loop Back Connector emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __DIAG264_SERIAL_LOOPBACK__
#define __DIAG264_SERIAL_LOOPBACK__


#include "emu.h"
#include "machine/cbmiec.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> diag264_serial_loopback_device

class diag264_serial_loopback_device :  public device_t,
										public device_cbm_iec_interface
{
public:
	// construction/destruction
	diag264_serial_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "diag264_serial_loopback"; }
	virtual void device_start();

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_atn(int state);
};


// device type definition
extern const device_type DIAG264_SERIAL_LOOPBACK;



#endif
