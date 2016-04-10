// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Mattel Intellivision Hand Controller

**********************************************************************/

#pragma once

#ifndef __INTV_HANDCTRL__
#define __INTV_HANDCTRL__


#include "emu.h"
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
	intv_handctrl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_intv_control_port_interface overrides
	virtual UINT8 read_ctrl() override;

private:
	required_ioport m_cfg;
    required_ioport m_keypad;
    required_ioport m_disc_dig;
    required_ioport m_disc_anx;
    required_ioport m_disc_any;
};


// device type definition
extern const device_type INTV_HANDCTRL;


#endif
