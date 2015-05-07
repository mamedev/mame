// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Diag264 User Port Loop Back Connector emulation

**********************************************************************/

#pragma once

#ifndef __DIAG264_USER_PORT_LOOPBACK__
#define __DIAG264_USER_PORT_LOOPBACK__

#include "emu.h"
#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> diag264_user_port_loopback_device

class diag264_user_port_loopback_device :  public device_t,
	public device_pet_user_port_interface
{
public:
	// construction/destruction
	diag264_user_port_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();

	// device_pet_user_port_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER(input_b) { output_6(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_k) { output_7(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_4) { output_j(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_5) { output_f(state); }

	virtual DECLARE_WRITE_LINE_MEMBER(input_6) { output_b(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_7) { output_k(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_j) { output_4(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_f) { output_5(state); }

	virtual DECLARE_WRITE_LINE_MEMBER(input_c) { output_m(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_d) { output_l(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_e) { output_h(state); }

	virtual DECLARE_WRITE_LINE_MEMBER(input_m) { output_c(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_l) { output_d(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_h) { output_e(state); }
};

// device type definition
extern const device_type DIAG264_USER_PORT_LOOPBACK;

#endif
