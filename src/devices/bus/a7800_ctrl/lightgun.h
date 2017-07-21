// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari lightgun emulation

**********************************************************************/

#ifndef MAME_BUS_A7800_CTRL_LIGHTGUN_H
#define MAME_BUS_A7800_CTRL_LIGHTGUN_H

#pragma once

#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a7800_lightgun_device

class a7800_lightgun_device : public device_t,
							public device_a7800_control_port_interface
{
public:
	// construction/destruction
	a7800_lightgun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_a7800_control_port_interface overrides
	virtual uint8_t a7800_joy_r() override;
	virtual uint8_t a7800_light_x_r() override;
	virtual uint8_t a7800_light_y_r() override;
	virtual bool is_lightgun() override { return true; }

private:
	required_ioport m_joy;
	required_ioport m_lightx;
	required_ioport m_lighty;
};


// device type definition
DECLARE_DEVICE_TYPE(A7800_LIGHTGUN, a7800_lightgun_device)

#endif // MAME_BUS_A7800_CTRL_LIGHTGUN_H
