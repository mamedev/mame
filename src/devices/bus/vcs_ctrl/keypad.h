// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System Keypad emulation

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_KEYPAD_H
#define MAME_BUS_VCS_CTRL_KEYPAD_H

#pragma once

#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_keypad_device

class vcs_keypad_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	vcs_keypad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_vcs_control_port_interface overrides
	virtual uint8_t vcs_joy_r() override;
	virtual void vcs_joy_w( uint8_t data ) override;
	virtual uint8_t vcs_pot_x_r() override;
	virtual uint8_t vcs_pot_y_r() override;

	virtual bool has_pot_x() override { return true; }
	virtual bool has_pot_y() override { return true; }

private:
	required_ioport m_keypad;

	uint8_t   m_column;
};


// device type definition
DECLARE_DEVICE_TYPE(VCS_KEYPAD, vcs_keypad_device)

#endif // MAME_BUS_VCS_CTRL_KEYPAD_H
