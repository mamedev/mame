// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System digital joystick emulation with
    boostergrip adapter

**********************************************************************/

#pragma once

#ifndef __VCS_JOYSTICKBOOSTER__
#define __VCS_JOYSTICKBOOSTER__

#include "emu.h"
#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_joystick_booster_device

class vcs_joystick_booster_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	vcs_joystick_booster_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();

	// device_vcs_control_port_interface overrides
	virtual UINT8 vcs_joy_r();
	virtual UINT8 vcs_pot_x_r();
	virtual UINT8 vcs_pot_y_r();

	virtual bool has_pot_x() { return true; }
	virtual bool has_pot_y() { return true; }

private:
	required_ioport m_joy;
	required_ioport m_potx;
	required_ioport m_poty;
};


// device type definition
extern const device_type VCS_JOYSTICK_BOOSTER;


#endif
