// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System digital joystick emulation

**********************************************************************/

#pragma once

#ifndef __VCS_JOYSTICK__
#define __VCS_JOYSTICK__

#include "emu.h"
#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_joystick_device

class vcs_joystick_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	vcs_joystick_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_vcs_control_port_interface overrides
	virtual UINT8 vcs_joy_r() override;

private:
	required_ioport m_joy;
};


// device type definition
extern const device_type VCS_JOYSTICK;


#endif
