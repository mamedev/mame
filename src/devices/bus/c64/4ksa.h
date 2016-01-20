// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Kingsoft 4-Player Adapter emulation

**********************************************************************/

#pragma once

#ifndef __C64_4KSA__
#define __C64_4KSA__


#include "emu.h"
#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_4ksa_device

class c64_4ksa_device : public device_t,
	public device_pet_user_port_interface
{
public:
	// construction/destruction
	c64_4ksa_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	// device_pet_user_port_interface overrides
	virtual WRITE_LINE_MEMBER( input_4 ) override { output_6(state); }
	virtual WRITE_LINE_MEMBER( input_6 ) override { output_4(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
};


// device type definition
extern const device_type C64_4KSA;


#endif
