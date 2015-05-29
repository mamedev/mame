// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Diag264 Serial Loop Back Connector emulation

**********************************************************************/

#pragma once

#ifndef __DIAG264_IEC_LOOPBACK__
#define __DIAG264_IEC_LOOPBACK__

#include "emu.h"
#include "cbmiec.h"



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
	virtual void device_start();

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_atn(int state);
};


// device type definition
extern const device_type DIAG264_SERIAL_LOOPBACK;



#endif
