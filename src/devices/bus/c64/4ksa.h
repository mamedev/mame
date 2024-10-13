// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Kingsoft 4-Player Adapter emulation

**********************************************************************/

#ifndef MAME_BUS_C64_4KSA_H
#define MAME_BUS_C64_4KSA_H

#pragma once


#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_4ksa_device

class c64_4ksa_device : public device_t, public device_pet_user_port_interface
{
public:
	// construction/destruction
	c64_4ksa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_pet_user_port_interface overrides
	virtual void input_4(int state) override { output_6(state); }
	virtual void input_6(int state) override { output_4(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_4KSA, c64_4ksa_device)


#endif // MAME_BUS_C64_4KSA_H
