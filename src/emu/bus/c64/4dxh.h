// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    The Digital Excess & Hitmen 4-Player Joystick adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C64_4DXH__
#define __C64_4DXH__


#include "emu.h"
#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_4dxh_device

class c64_4dxh_device : public device_t,
	public device_vic20_user_port_interface
{
public:
	// construction/destruction
	c64_4dxh_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
};


// device type definition
extern const device_type C64_4DXH;


#endif
