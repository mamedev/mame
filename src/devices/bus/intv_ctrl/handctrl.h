// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Mattel Intellivision Hand Controller

**********************************************************************/

#ifndef MAME_BUS_INTV_CTRL_HANDCTRL_H
#define MAME_BUS_INTV_CTRL_HANDCTRL_H

#pragma once


#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> intv_handctrl_device

class intv_handctrl_device : public device_t,
							public device_intv_control_port_interface
{
public:
	// construction/destruction
	intv_handctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_intv_control_port_interface overrides
	virtual uint8_t read_ctrl() override;

private:
	required_ioport m_cfg;
	required_ioport m_keypad;
	required_ioport m_disc_dig;
	required_ioport m_disc_anx;
	required_ioport m_disc_any;
};


// device type definition
DECLARE_DEVICE_TYPE(INTV_HANDCTRL, intv_handctrl_device)


#endif // MAME_BUS_INTV_CTRL_HANDCTRL_H
