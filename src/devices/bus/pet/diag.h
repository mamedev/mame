// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore PET User Port Diagnostic Connector emulation

**********************************************************************/

#ifndef MAME_BUS_PET_DIAG_H
#define MAME_BUS_PET_DIAG_H

#pragma once

#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_userport_diagnostic_connector_device

class pet_userport_diagnostic_connector_device : public device_t, public device_pet_user_port_interface
{
public:
	// construction/destruction
	pet_userport_diagnostic_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_pet_user_port_interface overrides
	virtual void input_2(int state) override { output_b(state); }
	virtual void input_3(int state) override { output_c(state); }
	virtual void input_4(int state) override { output_d(state); }
	virtual void input_6(int state) override { output_7(state); output_8(state); }
	virtual void input_7(int state) override { output_6(state); output_8(state); }
	virtual void input_8(int state) override { output_6(state); output_7(state); }
	virtual void input_9(int state) override { output_k(state); }
	virtual void input_10(int state) override { output_l(state); }
	virtual void input_b(int state) override { output_2(state); }
	virtual void input_c(int state) override { output_3(state); }
	virtual void input_d(int state) override { output_4(state); }
	virtual void input_k(int state) override { output_9(state); }
	virtual void input_l(int state) override { output_10(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(PET_USERPORT_DIAGNOSTIC_CONNECTOR, pet_userport_diagnostic_connector_device)

#endif // MAME_BUS_PET_DIAG_H
