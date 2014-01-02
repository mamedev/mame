// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1011A/B RS-232C Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __VIC1011__
#define __VIC1011__

#include "emu.h"
#include "user.h"
#include "machine/serial.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1011_device

class vic1011_device :  public device_t,
						public device_vic20_user_port_interface
{
public:
	// construction/destruction
	vic1011_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_WRITE_LINE_MEMBER( write_rxd );
	DECLARE_WRITE_LINE_MEMBER( write_d );
	DECLARE_WRITE_LINE_MEMBER( write_e );
	DECLARE_WRITE_LINE_MEMBER( write_dcdin );
	DECLARE_WRITE_LINE_MEMBER( write_j );
	DECLARE_WRITE_LINE_MEMBER( write_cts );
	DECLARE_WRITE_LINE_MEMBER( write_dsr );
	DECLARE_WRITE_LINE_MEMBER( write_m );

protected:
	// device-level overrides
	virtual void device_start();

private:
	required_device<rs232_port_device> m_rs232;
};


// device type definition
extern const device_type VIC1011;



#endif
