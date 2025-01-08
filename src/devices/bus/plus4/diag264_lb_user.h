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
	virtual void device_start() override ATTR_COLD;

	// device_pet_user_port_interface overrides
	virtual void input_b(int state) override { output_6(state); }
	virtual void input_k(int state) override { output_7(state); }
	virtual void input_4(int state) override { output_j(state); }
	virtual void input_5(int state) override { output_f(state); }

	virtual void input_6(int state) override { output_b(state); }
	virtual void input_7(int state) override { output_k(state); }
	virtual void input_j(int state) override { output_4(state); }
	virtual void input_f(int state) override { output_5(state); }

	virtual void input_c(int state) override { output_m(state); }
	virtual void input_d(int state) override { output_l(state); }
	virtual void input_e(int state) override { output_h(state); }

	virtual void input_m(int state) override { output_c(state); }
	virtual void input_l(int state) override { output_d(state); }
	virtual void input_h(int state) override { output_e(state); }
};

// device type definition
DECLARE_DEVICE_TYPE(DIAG264_USER_PORT_LOOPBACK, diag264_user_port_loopback_device)

#endif // MAME_BUS_PLUS4_DIAG264_LB_USER_H
