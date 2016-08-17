// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System Keypad emulation

**********************************************************************/

#pragma once

#ifndef __VCS_KEYPAD__
#define __VCS_KEYPAD__

#include "emu.h"
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
	vcs_keypad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_vcs_control_port_interface overrides
	virtual UINT8 vcs_joy_r() override;
	virtual void vcs_joy_w( UINT8 data ) override;
	virtual UINT8 vcs_pot_x_r() override;
	virtual UINT8 vcs_pot_y_r() override;

	virtual bool has_pot_x() override { return true; }
	virtual bool has_pot_y() override { return true; }

private:
	required_ioport m_keypad;

	UINT8   m_column;
};


// device type definition
extern const device_type VCS_KEYPAD;


#endif
