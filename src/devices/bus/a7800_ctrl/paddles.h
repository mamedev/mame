// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari analog paddles emulation

**********************************************************************/

#ifndef MAME_BUS_A7800_CTRL_PADDLES_H
#define MAME_BUS_A7800_CTRL_PADDLES_H

#pragma once

#include "ctrl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a7800_paddles_device

class a7800_paddles_device : public device_t,
							public device_a7800_control_port_interface
{
public:
	// construction/destruction
	a7800_paddles_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;


protected:
	// device-level overrides
	virtual void device_start() override;

	// device_a7800_control_port_interface overrides
	virtual uint8_t a7800_joy_r() override;
	virtual uint8_t a7800_pot_x_r() override;
	virtual uint8_t a7800_pot_y_r() override;

	virtual bool has_pot_x() override { return true; }
	virtual bool has_pot_y() override { return true; }
	virtual bool is_paddle() override { return true; }

private:
	required_ioport m_joy;
	required_ioport m_potx;
	required_ioport m_poty;
};


// device type definition
DECLARE_DEVICE_TYPE(A7800_PADDLES, a7800_paddles_device)

#endif // MAME_BUS_A7800_CTRL_PADDLES_H
