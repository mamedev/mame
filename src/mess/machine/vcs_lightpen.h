/**********************************************************************

    Atari Video Computer System lightpen emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __VCS_LIGHTPEN__
#define __VCS_LIGHTPEN__


#include "emu.h"
#include "machine/vcsctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_lightpen_device

class vcs_lightpen_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	vcs_lightpen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

	DECLARE_INPUT_CHANGED_MEMBER( trigger );

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "vcs_lightpen"; }
	virtual void device_start();

	// device_vcs_control_port_interface overrides
	virtual UINT8 vcs_joy_r();

private:
	required_ioport m_joy;
	required_ioport m_lightx;
	required_ioport m_lighty;
};


// device type definition
extern const device_type VCS_LIGHTPEN;


#endif
