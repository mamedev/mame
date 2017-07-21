// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System Driving Wheel emulation

**********************************************************************/

#ifndef MAME_BUS_A7800_CTRL_WHEEL_H
#define MAME_BUS_A7800_CTRL_WHEEL_H

#pragma once

#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a7800_wheel_device

class a7800_wheel_device : public device_t,
							public device_a7800_control_port_interface
{
public:
	// construction/destruction
	a7800_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_a7800_control_port_interface overrides
	virtual uint8_t a7800_joy_r() override;

private:
	required_ioport m_joy;
	required_ioport m_wheel;
};


// device type definition
DECLARE_DEVICE_TYPE(A7800_WHEEL, a7800_wheel_device)

#endif // MAME_BUS_A7800_CTRL_WHEEL_H
