// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System analog paddles emulation

**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_PADDLES_H
#define MAME_BUS_VCS_CTRL_PADDLES_H

#pragma once

#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vcs_paddles_device

class vcs_paddles_device : public device_t,
							public device_vcs_control_port_interface
{
public:
	// construction/destruction
	vcs_paddles_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void reverse_players(device_t *device) { downcast<vcs_paddles_device &>(*device).m_reverse_players = true; }

	// device_vcs_control_port_interface overrides
	virtual uint8_t vcs_joy_r() override;
	virtual uint8_t vcs_pot_x_r() override;
	virtual uint8_t vcs_pot_y_r() override;

	virtual bool has_pot_x() override { return true; }
	virtual bool has_pot_y() override { return true; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_joy;
	required_ioport m_potx;
	required_ioport m_poty;

	bool m_reverse_players;
};


// device type definition
DECLARE_DEVICE_TYPE(VCS_PADDLES, vcs_paddles_device)

#endif // MAME_BUS_VCS_CTRL_PADDLES_H
