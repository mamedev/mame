// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Diag264 Serial Loop Back Connector emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_DIAG264_LB_IEC_H
#define MAME_BUS_CBMIEC_DIAG264_LB_IEC_H

#pragma once

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
	diag264_serial_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_cbm_iec_interface overrides
	virtual void cbm_iec_atn(int state) override;
};


// device type definition
DECLARE_DEVICE_TYPE(DIAG264_SERIAL_LOOPBACK, diag264_serial_loopback_device)


#endif // MAME_BUS_CBMIEC_DIAG264_LB_IEC_H
