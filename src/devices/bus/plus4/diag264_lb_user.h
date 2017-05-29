// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Diag264 User Port Loop Back Connector emulation

**********************************************************************/

#ifndef MAME_BUS_PLUS4_DIAG264_LB_USER_H
#define MAME_BUS_PLUS4_DIAG264_LB_USER_H

#pragma once

#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> diag264_user_port_loopback_device

class diag264_user_port_loopback_device : public device_t, public device_pet_user_port_interface
{
public:
	// construction/destruction
	diag264_user_port_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_pet_user_port_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER(input_b) override { output_6(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_k) override { output_7(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_4) override { output_j(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_5) override { output_f(state); }

	virtual DECLARE_WRITE_LINE_MEMBER(input_6) override { output_b(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_7) override { output_k(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_j) override { output_4(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_f) override { output_5(state); }

	virtual DECLARE_WRITE_LINE_MEMBER(input_c) override { output_m(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_d) override { output_l(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_e) override { output_h(state); }

	virtual DECLARE_WRITE_LINE_MEMBER(input_m) override { output_c(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_l) override { output_d(state); }
	virtual DECLARE_WRITE_LINE_MEMBER(input_h) override { output_e(state); }
};

// device type definition
DECLARE_DEVICE_TYPE(DIAG264_USER_PORT_LOOPBACK, diag264_user_port_loopback_device)

#endif // MAME_BUS_PLUS4_DIAG264_LB_USER_H
