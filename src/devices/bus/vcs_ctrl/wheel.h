// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System Driving Wheel emulation

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_WHEEL_H
#define MAME_BUS_VCS_CTRL_WHEEL_H

#pragma once

#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_wheel_device

class vcs_wheel_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	vcs_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_vcs_control_port_interface overrides
	virtual uint8_t vcs_joy_r() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_joy;
	required_ioport m_wheel;
};


// device type definition
DECLARE_DEVICE_TYPE(VCS_WHEEL, vcs_wheel_device)

#endif // MAME_BUS_VCS_CTRL_WHEEL_H
