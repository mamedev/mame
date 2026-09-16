// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Atari CX85 Numeric Keypad

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_CX85_H
#define MAME_BUS_VCS_CTRL_CX85_H

#pragma once

#include "ctrl.h"
#include "machine/mm74c922.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> atari_cx85_device

class atari_cx85_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	atari_cx85_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_vcs_control_port_interface overrides
	virtual u8 vcs_joy_r() override;
	virtual u8 vcs_pot_x_r() override;

	virtual bool has_pot_x() override { return true; }
	virtual bool has_pot_y() override { return false; } // pin 9 not used

private:
	required_device<mm74c923_device> m_encoder;
};


// device type declaration
DECLARE_DEVICE_TYPE(ATARI_CX85, atari_cx85_device)

#endif // MAME_BUS_VCS_CTRL_CX85_H
