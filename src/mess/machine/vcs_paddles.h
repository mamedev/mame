/**********************************************************************

    Atari Video Computer System analog paddles emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __VCS_PADDLES__
#define __VCS_PADDLES__


#include "emu.h"
#include "machine/vcsctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_paddles_device

class vcs_paddles_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	vcs_paddles_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "vcs_paddles"; }
	virtual void device_start();

	// device_vcs_control_port_interface overrides
	virtual UINT8 vcs_joy_r();
	virtual UINT8 vcs_pot_x_r();
	virtual UINT8 vcs_pot_y_r();
};


// device type definition
extern const device_type VCS_PADDLES;


#endif
