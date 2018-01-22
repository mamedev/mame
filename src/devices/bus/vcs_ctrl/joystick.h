// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System digital joystick emulation

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_JOYSTICK_H
#define MAME_BUS_VCS_CTRL_JOYSTICK_H

#pragma once

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
	vcs_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_vcs_control_port_interface overrides
	virtual uint8_t vcs_joy_r() override;

private:
	required_ioport m_joy;
};


// device type definition
DECLARE_DEVICE_TYPE(VCS_JOYSTICK, vcs_joystick_device)


#endif // MAME_BUS_VCS_CTRL_JOYSTICK_H
