// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision Super Action Controller emulation

**********************************************************************/

#ifndef MAME_BUS_COLECO_CONTROLLER_SAC_H
#define MAME_BUS_COLECO_CONTROLLER_SAC_H

#pragma once

#include "ctrl.h"

#include "machine/quadmouse.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coleco_super_action_controller_device

class coleco_super_action_controller_device : public device_t,
											public device_colecovision_control_port_interface
{
public:
	// construction/destruction
	coleco_super_action_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	ioport_value keypad_r();

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_vcs_control_port_interface overrides
	virtual uint8_t joy_r() override;

private:
	required_ioport m_io_common0;
	required_ioport m_io_common1;
	required_ioport m_io_keypad;
	required_device<quadencoder_device> m_encoder;

	uint8_t m_mn;
	uint8_t m_pl;

	void mn_w(int state);
};


// device type definition
DECLARE_DEVICE_TYPE(COLECO_SUPER_ACTION_CONTROLLER, coleco_super_action_controller_device)

#endif // MAME_BUS_COLECO_CONTROLLER_SAC_H
