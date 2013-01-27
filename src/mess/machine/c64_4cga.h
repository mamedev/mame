/**********************************************************************

    Classical Games/Protovision 4 Player Interface emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C64_4CGA__
#define __C64_4CGA__


#include "emu.h"
#include "machine/c64user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_4cga_device

class c64_4cga_device : public device_t,
						public device_c64_user_port_interface
{
public:
	// construction/destruction
	c64_4cga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_4cga"; }
	virtual void device_start();

	// device_c64_user_port_interface overrides
	virtual UINT8 c64_pb_r(address_space &space, offs_t offset);
	virtual void c64_pb_w(address_space &space, offs_t offset, UINT8 data);

private:
	required_ioport m_fire;
	required_ioport m_joy3;
	required_ioport m_joy4;

	int m_port;
};


// device type definition
extern const device_type C64_4CGA;


#endif
