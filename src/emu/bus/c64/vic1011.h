// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1011A/B RS-232C Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C64_VIC1011__
#define __C64_VIC1011__

#include "emu.h"
#include "user.h"
#include "machine/serial.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_vic1011_device

class c64_vic1011_device :  public device_t,
							public device_c64_user_port_interface
{
public:
	// construction/destruction
	c64_vic1011_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_WRITE_LINE_MEMBER( rxd_w );

protected:
	// device-level overrides
	virtual void device_start();

	// device_c64_user_port_interface overrides
	virtual UINT8 c64_pb_r(address_space &space, offs_t offset);
	virtual void c64_pb_w(address_space &space, offs_t offset, UINT8 data);
	virtual void c64_pa2_w(int state);

private:
	required_device<rs232_port_device> m_rs232;
};


// device type definition
extern const device_type C64_VIC1011;



#endif
